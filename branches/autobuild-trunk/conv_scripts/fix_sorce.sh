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
	if grep "../../include/" $file >/dev/null; then
		echo "Found old INC dir in $file"
		sed 's/\"..\/..\/include\//\"/g' $file > tmp1.c
		mv tmp1.c $file
	else
		echo "File is clean: $file"
	fi

done
