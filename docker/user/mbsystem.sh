#!/bin/bash
#
# mbsystem.sh: A laucher script for the dockerized MB-System programs.
#
# Status: PRELIMINARY
#

function log {
    echo -e "::: $1"
}

function error {
    echo -e "$1"
    exit "$2"
}

if [ -z "$MBSYSTEM_IMAGE" ]; then
    error "Required MBSYSTEM_IMAGE env var undefined" 1
fi

pgm=$1
if [ -z "$pgm" ]; then
    # TODO show help message including list of available programs?
    error "MB-System program not given" 2
fi
shift

# The graphical programs per https://www.mbari.org/products/research-software/mb-system/mb-system-youtube-tutorials/:
#   mbedit, mbeditviz, mbvelocitytool, mbgrdviz and mbnavedit
# plus: mbnavadjust  (TODO probably others?)
ISGUI=0
if [ "$pgm" = "mbedit" ] ||
   [ "$pgm" = "mbeditviz" ] ||
   [ "$pgm" = "mbnavadjust" ] ||
   [ "$pgm" = "mbvelocitytool" ] ||
   [ "$pgm" = "mbgrdviz" ] ||
   [ "$pgm" = "mbnavedit" ]; then
    ISGUI=1
fi

# Build the command to be run:

CMDPREFIX="docker run -it --rm"

# Set running user:
CMDPREFIX="$CMDPREFIX --user $(id -u)"

# Map current directory to /opt/MBSWorkDir:
CMDPREFIX="$CMDPREFIX -v $(pwd):/opt/MBSWorkDir"

# TODO determine host environment, in particular for any needed
#  preparations for the GUI programs.
#  Initially testing with MacOS and linux.
# Using a generic `os` variable for this:
log "OSTYPE=$OSTYPE"
if [[ "$OSTYPE" == "linux-gnu" ]]; then
    os="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    os="macos"
else
    echo "TODO $OSTYPE"
    exit 3
fi

if [ $ISGUI ]; then
    if [ "$os" = "linux" ]; then
        if [ -z "$DISPLAY" ]; then
            error "Required DISPLAY env var undefined" 4
        fi
        CMDPREFIX="${CMDPREFIX} -e DISPLAY --net=host"

    elif [ "$os" = "macos" ]; then
        socat TCP-LISTEN:6000,reuseaddr,fork UNIX-CLIENT:\"$DISPLAY\" &
        socat_pid=$!
        log "socat PID=$socat_pid"
        ip=$(ifconfig en0 | grep "inet " | sed 's/.*inet \([0-9\.]*\).*/\1/g')
        xhost + "${ip}"
        export DISPLAY=${ip}:0
        CMDPREFIX="${CMDPREFIX} -e DISPLAY"
    else
        # should not happen
        error "Unexpected os=$os" 99
    fi
fi

# final complete command to run:
CMD="$CMDPREFIX $MBSYSTEM_IMAGE $pgm $@"
log "running $CMD"
$CMD

if [ $ISGUI ]; then
    if [ "$os" = "macos" ]; then
        kill $socat_pid
    fi
fi
