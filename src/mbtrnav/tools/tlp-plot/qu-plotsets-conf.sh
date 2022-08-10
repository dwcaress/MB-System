#!/usr/local/bin/bash
##/bin/bash

# qu-plotsets-conf
# Stores definitions for plotsets so that they may be run again

# Plotset entries consist of
#   - a path to a set of mbtrnpp logs
#   - an mbtrnpp session ID (yyyymmdd-hhmmss) identifying the log files
#   - a plot description string (should describe the data set, used in plot subtitles)
#
# Each element is stored in an array, indexed by a unique key string
# The following key naming conventions are used:
#   - lower case
#   - begin with a letter (p)
# followed by
#   - session ID date (yymmdd)
#   - a letter+number data context indicator: mission (mn), test (tn)
#   - replay indicator (rn) if the data is a replay session
#   - version indicator (vn) to disambiguate duplicate entries (e.g. w/ different source data)
#     it is recommended to include the session ID, data origin, and context
# The keys are stored in a (non-associative) array
# The keys and indices are assigned dynamically, making it trivial to
# add/insert/delete/reorder entries without cut/paste errors
#
# qu-plotsets-conf is sourced by mbtrpp-plot.sh
# (alternative versions may be substituted)

# log path array
declare -A QU_LOG_PATHS
# session ID array
declare -A QU_SESSION_IDS
# description array
declare -A QU_DATA_SET_IDS
# key array
declare -a QU_PLOTSET_KEYS

# index dynamically so that entries may be
# modified w/ minimal editing
let "idx=-1"

# Example data set
let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="pExample"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="./example"
QU_SESSION_IDS["$QU_KEY"]="20200206-1658170000"
QU_DATA_SET_IDS["$QU_KEY"]="example plot mission (${QU_SESSION_IDS[$QU_KEY]})"

# 201911xx missions and replays
let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191113t0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/Volumes/linux-share/logs"
QU_SESSION_IDS["$QU_KEY"]="20191113-2245120000"
QU_DATA_SET_IDS["$QU_KEY"]="13nov19 test0 (${QU_SESSION_IDS[$QU_KEY]})"

