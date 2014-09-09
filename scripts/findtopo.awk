#!/bin/awk -f

# find the 2x2 topo grid with this region fits in.

BEGIN {
	FS="/"
}
{
	if ($1 > 1) {
		w=int($1);
	} else if ($1 < -1) {
		w=int($1)-1
	} else  
		w=int($2)-2

        if ($3 > 1) {
                s=int($3);
        } else if ($3 < -1) {
                s=int($3)-1
        } else 
                s=int($4)-2

	if (w +2 < $2 || s + 2 < $4) {
		print "out of range $0 " > 2
	}

	print "topo_" w "_" w + 2 "_" s "_" s + 2 ".grd"
}
