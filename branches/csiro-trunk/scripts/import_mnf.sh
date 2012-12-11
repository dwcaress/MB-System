#!/bin/sh -x

# script to inport MNF cleaned swath data into the mbblessed data set.

if [ -z "$1" ] ; then
	echo "Usage: $0 survey [src gsf raw dest]"
	echo " where survey is the voyage name"
	echo '       src is usually /home/MNF_multibeam/*survey '
	echo "       gsf is usually CARIS/Exports/GSF"
	echo "       raw is usually raw_em300"
	echo "       dest is usually /home/acoustics_swath/mbblessed/em300"
	exit 0
fi

if [ `id -gn` != echo-hf ] ; then
        sg echo-hf -c "$0 $*"
        exit 0
fi
umask 002

survey=$1
source="$2"

if [ -z "$source" ] ; then

	for src in /home/MNF_multibeam/*$survey* ; do
		if [ -z "$source" ] ; then
			source="$src"
		else
			echo multiple directories: $source  $src
			echo "I'm confused, quitting"
			exit 1
		fi
	done

fi

if [ -z "$3" ] ; then
	gsf="$source/CARIS/Exports/GSF"
elif [ "$3" == "-" ] ; then
	gsf="$source"
else
	gsf="$source/$3 -maxdepth 1"
fi

dest=/home/acoustics_swath/mbblessed/em300/

if [ -z "$4" ] ; then 
	raw="$source/raw_em300"
else
	raw=$source/$4/
fi

mkdir -p $dest/$survey
grep -q $survey $dest/datalist.mb-1 || echo $survey/datalist.mb-1  -1 1 >> $dest/datalist.mb-1

find $gsf -iname "*.gsf" | while read file ; do
	base=`basename $file .gsf`
	base=`basename $base .GSF`
	base=`basename $base _raw`
	rawfile=`find $raw -name "$base*.all"`
	if [ -z "$rawfile" ] ; then
		echo $base >> notfound
	else
		out=$dest/$survey/${base}_merge.mb57
		mbcopy -F56/57/121 -I $rawfile -M $file -O $out
		echo `basename $out` 57 1  >> $dest/$survey/datalist.mb-1
	fi
done

sort -k2 -t_ datalist.mb-1 > datalist
mv datalist datalist.mb-1

mbdatalist -O -I $dest/$survey/datalist.mb-1

mb_process $dest/$survey/datalist.mb-1

