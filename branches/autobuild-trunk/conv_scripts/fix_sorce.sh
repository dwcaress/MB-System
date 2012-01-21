#!/bin/sh

for file in src/mbview/*.c
do
	ff=`basename $file`

	echo "checking file $ff ($file)"
	if grep "HAVE_CONFIG_H" $file >/dev/null; then
		echo "Alread has HAVE_CONFIG"
	else
		echo "Adding ifdef HAVE_CONFIG_H"
		cat cnfg1.txt $file > tmp1.c
		mv tmp1.c $file
	fi

done
