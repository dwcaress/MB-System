#!/bin/sh

for file in src/mbview/*.h
do
	echo "checking file ($file)"
	if grep "../../include/" $file >/dev/null; then
		echo "Found old INC dir in $file"
		sed 's/\"..\/..\/include\//\"/g' $file > zz
		mv zz $file
	else
		echo "File is clean: $file"
	fi

done
