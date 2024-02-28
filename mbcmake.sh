#!/bin/bash
# Wrapper for MB-System cmake.

# N.B.: Most command-line options to this script *MUST* match spelling of
# of variables that can be provided on command-line to MB-System/CMakeLists.txt

# Boolean options are specified as option=0 or option=1
validBoolOptions=(buildGUIs buildOpenCV buildPCL
                  buildTRN buildTRNLCM buildTNav buildQt buildGSF buildTest)

# Module include directories and library files options are specified as
# <module>_include=<DIR> and <module>_lib=<LIBS>, respectively
validModules=(GMT Proj OpenGL X11 Motif GDAL NetCDF FFTW VTK)

# Directory parameters have format param=<DIR>
validDirParams=(otpsDir installDir)

help() {
    echo "$1"
    echo "specify boolean options as option=OFF or option=ON, where option is one of:"
    for opt in ${validBoolOptions[@]}; do
        echo $opt
    done

    echo
    echo "specify module include path with module_include=<DIR>"
    echo "specify module library with module_lib=<LIB>"
    echo "specify module libraries with module_lib=<LIB;LIB2;LIB3...>"        
    echo "where 'module' is one of:"
    for package in ${validModules[@]}; do
        echo $package
    done

    echo
    echo "specify directory parameters with parameter=<DIR>"
    echo "where 'parameter' is one of:"
    for dir in ${validDirParams[@]}; do
        echo $dir
    done

    echo
    echo "-DCMAKE_BUILD_TYPE=Debug/Release Compile/link with/without debug flags"
    echo "-DBUILD_SHARED_LIBS=ON/OFF Build shared libraries"
    }

# Process commad-line options, convert each to format recognized by cmake
declare -a options
error=false

while (( "$#" )); do
    found=false
    echo arg: $1
    # Is arg a valid boolean option?
    for opt in ${validBoolOptions[@]}; do
        if [[ "$1" =~ ${opt}=.* ]]; then
            options[${#options[@]}]="-D${1}"
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
            options[${#options[@]}]="-D${1}"
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
            options[${#options[@]}]="-D${1}"
            found=true
            break
        fi
    done
    if [ "$found" == true ]; then
        shift
        continue
    fi    

    if [[ "$1" =~ -DCMAKE_BUILD_TYPE=.* ]]; then
        options[${#options[@]}]="-DCMAKE_BUILD_TYPE=Debug"
        echo OK
        shift
        continue
    fi

    if [[ "$1" =~ -DBUILD_SHARED_LIBS=.* ]]; then
        options[${#options[@]}]="-DBUILD_SHARED_LIBS=ON"
        shift
        continue
    fi    

    if [[ "$1" =~ verbose ]]; then
        options[${#options[@]}]="-Dverbose=1"
        shift
        continue
    fi

    # If we get here, then unknown option
    error=true
    echo unknown/invalid/incomplete option: \'$1\'

    shift
done

if [ "$error" == true ]; then
    help
    exit 1
fi

echo options: ${options[*]}

echo run cmake
# Go to build and invoke cmake
cd build
cmake ${options[*]} ..
