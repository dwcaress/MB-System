#!/bin/bash
# Wrapper for MB-System cmake.
validOptions=(buildTrn buildTnav buildQt buildOpencv buildGsf buildTest
              buildPCL)

validModules=(GMT Proj OpenGL X11 Motif GDAL NetCDF FFTW)

help() {
    echo -n "$1 : "
    echo valid options:
    for opt in ${validOptions[@]}; do
        echo $opt
    done

    echo "assert package include path with package_include=<DIR>"
    echo "assert package library with package_lib=<LIB>"    
    echo "where 'package' is one of:"
    for package in ${validModules[@]}; do
        echo $package
    done
    }

# Process commad-line options, convert each to format recognized by cmake
options=

while (( "$#" )); do
    found=false
    echo arg: $1
    # Is arg a valid build option?
    for opt in ${validOptions[@]}; do
        if [ "$1" == $opt ]; then
            options=${options}' -D'$1
            echo FOUND VALID OPTION $1
            found=true
            break
        fi
    done
    if [ "$found" == true ]; then
        shift
        continue
    fi

    # Is arg a valid build parameter?
    for package in ${validModules[@]}; do
        hdrParam=${package}_include='.*'
        libParam=${package}_lib='.*'
        ### if [[ "$1" =~ $hdrParam ]]; then
        if [[ "$1" =~ $hdrParam ]] || [[ "$1" =~ $libParam ]]; then
            options=${options}' -D'$1
            echo FOUND VALID PARAMETER $1 
            found=true
            break
        fi
    done
    if [ "$found" == true ]; then
        shift
        continue
    fi
        
    # If we get here, then unknown option
    help "unknown option '$1'"

    shift
done
echo options: $options

# Go to build and invoke cmake
# cd build
# cmake $options ..
