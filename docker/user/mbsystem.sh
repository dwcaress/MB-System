#!/bin/bash
#
# mbsystem.sh: A launcher script for the dockerized MB-System programs.
#
# Status: Under testing
#

function usage() {
cat << EOF
${0##*/}: A launcher script for the dockerized MB-System programs.

Usage: ${0##*/} [-h] [-L] [program [args...]]

Options:
  -h                     Display this message and exit.
  -L                     Print some basic launching log messages.
  [program [args...]]    Desired program in the container to run.
                         By default, bash.

The script assumes the MBSYSTEM_IMAGE environment variable is defined.

EOF
}

doLog=0

function log {
    if [[ "$doLog" = "1" ]]; then
        echo -e "::: $1"
    fi
}

function error {
    echo -e "$1"
    exit "$2"
}

if [ -z "$MBSYSTEM_IMAGE" ]; then
    error "Required MBSYSTEM_IMAGE env var undefined" 1
fi

while [ $# -gt 0 ]; do
    case "$1" in
        -h)
          usage
          exit 0
          ;;
        -L)
          doLog=1
          shift
          ;;
        *)
          break
          ;;
    esac
done

pgm=$1
if [ -z "$pgm" ]; then
    pgm=bash
else
    shift
fi

# Build the command to be run:

#CMDPREFIX="docker run -it --rm --name `basename $pgm`"
CMDPREFIX="docker run -it --rm"

# Set running user:
CMDPREFIX="$CMDPREFIX --user $(id -u)"
log "Running as user $(id -u)"

# Map current directory to /opt/MBSWorkDir:
HOST_WORK_DIR=$(pwd)
CMDPREFIX="$CMDPREFIX -v $HOST_WORK_DIR:/opt/MBSWorkDir"
log "Host directory $HOST_WORK_DIR mounted as /opt/MBSWorkDir in container"

# Determine host environment.
# Initial focus on Linux and MacOS.
# From $OSTYPE, simplify the OS name in an $os variable:
log "OSTYPE=$OSTYPE"
if [[ "$OSTYPE" == "linux-gnu" ]]; then
    os="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    os="macos"
else
    echo "TODO $OSTYPE"
    exit 3
fi

# Check docker is available:
if [ "$os" = "linux" ] || [ "$os" = "macos" ]; then
    if ! command -v docker >/dev/null 2>&1 ; then
        error "'docker' command not found" 1
    fi
fi

if [ "$os" = "linux" ]; then
    if [ -z "$DISPLAY" ]; then
        error "Required DISPLAY env var undefined" 4
    fi
    CMDPREFIX="${CMDPREFIX} -e DISPLAY --net=host"

elif [ "$os" = "macos" ]; then
    if ! command -v socat >/dev/null 2>&1 ; then
        error "'socat' command not found" 1
    fi
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

# final complete command to run:
CMD="$CMDPREFIX $MBSYSTEM_IMAGE $pgm $@"
log "Running: $CMD\n"
$CMD

if [ "$os" = "macos" ]; then
    echo ""
    log "Ending socat PID=$socat_pid"
    kill $socat_pid
fi
