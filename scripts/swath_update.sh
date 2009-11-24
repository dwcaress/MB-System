#!/bin/bash
#
#  Command to automatically generate an up to date set of MBsystem format 57 files and datalists from EM300 raw data
#
#  usage: swath_update.sh [ source_directory [ destination_directory ] ]
#                         source_directory - directory containing EM300 surveys - default /swath4/raw
#                         destinaton_directory - directory to copy mb57 sureveys - default /swath4/mbproc/raw
#
#  Author Gordon Keith - CSIRO Marine - 27 April 2004
#
#  Modification history
#    20071017 GJK mbbackangle -P changed from 50 to 1000
#

# The following two lines added for when run as a cron job by root
PATH=$PATH:/usr/local/mbsystem/bin:/usr/local/bin:/usr/bin
umask 002

SRC=$1
DST=$2

RAWEXT=_raw.all
MBEXT=.mb57
DATALIST=datalist.mb-1

case "$HOST" in
	neptune)
		[ -z "$SRC" ] && SRC=/swath4/raw
		[ -z "$DST" ] && DST=/swath4/mbproc/raw
		;;

	larry|tracy|ingrid|*-hf)
		if [ `id -gn` != echo-hf ] ; then
			sg echo-hf -c "$0 $*"
			exit 0
		fi
		[ -z "$SRC" ] && SRC=/home/swath/raw/incoming
		[ -z "$DST" ] && DST=/home/blue8/acoustics/swath/mbproc/em300

		;;
esac

LOCKFILE=/tmp/swath_update.lock


# Drop out if previous update is still running.
# Unless previous update is on a different day, in which case assume it
# has fallen over and go anyway.
#
# There should not be a problem if multiple updates run at once,
# except that the system will thrash if there are too many of them.

DATE=`date -u +%Y%m%d`
if [ -f $LOCKFILE ] ; then
	test=`cat $LOCKFILE`
	if [ "$test" = "$DATE" ] ; then
		echo Lock file $LOCKFILE exists. Another update in progress.
		exit 0
	fi
fi

echo $DATE > $LOCKFILE

if [ -d "$SRC" ] ; then
# Ensure all raw files under $SRC are mbcopy'ed to $DST
echo `date` Updating dataset...
(
cd $SRC
ls -d * */*  2> /dev/null | (
    while read file; do
	
	# if the file is a directory make sure we copy the directory structure
	if [ -d "$SRC/$file" -a ! -d "$DST/$file" ] ; then
	    mkdir "$DST/$file"
	    touch "$DST/$file/$DATALIST"
	    echo "$file/$DATALIST" -1 1 >> "$DST/$DATALIST"
	fi
	
	# if it is a real file and it's old enough and it's not already there
	# copy it
	if [ -f "$SRC/$file" ] ; then

	    SRCDIR=`dirname	"$file"`
	    SRCLINE=`basename "$file" $RAWEXT`

	    mbfile="$SRCDIR/$SRCLINE$MBEXT"

	    # if filename ends in _raw.all
	    if [ "$file" == "$SRCDIR/$SRCLINE$RAWEXT" ] ; then
		if [ ! -f "$DST/$mbfile" -o "$DST/$mbfile" -ot "$SRC/$file" ] ; then
		    echo $file
		    nice mbcopy -F56/57 -I"$SRC/$file" -O"$DST/$mbfile" && \
			echo $SRCLINE$MBEXT 57 1 >> "$DST/$SRCDIR/$DATALIST"
		    nice mbclean -S10/3/2 -F57 -I"$DST/$SRCDIR/$SRCLINE$MBEXT"
		    nice mbbackangle -A1 -Q -n181/90 -p1000 -R40 -F57 -I"$DST/$SRCDIR/$SRCLINE$MBEXT" -X-2.26
		    nice mbprocess -I"$DST/$SRCDIR/$SRCLINE$MBEXT"
		fi
	    fi 
	fi
    done
    )
)
fi

# Create list of lines to process

(
cd $DST


echo `date` Compiling data list...
nice mbdatalist -I $DATALIST -O 

)

/bin/rm -f $LOCKFILE
