# merge.sh - merge a directory of GSF files with the matching raw files

SURVEY=$1

if [ -z "$SURVEY" ] ; then
	echo "Usage: $0 survey [outdir] [rawdir]"
	echo "       outdir defaults to /home/swath/mbblessed/em300/<survey>"
	echo "       rawdir defaults to /home/swath/raw"
	exit 0
fi

RAWDIR=$3
OUTDIR=$2

[ -z "$RAWDIR" ] && RAWDIR=/home/swath/raw
[ -z "$OUTDIR" ] && OUTDIR=/home/swath/mbblessed/em300/`basename $SURVEY`

mkdir $OUTDIR && echo `basename $OUTDIR`/datalist.mb-1 -1 1 >> `dirname $OUTDIR`/datalist.mb-1

cd $SURVEY
ls *.gsf *.GSF | while read file ; do
	name=`basename $file .gsf`
	name=`basename $name .GSF`

	raw=$RAWDIR/$SURVEY/${name}_raw.all

	if [ ! -f $raw ] ; then
		raw=`ls $RAWDIR/*/*$name* | grep raw.all | head -1`
	fi

	if [ -z "$raw" ] ; then
		echo $file >> notfound
	
	else
		out=$OUTDIR/`basename $raw _raw.all`_merge.mb57
	

		if [ -f $raw ] ; then
			mbcopy -F56/57/121 -I $raw -O $out -M $file
			nice mbbackangle -Q -n181/90 -p1000 -R40 -F57 -I $out &
			echo `basename $out` 57 1 >> `dirname $out`/datalist.mb-1
		fi
	fi
done	
