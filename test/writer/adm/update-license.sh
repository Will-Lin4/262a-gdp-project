#!/bin/sh

adm=`dirname $0`

update_license() {
	file=$1
	{
	    cp $file $file.BAK &&
	    awk -f $adm/update-license.awk $file > $file.$$ &&
	    cp $file.$$ $file &&
	    rm $file.$$
	} ||
	    echo WARNING: could not update license for $file 1>&2
}

#  Fudge LICENSE file in this directory or parent, no further up
rmLICENSE=false
if [ ! -r LICENSE ]
then
	if [ -r ../LICENSE ]
	then
		ln -s ../LICENSE
		rmLICENSE=true
	else
		echo "[ERROR] LICENSE file must exist"
		exit 1
	fi
fi


for f
do
	echo Updating $f
	update_license $f
done

if $rmLICENSE
then
	rm LICENSE
fi

exit 0
