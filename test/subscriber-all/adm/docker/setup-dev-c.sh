#!/usr/bin/env sh

#
#  Script to set up the source tree.
#	Installs the developer-related code.
#	Removes the source tree, except for adm, which is used by other
#		builds.
#	Doesn't install documentation, because that requires pandoc,
#		which pulls in a huge number of dependencies.  This
#		should be OK since humans tend to work outside
#		containers rather than in them.  If it's needed,
#		it will be necessary to run:
#			sudo apt install pandoc
#			cd /src/gdp
#			sudo make install-doc
#

set -e

cd /src/gdp
make install-dev-c
srcfiles=`ls | grep -v '^adm$'`
rm -r $srcfiles
