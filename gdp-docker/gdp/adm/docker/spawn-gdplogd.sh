#!/bin/sh

#
#  Spawn a gdplogd container.
#
#	This is run on the host OS, not in the container.
#	It arranges to run the container with appropriate defaults.
#
#	NOTE: This does not work with bash, which "helpfully" tries
#	to escape quotes, but gets it wrong.  Fortunately, the default
#	shell on Debian is dash, which is better, although even dash
#	gets it wrong, which is why we individually export envars
#	and then tell docker to pick them up from the environment.
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
#  This parses the same files that would be used if the daemon were not
#  running in a container and passes them into the container using
#  environment variables.  It only handles a small set of important
#  variables.
paramfiles=`cat <<-EOF
	/etc/gdp/params/gdp
	/etc/gdp/params/gdplogd
	/usr/local/etc/gdp/params/gdp
	/usr/local/etc/gdp/params/gdplogd
	$HOME/.ep_adm_params/gdp
	$HOME/.ep_adm_params/gdplogd
EOF
`

# drop comments, change dots in names to underscores, clean up syntax.
# need to repeat one of the patterns because the semantics of global
# substitute means it won't retry overlapping patterns.
tmpfile=/tmp/$$
sed \
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
		$paramfiles > $tmpfile 2>/dev/null
unset paramfiles
. $tmpfile
rm $tmpfile

# fixed arguments: data (log) location
: ${swarm_gdp_data_dir:=/var/swarm/gdp}
: ${swarm_gdplogd_log_dir:=glogs}
pwd
cd $swarm_gdp_data_dir		# in case log_dir uses relative path
cd $swarm_gdplogd_log_dir	# might be relative or absolute
args="-v $PWD:/var/swarm/gdp/glogs"
args="$args -v ${GDP_ETC}:/etc/gdp:ro"

# name of log server
if [ -z "$swarm_gdplogd_gdpname" ]; then
	# make up a name based on our host name
	swarm_gdplogd_gdpname=`hostname --fqdn | \
				tr '.' '\n' | \
				tac | \
				tr '\n' '.' | \
				sed 's/\.$//'`
fi
: ${GDPLOGD_NAME:=${swarm_gdplogd_gdpname}}
export GDPLOGD_NAME
args="$args -e GDPLOGD_NAME"

# optional argument: routers
: ${GDP_ROUTER:=${swarm_gdp_routers}}
export GDP_ROUTER
test -z "${GDP_ROUTER}" || args="$args -e GDP_ROUTER"

# name of Human-Oriented Name to GDPname Directory server (IP address)
: ${GDP_HONGD_SERVER:=${swarm_gdp_hongdb_host}}
export GDP_HONGD_SERVER
test -z "$GDP_HONGD_SERVER" || args="$args -e GDP_HONGD_SERVER"

# additional arguments to gdplogd itself (passed in environment)
# can also be passed on the command line (after "--")
test -z "$GDPLOGD_ARGS" || args="$args -e GDPLOGD_ARGS"

if $debug; then
	echo ""
	echo "=== Environment ==="
	env | grep GDP
	echo ""
	echo "=== Command Line Args ==="
	echo "$*"
	echo ""
	echo "=== Command ==="
	echo "exec docker run $args $@ gdplogd:$VER"
else
	echo "exec docker run $args gdplogd:$VER $@"
	exec docker run $args gdplogd:$VER "$@"
fi
