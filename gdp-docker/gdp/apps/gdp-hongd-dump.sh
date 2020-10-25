#!/bin/sh

#
#  Dump the HONGD database
#

args=`getopt Dv: $*`
test $? != 0 && echo "Usage: $0 [-D] [-v version]" >&2 && exit 1
set -- $args
debug=false
: ${VER:=latest}
while true
do
	case "$1" in
	  -D)
		debug=true
		;;
	  -v)
		VER=$1
		shift
		;;
	  --)
		shift
		break;;
	esac
	shift
done

: ${GDP_ETC:=/etc/gdp}
#  This parses the same files that are used by the C library.
#  It only handles a small set of important variables.
paramfiles=`cat <<-EOF
	/etc/gdp/params/gdp
	/usr/local/etc/gdp/params/gdp
	$HOME/.ep_adm_params/gdp
EOF
`

# drop comments, change dots in names to underscores, clean up syntax.
# need to repeat one of the patterns because the semantics of global
# substitute means it won't retry overlapping patterns.
tmpfile=/tmp/gdp-$$
grep -h '^swarm' $paramfiles 2>/dev/null | sed \
	-e "/^#/d" \
	-e "s/\.\([^.=]*\)=/_\1=/" \
	-e "s/\.\([^.=]*\)=/_\1=/" \
	-e "s/\.\([^.=]*\)=/_\1=/" \
	-e "s/\.\([^.=]*\)=/_\1=/" \
	-e "s/\.\([^.=]*\)=/_\1=/" \
	-e "s/\.\([^.=]*\)=/_\1=/" \
	-e "s/\.\([^.=]*\)=/_\1=/" \
	-e "s/\.\([^.=]*\)=/_\1=/" \
	-e "s/ *= */=\"/" \
	-e "s/$/\"/" \
		> $tmpfile 2>/dev/null
unset paramfiles
. $tmpfile
rm $tmpfile

if [ -z "${swarm_gdp_hongdb_host-}" ]
then
	echo "HONGD not configured" 1>&2
	exit 72
fi
: ${swarm_gdp_hongdb_database:=gdp_hongd}
: ${swarm_gdp_hongdb_table:=human_to_gdp}
: ${swarm_gdp_hongdb_user:=gdp_user}
: ${swarm_gdp_hongdb_passwd:=gdp_user}

echo mysql -h${swarm_gdp_hongdb_host} \
	-D${swarm_gdp_hongdb_database} \
	-u${swarm_gdp_hongdb_user} \
	-p${swarm_gdp_hongdb_passwd} \
	'SELECT hex(gname), hname FROM $swarm_gdp_hongdb_table;'
mysql -h${swarm_gdp_hongdb_host} \
	-D${swarm_gdp_hongdb_database} \
	-u${swarm_gdp_hongdb_user} \
	-p${swarm_gdp_hongdb_passwd} \
<<- EOSQL
	SELECT hex(gname), hname
	FROM $swarm_gdp_hongdb_table;
EOSQL
