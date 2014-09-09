#!/bin/bash

[ -z "$1" ] || datalist=$1
[ -z "$datalist" ] && datalist=/home/acoustics_swath/mbproc/em300/datalist.mb-1

mbdatalist -I $datalist > new_datalist
awk '{print $1}' new_datalist | sed s:.*/:: | sed s/.mb57.*// | awk -F_ '{print $1 "_" $2 "_" $3}' | sort -t_ -k2 > new_datafiles
sort -t_ -k2 -u new_datafiles > new_datafiles_u
diff new_datafiles new_datafiles_u | grep -v d | sed "s/< //" > new_duplicates
grep -f new_duplicates new_datalist


