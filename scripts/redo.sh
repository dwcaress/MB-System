#!/bin/bash -x

# Reprocess an area for a given scale and smaller

west=$1
east=$2
south=$3
north=$4
scale=$5

while [ $scale -gt 0 ] ; do
	if [ $scale -gt 2 ] ; then
		./process.sh $west $east $south $north $scale w$scale
	else
		wst=$[  $west / 10 ]
		est=$[  $east / 10  + 1 ]
		ten=$wst
		while [ $ten -lt $est ] ; do
			ww=${ten}0
			ee=$[ $ten + 1 ]0
			[ $ww -lt $west ] && ww=$west
			[ $ee -gt $east ] && ee=$east
			 ./process.sh $ww $ee $south $north $scale w${scale}_$ten
			ten=$[ $ten + 1 ]
		done
	fi
	scale=$[ $scale / 2 ]
done

