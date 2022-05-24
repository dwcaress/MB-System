#!/bin/bash
# Wrapper for MB-System cmake.

# Boolean options are specified as single word, which assumes true
validBoolOptions=(buildGUIs buildOpenCV buildPCL
                  buildTRN buildTNav buildQt buildGSF buildTest)

# Module include directories and library files options are specified as
# <module>_include=<DIR> and <module>_lib=<LIBS>, respectively
validModules=(GMT Proj OpenGL X11 Motif GDAL NetCDF FFTW VTK)

# Directory parameters have format param=<DIR>
validDirParams=(otpsDir installDir)

help() {
    echo -n "$1 : "
    echo valid boolean options:
    for opt in ${validBoolOptions[@]}; do
        echo $opt
    done

    echo "assert module include path with module_include=<DIR>"
    echo "assert module library with module_lib=<LIB>"    
    echo "where 'module' is one of:"
    for package in ${validModules[@]}; do
        echo $package
    done

    echo "specify directory parameters with parameter=<DIR>"
    echo "where 'parameter' is one of:"
    for dir in ${validDirParams[@]}; do
        echo $dir
    done
    
    }

# Process commad-line options, convert each to format recognized by cmake
options=
error=false

while (( "$#" )); do
    found=false
    echo arg: $1
    # Is arg a valid boolean option?
    for opt in ${validBoolOptions[@]}; do
        if [ "$1" == $opt ]; then
            options=${options}' '-D${1}=1
            found=true
            break
        fi
    done
    if [ "$found" == true ]; then
        shift
        continue
    fi

    # Is arg a valid module header/lib parameter?
    for module in ${validModules[@]}; do
        hdrParam=${module}_include='.+'
        libParam=${module}_lib='.+'
        if [[ "$1" =~ $hdrParam ]] || [[ "$1" =~ $libParam ]]; then
            options=${options}' "-D'$1\"
            found=true
            break
        fi
    done
    if [ "$found" == true ]; then
        shift
        continue
    fi

    # Is arg a valid directory parameter?
    for param in ${validDirParams[@]}; do
        pattern=${param}'.+'
        if [[ "$1" =~ $pattern ]]; then
            options=${options}' "-D'$1\"
            found=true
            break
        fi
    done
    if [ "$found" == true ]; then
        shift
        continue
    fi    

    if [ "$1" == "debug" ]; then
        options=${options}' "-DCMAKE_BUILD_TYPE=Debug"'
        shift
        continue
    fi

    if [ "$1" == "shared" ]; then
        options=${options}' "-DBUILD_SHARED_LIBS=ON"'
        shift
        continue
    fi    

    # If we get here, then unknown option
    error=true
    echo "unknown/invalid option '$1'"

    shift
done
echo options: $options

if [ "$error" == true ]; then
    help usage:
    exit 1
fi

echo run cmake
# Go to build and invoke cmake
cd build
cmake $options ..
