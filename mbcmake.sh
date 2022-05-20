#!/bin/bash
# Wrapper for MB-System cmake.
validOptions=("buildTrn" "buildQt")

help() {
    echo -n "$1 : "
    echo valid options:
    for opt in ${validOptions[@]}; do
        echo $opt
    done
    
    }

# Process commad-line options, convert each to format recognized by cmake
options=

while (( "$#" )); do
    echo arg: $1
    if [ "$1" == "buildTrn" ] ||
       [ "$1" == "buildQt" ]; then
        options=${options}' -D'$1
    else
        help 'unknown option specified'
    fi
    shift
done
echo options: $options

# Go to build and invoke cmake
# cd build
# cmake $options ..
