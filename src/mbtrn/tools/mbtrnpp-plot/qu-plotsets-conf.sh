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

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191114t0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/Volumes/linux-share/logs/m1test-14nov19"
QU_SESSION_IDS["$QU_KEY"]="20191114-2237200000"
QU_DATA_SET_IDS["$QU_KEY"]="14nov19 test0 (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m1v0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/Users/headley/projects/precisionControl/cruise-15nov2019/m1-20191115-161252"
QU_SESSION_IDS["$QU_KEY"]="20191115-1612520000"
QU_DATA_SET_IDS["$QU_KEY"]="15nov19 m1 (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m2v0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/Users/headley/projects/precisionControl/cruise-15nov2019/m2-20191115-174945"
QU_SESSION_IDS["$QU_KEY"]="20191115-1749450000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 15nov19 m2 (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m3_4v0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/Users/headley/projects/precisionControl/cruise-15nov2019/m3-4-20191115-174945"
QU_SESSION_IDS["$QU_KEY"]="20191115-1749450000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 15nov19 m3-4 (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m5v0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/Users/headley/projects/precisionControl/cruise-15nov2019/m5-20191115-204508"
QU_SESSION_IDS["$QU_KEY"]="20191115-2045080000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 15nov19 m5 (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m5r0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/home/headley/tmp/logs"
QU_SESSION_IDS["$QU_KEY"]="20191117-0306570000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 15nov19 m5 replay (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m6r0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/home/headley/tmp/logs"
QU_SESSION_IDS["$QU_KEY"]="20191117-0522420000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 15nov19 m6 replay (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m4r0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/mnt/vmhost/logs/20191115m4-replay-20191118-1901"
QU_SESSION_IDS["$QU_KEY"]="20191118-1901590000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 15nov19 m4 replay (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m5r1"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/mnt/vmhost/logs/20191115m5-replay-20191118-1741"
QU_SESSION_IDS["$QU_KEY"]="20191118-1741430000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 15nov19 m5 replay (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m6r1"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/mnt/vmhost/logs/20191115m6-replay-20191118-1822"
QU_SESSION_IDS["$QU_KEY"]="20191118-1822200000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 15nov19 m6 replay (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m3_4v1"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/mnt/vmhost/logs/m3-4-20191115-174945"
QU_SESSION_IDS["$QU_KEY"]="20191115-1749450000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 15nov19 m3-4 mission (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20191115m5_7v0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/mnt/vmhost/logs/m5-7-20191115-204508"
QU_SESSION_IDS["$QU_KEY"]="20191115-2045080000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 15nov19 m5-7 mission (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20200206m1_2v0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/Users/headley/projects/precisionControl/cruise-06feb20/worklogs"
QU_SESSION_IDS["$QU_KEY"]="20200206-1658170000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 06feb19 m1-2 mission (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20200206m3_8v0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/Users/headley/projects/precisionControl/cruise-06feb20/worklogs"
QU_SESSION_IDS["$QU_KEY"]="20200206-1945060000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 06feb19 m3-8 mission (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20190206m1r0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/mnt/vmhost/logs/20200206m1-replay"
QU_LOG_PATHS["$QU_KEY"]="/Volumes/linux-share/logs/20200206m1-replay"
QU_SESSION_IDS["$QU_KEY"]="20200206-2259150000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 06feb19 m1 replay (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20190206m2r0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/mnt/vmhost/logs/20200206m2-replay"
QU_LOG_PATHS["$QU_KEY"]="/Volumes/linux-share/logs/20200206m2-replay"
QU_SESSION_IDS["$QU_KEY"]="20200206-2311530000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 06feb19 m2 replay (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20190206m3r0"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/mnt/vmhost/logs/20200206m3-replay"
QU_LOG_PATHS["$QU_KEY"]="/Volumes/linux-share/logs/20200206m3-replay"
QU_SESSION_IDS["$QU_KEY"]="20200207-0006120000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 06feb19 m3 replay (${QU_SESSION_IDS[$QU_KEY]})"

let "idx=$idx+1"
QU_PLOTSET_KEYS[$idx]="p20190206m2r1"
QU_KEY=${QU_PLOTSET_KEYS[$idx]}
QU_LOG_PATHS["$QU_KEY"]="/mnt/vmhost/logs/20200206m2r1"
QU_LOG_PATHS["$QU_KEY"]="/Volumes/linux-share/logs/20200206m2r1"
QU_SESSION_IDS["$QU_KEY"]="20200212-1807160000"
QU_DATA_SET_IDS["$QU_KEY"]="cruise 06feb19 m2 replay (new/fixed logs) (${QU_SESSION_IDS[$QU_KEY]})"
