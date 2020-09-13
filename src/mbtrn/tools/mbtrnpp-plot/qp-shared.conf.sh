#!/usr/local/bin/bash
##/bin/bash

# Shared variable and environment values

# Variable Naming Convention:
# QP_ prefix denotes environment/variables that qplot depends upon (don't modify)
# QU_ prefix for user environment prevents naming conflicts (may be imported/exported, i.e. shared between files)
# QX_ prefix denotes local/application-specific variables (file scope)

# basic configuration
# usually QP_PLOT_HOME is set by the calling script or b
# overridden by the user
export QP_PLOT_HOME=${QP_PLOT_HOME:-`pwd`}
export QPLOT_CMD="${QP_PLOT_HOME}/qplot/bin/qplot"

export QP_VERBOSE=${QP_VERBOSE:-"N"}
export QP_DEBUG=${QP_DEBUG:-"Y"}
export QP_OUTPUT_DIR_DFL="plot-output"
export QP_PLOT_DATA_DIR_DFL="plot-data"
export QP_PLOT_DATA_DIR=${QP_PLOT_DATA_DIR:-${QP_PLOT_DATA_DIR_DFL}}
export QP_OUTPUT_DIR=${QP_OUTPUT_DIR:-${QP_OUTPUT_DIR_DFL}}

# plot output terminal type
QU_IMG_TYPE="png"

# plot styles
export QU_POINT_COLOR=9 #7(orng,blk,gray)
export QU_POINT_SIZE=0.1
export QU_POINT_TYPE=5
export QU_OTERM="pngcairo"

# define terminal output sizes
# index using QU_OTERM
declare -A QU_TERM_OSIZE
QU_TERM_OSIZE["aqua"]="2048,1536"
QU_TERM_OSIZE["pngcairo"]="1600,1200" #800,600" #"2048,1536"
QU_TERM_OSIZE["pdfcairo"]="17,11"
QU_TERM_OSIZE["x11"]="1024,768"
QU_TERM_OSIZE["png"]="800,600"
QU_TERM_OSIZE["pdf"]="800,600"

# lines
export QU_LINE_TYPE_DFL="
set linetype 1 lc rgb \"dark-red\"    lw 1 pt 5  dashtype 1
set linetype 2 lc rgb \"blue\"        lw 1 pt 8  dashtype 1
set linetype 3 lc rgb \"sea-green\"   lw 1 pt 7  dashtype 1
set linetype 4 lc rgb \"black\"       lw 1 pt 11 dashtype 1
set linetype 5 lc rgb \"dark-orange\" lw 1 pt 3  dashtype 1
set linetype 6 lc rgb \"goldenrod\"   lw 1 pt 3  dashtype 2
set linetype 7 lc rgb \"dark-violet\" lw 1 pt 1  dashtype 3
set linetype 8 lc rgb \"cyan\"        lw 1 pt 6  dashtype 4
set linetype cycle 8
"
export QU_LINE_TYPE_DFL="
set linetype 1 lc rgb '#0072bd'   lw 1 pt 5  #dashtype 1
set linetype 2 lc rgb '#d95319'   lw 1 pt 8  #dashtype 1
set linetype 3 lc rgb '#edb120'   lw 1 pt 7  #dashtype 1
set linetype 4 lc rgb '#7e2f8e'   lw 1 pt 11 #dashtype 1
set linetype 5 lc rgb '#77ac30'   lw 1 pt 3  #dashtype 1
set linetype 6 lc rgb '#4dbeee'   lw 1 pt 3  #dashtype 2
set linetype 7 lc rgb '#a2142f'   lw 1 pt 1  #dashtype 3
set linetype 8 lc rgb 'cyan'      lw 1 pt 6  #dashtype 4
set linetype cycle 8
"
# colors
export QU_BLUE="\'#0072bd\'"   # blue
export QU_ORANGE="\'#d95319\'" # orange
export QU_LTORANGE="\'#c97339\'" # light-orange
export QU_YELLOW="\'#edb120\'" # yellow
export QU_PURPLE="\'#7e2f8e\'" # purple
export QU_GREEN="\'#77ac30\'"  # green
export QU_LTBLUE="\'#4dbeee\'" # light-blue
export QU_RED="\'#a2142f\'"    # red
export QU_BLACK="\'#000000\'"  # black
export QU_GRAY="\'#aaaaaa\'"   # gray
export QU_GOLDENROD="\'goldenrod\'"  # goldenrod
