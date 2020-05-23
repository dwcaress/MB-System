#!/bin/bash
#
# mbsystem.sh: A launcher script for the dockerized MB-System
# on Linux and MacOS hosts.
#
# Edit this file to adjust the first couple of settings as needed,
# and then run it, for example:
#
#       ./mbsystem.sh -h
#
# Status: Under testing
#
###################################################################

# The concrete image to use:
MBSYSTEM_IMAGE=mbari/mbsystem:5.7.6beta37

# Host directory to mount as /opt/MBSWorkDir in the container:
HOST_WORK_DIR=$(pwd)

# The running user (UID) for the container:
RUNNING_USER=$(id -u)


###################################################################
# Probably no more changes needed after this point.
###################################################################

function usage() {
cat << EOF
${0##*/}: A launcher script for the dockerized MB-System programs.

Usage: ${0##*/} [-h] [-L] [program [args...]]

Options:
  -h                     Display this message and exit.
  -L                     Print some basic launching log messages.
  [program [args...]]    Desired program in the container to run.
                         By default, bash.

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


pgm=$1
if [ -z "$pgm" ]; then
    pgm=bash
else
    shift
fi

# Container name only dependent on the concrete program to be run (any arguments ignored):
container_name="mbsystem-$(basename $pgm | sed 's/[^a-zA-Z0-9_]/-/g')"
# In particular, this helps in two ways:
# - determining whether to start a brand new container or re-start one previously run;
# - keep the state of a stopped/resumed container (e.g., for bash history purposes).

# Has the container already been created (i.e., previously run and now stopped)?
already_created=$(docker ps -a --filter "name=^/${container_name}$" --format '{{.Names}}')
# If so, just need to re-start it, see below.

# But, for simplicity, we force a complete fresh container if any arguments are being
# passed to the program (as they may be different):
if [[ $already_created ]] && [[ $# -gt 0 ]]; then
    log "Forcing new container ${container_name} because of passed arguments."
    docker rm "${container_name}"
    already_created=""
fi

# For simplicity, most of the following assuming a fresh container, but
# then we check already_created for the actual final command to be run.

# Build the command to be run (assuming fresh container):

CMDPREFIX="docker run -it --name ${container_name}"

# Set running user as the user for the container:
CMDPREFIX="$CMDPREFIX --user ${RUNNING_USER}"
log "Running as user ${RUNNING_USER}"

# Set work directory mapping:
CMDPREFIX="$CMDPREFIX -v ${HOST_WORK_DIR}:/opt/MBSWorkDir"
log "Mounting $HOST_WORK_DIR as /opt/MBSWorkDir in container"

# Map the following host file to /opt/mbsystem.bash_history:
HOST_BASH_HISTORY="$HOME/.mbsystem.bash_history"
CMDPREFIX="$CMDPREFIX -v $HOST_BASH_HISTORY:/opt/mbsystem.bash_history"
log "Mounting $HOST_BASH_HISTORY as /opt/mbsystem.bash_history in container"

# Make sure $HOST_BASH_HISTORY exists and is a regular file:
if test -d "$HOST_BASH_HISTORY"; then
    error "Unexpected: $HOST_BASH_HISTORY is a directory" 5
elif [[ ! -f "$HOST_BASH_HISTORY" ]]; then
    touch "$HOST_BASH_HISTORY"
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

# Ready to launch:

if [[ $already_created ]]; then
    log "Restarting existing container '${container_name}'"
    CMD="docker start -ai ${container_name}"
else
    log "Starting container '${container_name}'"
    CMD="$CMDPREFIX $MBSYSTEM_IMAGE $pgm $@"
fi

log "Running: $CMD\n"
$CMD

if [ "$os" = "macos" ]; then
    echo ""
    log "Ending socat PID=$socat_pid"
    kill $socat_pid
fi
