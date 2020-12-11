#!/bin/sh

#
#  If we are running git, get the current commit info
#

cd `dirname $0`/..

if type git > /dev/null 2>&1 && test -d .git
then
	# this will return the null string if we're not under git control
	rev=`git rev-parse HEAD 2> /dev/null`
	mods=`git status -s | grep -v '??'`
	test -z "$mods" || mods="++"
	revmods="$rev$mods"
	echo "$revmods" > git-version.txt
elif test -r git-version.txt
then
	# git isn't installed, but we have an old version file
	revmods=`cat git-version.txt`
else
	# no git information at all
	revmods=""
	cp /dev/null git-version.txt
fi

# add a leading space
test -z "$revmods" || revmods=" $revmods"

dtime=`date +'%Y-%m-%d %H:%M'`
echo "($dtime)$revmods"
