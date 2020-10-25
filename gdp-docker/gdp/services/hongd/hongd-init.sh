#!/bin/sh

#
#  Initialize HONGD Service database
#
#	We're assuming MariaDB here, although MySQL can work.  The issue
#	(as of this writing) is about licenses, not functionality.  That
#	may (probably will) change in the future, since it appears that
#	recent versions of MariaDB have better support for replication.
#

set -e
cd `dirname $0`/../..
: ${GDP_SRC_ROOT:=`pwd`}
cd $GDP_SRC_ROOT
if [ "$GDP_SRC_ROOT" = "/" -a -d "gdp/adm" ]
then
	# running in a Docker container
	GDP_SRC_ROOT="/gdp"
	cd $GDP_SRC_ROOT
fi
. adm/common-support.sh

debug=false
install_mariadb=false
args=`getopt Di $*`

usage() {
	echo "Usage: $0 [-D] [-i]" >&2
	exit $EX_USAGE
}

if [ $? != 0 ]; then
	usage
fi
eval set -- $args
while true
do
	case "$1" in
	  -D)
		debug=true
		;;
	  -i)
		install_mariadb=true
		;;
	  --)
		shift
		break;;
	esac
	shift
done

: ${GDP_ETC:=/etc/gdp}

info "Installing Human-Oriented Name to GDPname Directory Service (HONGD)."
$debug || mkdir_gdp $GDP_ETC

# you should probably be able to import these
creation_service_name="gdp_creation_service"
creation_service_pw_file="${GDP_ETC}/creation_service_pw.txt"
set_up_passwd creation_service $creation_service_pw_file

hongd_admin_name="hongd_admin"
hongd_admin_pw_file="${GDP_ETC}/hongd_admin_pw.txt"
set_up_passwd hongd_admin $hongd_admin_pw_file

# these need to be well known, at least in your trust domain
gdp_user_name="gdp_user"
gdp_user_pw="gdp_user"
if [ -r "${GDP_ETC}/gdp_user_pw.txt" ]; then
	gdp_user_pw=`head -1 "${GDP_ETC}/gdp_user_pw.txt"`
fi

#
#  We need the Fully Qualified Domain Name because MariaDB/MySQL uses
#  it for authentication.  Unfortunately some systems require several
#  steps to set it properly, so often it is left unqualified.  We do
#  what we can.
#
set_fqdn() {
	fqdn=`hostname -f`
	case "$fqdn" in
	    *.*)
		# hostname is fully qualified (probably)
		return 0
		;;
	    "")
		fatal "Hostname not set --- cannot proceed."
		;;
	    *)
		warn "Cannot find domain name for host $fqdn."
		warn "Suggest adjusting /etc/hosts on your system."
		return 1
		;;
	esac
}


#
#  Install appropriate packages for MariaDB.  On some systems this can
#  require additional operations to make sure the package is current.
#
install_mariadb_packages() {
	info "Installing required packages"
	: ${GDP_MARIADB_VERSION:="10.4"}
	case "$OS" in
	   "ubuntu" | "debian" | "raspbian")
		sudo apt-get update
		sudo apt-get clean
		package mariadb-server-${GDP_MARIADB_VERSION}
		sudo cp /dev/stdin /etc/mysql/conf.d/open-tcp.cnf <<- EOF
			[mysqld]
			# allow connections from any address
			bind-address = 0.0.0.0
EOF
		sudo chmod 644 /etc/mysql/conf.d/open-tcp.cnf
		;;

	   "darwin")
		sudo port selfupdate
		package mariadb-${GDP_MARIADB_VERSION}-server
		sudo port select mysql mariadb-$GDP_MARIADB_VERSION
		sudo port load mariadb-${GDP_MARIADB_VERSION}-server
		;;

	   "freebsd")
		sudo pkg update
		ver=`echo $GDP_MARIADB_VERSION | sed 's/\.//'`
		package mariadb${ver}-server
		package base64
		;;

	   *)
		fatal "%0: unknown OS $OS"
		;;
	esac
}


# needs to be customized for other OSes
control_service() {
	cmd=$1
	svc=$2
	case "$OS" in
	  "ubuntu" | "debian" | "raspbian")
		sudo -s systemctl $cmd $svc
		;;
	  *)
		fatal "%0: unknown OS $OS"
		;;
	esac
}


#
#  Read a new password.
#  Uses specific prompts.
#
read_new_password() {
	local var=$1
	local prompt="${2:-new password}"
	local passwd
	read_passwd passwd "Enter $prompt"
	local passwd_compare
	read_passwd passwd_compare "Re-enter $prompt"
	if [ "$passwd" != "$passwd_compare" ]
	then
		error "Sorry, passwords must match"
		return 1
	fi
	eval "${var}=\$passwd"
	return 0
}


save_pw() {
	svc=$1
	svc_pw=`eval ${svc}_pw`
	svc_pw_file=`eval ${svc}_pw_file`
	echo svc=$svc svc_pw=$svc_pw svc_pw_file=$svc_pw_file
}

save_creation_service_pw() {
	if [ -r $creation_service_pw_file ]
	then
		if [ "$creation_service_pw" = `cat $creation_service_pw_file` ]
		then
			# file already exists
			info "$creation_service_pw_file is unchanged"
			return
		fi
	fi
	sudo -c \
		"umask 0137; \
		echo $creation_service_pw > $creation_service_pw_file \
		chown gdp:gdp $creation_service_pw_file"
}


#
#  This sets up the Human-GDP name database.  If necessary it will
#  try to set up the MariaDB system schema using initialize_mariadb.
#  It should be OK to call this even if HONGD database is already
#  set up, but it will prompt you for a password that won't be needed.
#
create_hongd_db() {
	# determine if mariadb or mysql are already up and running
	if ps -alx | grep mysqld | grep -vq grep
	then
		# it looks like a server is running
		warn "It appears MySQL or MariaDB is already running; I'll use that."
	else
		# apparently nothing running
		info "Starting up MariaDB/MySQL"
		$debug || control_service start mysql
	fi

	info "Setting up Human-Oriented Name to GDPname Directory database."
	hongd_sql=`cat << XYZZY
		-- Schema for the external -> internal log name mapping

		-- We create some roles for convenience
		CREATE ROLE IF NOT EXISTS hongd_reader;
		CREATE ROLE IF NOT EXISTS hongd_update;
		CREATE ROLE IF NOT EXISTS hongd_admin;

		-- The database is pretty simple....
		CREATE DATABASE IF NOT EXISTS gdp_hongd
			DEFAULT CHARACTER SET 'utf8';
		USE gdp_hongd;

		CREATE TABLE IF NOT EXISTS human_to_gdp (
			hname VARCHAR(255) PRIMARY KEY,
			gname BINARY(32));
		GRANT SELECT (hname, gname) ON human_to_gdp TO hongd_reader;
		GRANT SELECT, INSERT ON human_to_gdp TO hongd_update;
		GRANT * ON human_to_gdp TO hongd_admin;

		-- Minimally privileged user for doing reads, well known
		-- password.  Anonymous users kick out too many warnings.
		CREATE USER IF NOT EXISTS '$gdp_user_name'@'%'
			IDENTIFIED BY '$gdp_user_pw';
		GRANT hongd_reader TO '$gdp_user_name'@'%';
		SET DEFAULT ROLE hongd_reader FOR '$gdp_user_name'@'%';

		-- Privileged user for doing creations (append only)
		-- (should figure out a better way of managing the password)
		CREATE USER IF NOT EXISTS '$creation_service_name'@'%'
			IDENTIFIED BY '$creation_service_pw';
		GRANT hongd_update TO '$creation_service_name'@'%';
		SET DEFAULT ROLE hongd_update FOR '$creation_service_name'@'%';

		-- Privileged user for administration
		-- (should figure out a better way of managing the password)
		CREATE USER IF NOT EXISTS '$hongd_admin_name'@'%'
			IDENTIFIED BY '$hongd_admin_pw';
		GRANT hongd_admin TO '$hongd_admin_name'@'%';
		SET DEFAULT ROLE hongd_admin FOR '$hongd_admin_name'@'%';

		-- convenience view showing printable contents
		CREATE OR REPLACE VIEW human_to_printable AS
			SELECT hname, TO_BASE64(gname) AS pname
			FROM human_to_gdp;
		GRANT SELECT ON human_to_printable TO hongd_reader;
		GRANT SELECT ON human_to_printable TO hongd_update;
		GRANT SELECT ON human_to_printable TO hongd_admin;

		-- Convenience script to query service
		DELIMITER //
		CREATE OR REPLACE PROCEDURE
			hname2gname(pat VARCHAR(255))
		  BEGIN
			SELECT *
			FROM human_to_printable
			WHERE hname LIKE IFNULL(pat, '%');
		  END //
		DELIMITER ;
		GRANT EXECUTE ON PROCEDURE hname2gname TO hongd_reader;
		GRANT EXECUTE ON PROCEDURE hname2gname TO hongd_update;
		GRANT EXECUTE ON PROCEDURE hname2gname TO hongd_admin;
XYZZY
	`
	pwfile=`basename "$creation_service_pw_file"`
	if $debug
	then
		info "Will run:"
		echo "$hongd_sql"
	else
		info "The next password should be the MySQL/MariaDB root password"
	fi
	if $debug ||  echo "$hongd_sql" | sudo mysql -p
	then
		if ! $debug
		then
			save_creation_service_pw
		fi
	else
		fatal "Unable to initialize HONGD database."
	fi
}


#
#  Now is the time to make work actually happen.
#

set_fqdn
$debug && echo fqdn = $fqdn
$install_mariadb && install_mariadb_packages
create_hongd_db

echo ""
action "Please read the following instructions:"
cat <<- XYZZY
	All GDP client hosts that want to use Human-Oriented Names (hint: this will
	be almost all of them) need to have a pointer to this service in their
	runtime GDP configuration.  This will normally be in /etc/gdp/params/gdp
	or /usr/local/etc/gdp/params/gdp.  There should be a line in that
	file that reads:
	   swarm.gdp.hongdb.host=$fqdn
	Everything else should be automatic.

	We have plans to improve this in the future.
XYZZY
info "Thank you for your attention."
