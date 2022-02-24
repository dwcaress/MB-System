#!/bin/bash

# summary: output mbtrnpp config contents as command line options

if [ ${#} -ge 1 ] && [ -f $1 ]
then

while [ ${#} -ge 1 ]
do
cfg=$1

# fill array with options from config file
for opt in `grep -e "[a-zA-Z0-9]" $cfg | grep -v -e "^[ \t]*[/#]"` 
do
echo -n "--${opt} "
done
shift
done

else
echo 
echo " summary: output mbtrnpp config contents as command line options"
echo " use: $(basename $0) <config>"
echo 
echo " Examples:"
echo "  # show command line option string"
echo "    $(basename $0) foo.cfg"
echo 
echo "  # combine multiple file args"
echo "    $(basename $0) foo/bar/*.cfg"
echo 
echo "  # Use on mbtrnpp command line (with overrides)"
echo "    mbtrnpp \$($(basename $0) foo.cfg) --verbose=0 --trn-map=different.map"
echo 
fi
