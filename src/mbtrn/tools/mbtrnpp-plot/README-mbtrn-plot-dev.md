# mbtrnpp-plot 

mbtrnpp-plot is a front-end for qplot that generates plots using mbtrnpp log data.
mbtrnpp-plot reads mbtrnpp log files to produce CSV input for qplot.

 qplot generates gnuplot scripts and PDF files that use gnuplot and ghostscript APIs.
 qplot uses configuration files to define one or more plot images or to combine a set of
 plot images into a PDF file.

 
## Required Packages

* qplot
* bash 4+ (earlier versions do not support associative arrays)
* gnuplot
* ghostscript
* libpng
* libgd or gd2

### OSX requirements

* macports or homebrew

### Windows requirements

* cygwin 


## Quick Start

* clone mbtrnpp-plot repository  
`cd mbtrnpp-plot`
* Run the example  
`./mbtrnpp-plot.sh -vk pExample` 
* plots and data output to mbtrnpp-plot/jobs/job-0-<pid>
* all plots are included in mbtrnpp-comb.pdf

## mbtrnpp-plot files

The following files are used by mbtrrnpp-plot:

|             File              |                              Description                            |
| ----------------------:|:----------------------------------------------------- |
| mbtrnpp-plot.sh      | main plot script                                                  |
| qp-trnu.conf.sh       | TRN output/state log                                          |
| qp-mb1svr.conf.sh  | MB1 server metrics                                            |
| qp-trnusvr.conf.sh   | TRN pub/sub server metrics plot configuration |
| qp-comb.conf.sh     | qplot PDF combiner job configuration               |
| qp-mbtrnpp.conf.sh | mbtrnpp metrics plot configuration                   |
| qp-shared.conf.sh | shared qplot configuration                                |
| qp-trnsvr.conf.sh      | TRN server metrics plot configuration              |
| qu-plotsets-conf.sh  | mbtrnpp-plot plotset definitions                       |

 The configuration files are maintained by the user.
 
## Plot an existing data set

Plot sets defined in qu_plotsets.conf may be plotted using it's index or key.  
To list the existing plot set definitions, use the -l option; use -vl to see entry details.  
	`mbtrnpp-plot.sh -l`  
To plot by index, use the -i option (may use more than once).  
	`mbtrnpp-plot.sh -i 2 -i 17`  
To plot by key, use the -k option (may use more than once).  
	`mbtrnpp-plot.sh -k 'pExample -k 'p20191115m3_4v1'`   

## Plot a data set w/o a qu-plotsets-conf entry

Datasets may be plotted on-the-fly using the -s option. The argument for this option is
a data set specifier, a quoted, comma-delimited string containing:  
* the path the the mbtrnpp log files
* the mbtrnpp session ID (yyyymmdd-hhmmss) used in the log file names
* a session set ID string (a brief description that will appear in the plot sub-title)

Example:  
`mbtrnpp-plot.sh -s "/path/to/logs,20200102-140700,02jan20 cruise mission 3 replay (20200102-140700)"`   

## Add a plotset to qu-plotsets-conf

Plot set entries consist of 6 lines:  
```
let "idx=$idx+1"  
QU_PLOTSET_KEYS[$idx]="pExample"  
QU_KEY=${QU_PLOTSET_KEYS[$idx]}  
U_LOG_PATHS["$QU_KEY"]="./example"
QU_SESSION_IDS["$QU_KEY"]="20200206-1658170000"
QU_DATA_SET_IDS["$QU_KEY"]="example plot mission ($QU_SESSION_IDS[$QU_KEY])"
```
* index line: must be first  
   `let "idx=$idx+1"`  
* key definition: must be second    
`QU_PLOTSET_KEYS[$idx]="pExample"`
* for convenience, set entry key: must be third  
`QU_KEY=${QU_PLOTSET_KEYS[$idx]}`
* Log path definition:  
`QU_LOG_PATHS["$QU_KEY"]="./example"`
* Session ID definition:  
`QU_SESSION_IDS["$QU_KEY"]="20200206-1658170000"`
* Data set description:  
`QU_DATA_SET_IDS["$QU_KEY"]="example plot mission ($QU_SESSION_IDS[$QU_KEY])"`

## Add a plot 

* modify the relevant function in mbtrnpp-plot, e.g. plot_trnusvr()
  * define grep filter(s) and output CSV file names for plot set data
```
  # client activity
  export QU_TRNUSVR_XT_PUB_CSV="xt_trnusvr_pub.csv"
  export QU_TRNUSVR_CLICON_FILTER="cli_con"
  export QU_TRNUSVR_CLICON_CSV="trnusvr_clicon.csv"
  export QU_TRNUSVR_CLIDIS_FILTER="cli_dis"
  export QU_TRNUSVR_CLIDIS_CSV="trnusvr_clidis.csv"
  export QU_TRNUSVR_CLILL_FILTER="cli_list_len"
  export QU_TRNUSVR_CLILL_CSV="trnusvr_clill.csv"
```
  * add line to apply filter and generate CSV file
```
  grep ${QU_TRNUSVR_CLICON_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_CLICON_CSV}
  grep ${QU_TRNUSVR_CLIDIS_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_CLIDIS_CSV}
  grep ${QU_TRNUSVR_CLILL_FILTER} ${TRNUSVR_LOG} > ${QP_PLOT_DATA_DIR}/${QU_TRNUSVR_CLILL_CSV}
```
* add plot definition to qp-\<log\>-conf
    * define plot title, sub-title, output image name
```
  QU_TRNUSVR_CLI_PTITLE=${QU_PTITLE:-"TRNUSVR client activity"}
  QU_TRNUSVR_CLI_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}\
  export QU_TRNUSVR_CLI_OIMG_NAME="trnusvr-cli"
```
    * add plot job key (QU_KEYS) entry
```
  declare -a QU_KEYS=( "trnusvr-prof" "trnusvr-cli" "comb-all" )
```
    * add job definition entry
```
  QP_JOB_DEFS["${QU_KEYS[1]}"]="plot,${QU_OTERM},${QU_KEYS[1]}"
```
    * add job order entry 
```
  QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[1]}"
```
    * define plot parameters
```
QU_KEY=${QU_KEYS[1]}
QP_OFILE_NAME["$QU_KEY"]="${QU_TRNUSVR_CLI_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_TRNUSVR_CLI_PTITLE}${QU_TRNUSVR_CLI_STITLE}"
...
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNUSVR_CLICON_CSV},${QU_BLUE},1,,1,5,,con"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNUSVR_CLIDIS_CSV},${QU_ORANGE},1,,1,5,,dis"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_TRNUSVR_CLILL_CSV},${QU_GREEN},1,,1,5,,llen"
```
* add plot to qp-comb.conf
  * define image name (must match plot image name defined in qp-\<log\>-conf)
```
  export QU_TRNUSVR_CLI_OIMG_NAME="trnusvr-cli"
```
  * define image path variable
```
  TRNUSVR_CLI="${QP_OUTPUT_DIR}/${QU_TRNUSVR_CLI_OIMG_NAME}.${QU_IMG_TYPE}"
```
  * insert image path variable into file list in the desired order
```
QU_FILE_LIST="${ESTMSE}|${ESTVAR}|${ESTMLE}|${MSEDAT}|${MSEX}|${MSEY}|\
${MSEZ}|${PTDAT}|${PTX}|${PTY}|${PTZ}|${MLEDAT}|${MLEX}|${MLEY}|${MLEZ}|\
${MBTRNPP_PROFILE}|${MBTRNPP_EVENTS}|${MBTRNPP_ERRORS}|${TRNSVR_PROFILE}|\
${TRNSVR_CLI}|${TRNUSVR_PROFILE}|${TRNUSVR_CLI}|${MB1SVR_PROFILE}|\
${MB1SVR_CLI}|${TRN_STATE1}|${TRN_STATE2}"
```
## File Naming Conventions

qp-\*.conf : qplot job configuration/job definitionos
qu-\*.conf : mbtrnpp configuration/job definitions

## Variable Naming Conventions

QU_ prefix for user environment prevents naming conflicts
QP_ prefix denotes qplot environment/variables
QX_ prefix denotes local environment/variables

## Appendix: qplot Configuration

qplot configuration files are used to configure plot images for one or more data sets.
By convention, these are named   
```
qp-\<name\>.conf
```
and contain configruations for a related set of files.  
Configuration files typically have the following general structure:  
* Environment setup
* Using environment variable enable definition in a single location for use throughout a configuration
* import shared variables (colors, global variables, etc)
* environment may be exported by upstream callers
* define local environment and variables 
* plot titles, image names, time formats, plot job keys (mnemonics) etc.
* Plot job setup
* job definitions
* set job order
* optionally define local PDF combiner job
* Plot configurations
* define one or more plots

### File Naming Conventions

qp-\*.conf : qplot job configuration/job definitionos
qu-\*.conf : mbtrnpp configuration/job definitions

### Variable Naming Convention

Variables in cp configuration files and scripts use the following naming convention:
* QP_ prefix denotes environment/variables that qplot depends upon (don't modify)
* QU_ prefix for user environment prevents naming conflicts (may be imported/exported, i.e. shared between files)
* QX_ prefix denotes local/application-specific variables (file scope)

### Bash version

qplot uses Bash version 4 or later.  
Earlier versions do not support associative arrays, which are used extensively in qplot.  
OSX typically uses an older Bash; a more recent version may be installed using mac ports or homebrew.
In that case, it may be necessary to modify the bash invocation (#!/bin/bash) in qplot and/or configuration files and scripts to reflect the path to the correct version.

### Annotated Configuation Example : qp-mbtrnpp.conf

Importing shared environment:
```
# import shared environment, variables

source qp-shared.conf.sh
```
Define some parameters that will be referenced in the plot definitions.
These include  
* title, sub-title and image name defined for three plots
* a set of plot job keys (mnemonics)
* input/output time formats (we want to read and plot using ISO1806) 

Note that QU_DATA_SET_ID is exported by mbtrnpp-plot:  
```
# define plot titles and image names

QU_MBTRNPP_PROFILE_PTITLE=${QU_PTITLE:-"MBTRNPP operation profile"}
QU_MBTRNPP_PROFILE_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_MBTRNPP_PROFILE_OIMG_NAME="mbtrnpp-prof"

QU_MBTRNPP_EVENTS_PTITLE=${QU_PTITLE:-"MBTRNPP EVENTS"}
QU_MBTRNPP_EVENTS_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_MBTRNPP_EVENTS_OIMG_NAME="mbtrnpp-events"

QU_MBTRNPP_ERRORS_PTITLE=${QU_PTITLE:-"MBTRNPP ERRORS"}
QU_MBTRNPP_ERRORS_STITLE=${QU_STITLE:-"\n${QU_DATA_SET_ID}"}
export QU_MBTRNPP_ERRORS_OIMG_NAME="mbtrnpp-errors"

# Define job names to use in the configuration
declare -a QU_KEYS=( "mbtrpp-prof" "mbtrnpp-events" "mbtrnpp-errors" "comb-all" )

# Set time formats for data and plots
# time format strings conform to gnuplot syntax
# (see man pages: strftime/strptime, date)

# input time format (must match data files)
# e.g. 2019-11-13T22:52:29Z
QU_ITIME="%Y-%m-%dT%H:%M:%SZ"
# output time format (gnuplot plot x-axis)
QU_OTIME="%Y-%m-%dT%H:%M:%SZ"
```
Define three plot jobs. The combiner job is disabled, since a separate combinber job 
for all of the plots is configured in a separate configuration file. Each plot job definition consists of  
* the plot keyword
* terminal type
* key (job name)

Note that QU_OTERM (output terminal type) is exported by qp-shared.conf.  
```
# Job definitions (qplot associative array QP_JOB_DEFS)
# use the formats
#
#   plot,<terminal>,<key> (create plot image)
#   combine,<terminal>,<output_path>,<file_list> (combine plots in PDF)
# 
# where
#
# <terminal>    : output device 
#                  OSX/macports: ps, aqua, pdfcairo, pngcairo, x11 (requires Quartz)
#                  linux       : ps, png, pdf, x11
#
# <key>         : plot job name
#
# <output_path> : combiner output (PDF) file path
#
# <file_list>   : list of files to combine (delimited using '|')

QP_JOB_DEFS["${QU_KEYS[0]}"]="plot,${QU_OTERM},${QU_KEYS[0]}"
QP_JOB_DEFS["${QU_KEYS[1]}"]="plot,${QU_OTERM},${QU_KEYS[1]}"
QP_JOB_DEFS["${QU_KEYS[2]}"]="plot,${QU_OTERM},${QU_KEYS[2]}"
# local combiner job (e.g. make PDF containing plots in this configuration)
#QP_JOB_DEFS["${QU_KEYS[1]}"]="combine,png,plot-output/${QU_OFILE_NAME}${QU_SESSION_ID}.pdf,./plot-output/*png"
```
Set the job order. QP_JOB_ORDER is a qplot variable, an array containing job definition keys.
The combiner job is disabled.
```
# use QP_JOB_ORDER to enable plot jobs and set job order
# NOTE: plot combiner job is last, since plots must be completed first

# Set job order
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[0]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[1]}"
QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[2]}"
#QP_JOB_ORDER[${#QP_JOB_ORDER[*]}]="${QU_KEYS[3]}"
```
Finally, define the three plots; after setting plot formatting parameters, 
one or more plot specifiers are used to configure the plot traces.
The following plot parameters are used to define plot formatting options:
```
# Plot configuration parameters
# QP_OFILE_NAME     : Output file name (without extension)
# QP_PTITLE         : Plot title
# QP_TFMT           : Input time format (see man pages for date,strfmt)
# QP_XTFMT          : X-axis (output) time format
# QP_ISTIME         : X-axis is time [Y|N]
# QP_DSEP           : Data separator
# QP_KEY_FONT       : Key (legend) font name
# QP_KEY_SIZE       : Key (legend) font size
# QP_TERM_FONT      : Terminal font name
# QP_TERM_SIZE      : Terminal font size
# QP_TERM_OSIZE     : Terminal output size
# QP_XTITLE         : X-axis title
# QP_XRANGE_MIN     : X-axis range minimum value [*:*]
# QP_XRANGE_MAX     : X-axis range maximum value [*:*]
# QP_YTITLE         : Y-axis title
# QP_YRANGE_MIN     : Y-axis range minimum value [*:*]
# QP_YRANGE_MAX     : Y-axis range maximum value [*:*]
# QP_X2TITLE        : second X-axis title
# QP_X2RANGE_MIN    : second X-axis range minimum value [*:*]
# QP_X2RANGE_MAX    : second X-axis range maximum value [*:*]
# QP_Y2TITLE        : second Y-axis title
# QP_Y2RANGE_MIN    : second Y-axis range minimum value [*:*]
# QP_Y2RANGE_MAX    : second Y-axis range maximum value [*:*]
# QP_XROTATION      : X-axis tic label rotation (deg)
# QP_XOFFSET        : X-axis tic label offset (h,v characters)
# QP_X2ROTATION     : second X-axis tic label rotation (deg)
# QP_X2OFFSET       : second X-axis tic label offset (h,v characters)
# QP_XSCALE         : X axis value multiplier
# QP_YSCALE         : Y axis value multiplier
# QP_XOFS           : X axis value offset
# QP_YOFS           : Y axis value offset
# QP_PLOT_STYLE     : plot style - lines, points, linespoints [points]
# QP_POINTSIZE      : point size - decimal value
# QP_POINTTYPE      : point type [1:78]
#                      1 PLS   2 CRS    3 STAR  4 BOX
#                      5       6 BOXF   7 CIR   8 CIRF
#                      9 TRIU 10 TRIUF 11 TRID 12 TRIDF
#                     13 DIA  14 DIAF  15 PENT 16 PENTF
# QP_POINTCOLOR     : point color [1:10]
#                     may also use rgb : QU_BLUE="\'#0072bd\'"
# QP_INC_POINTTYPE  : increment point types (if QP_USE_LINETYPES=="N") ["Y"|"N"]
# QP_INC_POINTCOLOR : increment point color (if QP_USE_LINETYPES=="N") ["Y"|"N"]
# QP_LINE_TYPE      : line type [1:N] (N=8 defaults defined)
#                     (see QP_LINE_TYPES, QP_LINE_TYPES_DFL)
# QP_USE_LINETYPES  : use line type definitions ["Y"|"N"]
# QP_INC_LINETYPE   : increment line type ["Y"|"N"]
# QP_LINE_TYPES     : line type definitions
#                     "set linetype ...
#                     set linetype ..."
#                     {{linetype | lt} <line_type> | <colorspec>}
#                     {{linecolor | lc} <colorspec>}
#                     {{linewidth | lw} <line_width>}
#                     {{pointtype | pt} <point_type>}
#                     {{pointsize | ps} <point_size>}
#                     {{pointinterval | pi} <interval>}
#                     {{pointnumber | pn} <max_symbols>}
#                     {{dashtype | dt} <dashtype>}
#                     {palette}
# QP_EXPR           : column values are gnuplot expressions ["Y"|"N"]
#                     By default, plot specifiers assume x and y column values to be integer column numbers.
#                     Scaling and offset may be applied using QP_XSCALE, QP_YSCALE, QP_XOFS, QP_YSCALE,  QP_YOFS.
#                     The scaling parameters affect all traces in a plot.
#
#                     Alternatively, plots specifiers support functions using data columns as parameters for the y column(s).
#                     To do this, set QP_EXPR to "Y"; if addition is used in the expression, change the plot spec
#                     delimiter from '+' to '|' or other character using QP_SPECDEL, and use that character if there are multiple
#                     plot specifiers.
#                     Note that the first specifier should not begin with the delimiter (errors may occur if it is included).
#                     Expressions must use correct gnuplot `plot using` expression syntax:
#                       * enclosed in parentheses
#                       * '$' must be escaped "\$"
#                       * operators must be separated by whitespace
#                       * gnuplot functions may be called, e.g. sin(\$3)
#                     If QP_EXPR is set to "Y", all plot specifiers in that plot must use expression syntax (e.g. "(\$4)" not "4").
#                     QP_EXPR is ignored for the xcol if QP_ISTIME=="Y".
# QP_SPECDEL        : plot specifier delimiter ["+"|"|"]
#                     Used when QP_EXPR is set, to prevent splitting expressions containing "+"
```
Plot specifiers use the following format
```
# QP_PLOT_SPECS     : Plot specifications
#                     plot specifier format (comma delimited):
#                       0: file
#                       1: point color
#                       2: xcol
#                       3: x2col
#                       4: number of y columns (each ycol includes column,axes,title)
#                       5: ycol
#                       6: axes (blank,x1y1, x1y2, x2y1, x2y2)
#                       7: title...
#                       n-1: ycol
#                       n: title
#
#                     NOTE: plot specs may be concatenated using '+' (or to '|' using QP_SPECDEL )
#                     to include multiple graphs on the same plot
```
Define the plots; each defines multiple plots on a common axis.
Not all parameters are set for each plot (uses qplot defaults).

The QU_KEY value is set before each set of plot definitions.
Then the name, title, and time format are set.  
* IS_TIME parameter is set to Y since our x-axis is time.
* QP_TFMT must match the input (CSV) data format
* QP_XFMT is used to format the output (x-axis) time on the plot  

```
QU_KEY=${QU_KEYS[0]}
QP_OFILE_NAME["$QU_KEY"]="${QU_MBTRNPP_PROFILE_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_MBTRNPP_PROFILE_PTITLE}${QU_MBTRNPP_PROFILE_STITLE}"
QP_TFMT["$QU_KEY"]=${QU_ITIME}
QP_XTFMT["$QU_KEY"]=${QU_OTIME}
QP_ISTIME["$QU_KEY"]="Y"
```
The y-axis is numeric (uses automatic scientific notation)
```
QP_YFMT["$QU_KEY"]="%g" #${QU_OTIME}
```
Data separator and fonts...
```
QP_DSEP["$QU_KEY"]=","
#QP_KEY_FONT["$QU_KEY"]="arial"
#QP_KEY_SIZE["$QU_KEY"]="9"
#QP_TERM_FONT["$QU_KEY"]="arial"
#QP_TERM_SIZE["$QU_KEY"]="10"
```
Output image size (varies per terminal type, uses QU_TERM_OSIZE, QU_OTERM (imported from qp-shared.conf )...
```
QP_TERM_OSIZE["$QU_KEY"]=${QU_TERM_OSIZE[${QU_OTERM}]}
```
Axis titles, range, scale, offset...
```
QP_XTITLE["$QU_KEY"]="Time (h:m:s)"
#QP_XRANGE_MIN["$QU_KEY"]="\"2019-11-10T00:00:00Z\""
#QP_XRANGE_MAX["$QU_KEY"]=
QP_YTITLE["$QU_KEY"]="Time (s)"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
QP_YRANGE_MAX["$QU_KEY"]=1. #0.8
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=0
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
QP_XSCALE["$QU_KEY"]=1.0
QP_YSCALE["$QU_KEY"]=1.0
QP_XOFS["$QU_KEY"]=0.0
QP_YOFS["$QU_KEY"]=0.0
```
Plot styling (point/line styles, etc.)
```
QP_PLOT_STYLE["$QU_KEY"]="linespoints" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="N"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
```
This plot spec combines 11 traces on a single plot.
Note that the first plot spec uses 
```
QP_PLOT_SPECS[key]="<plot spec string>"
```
and subsequent entries
```
QP_PLOT_SPECS[key]+="+<plot spec string>"
```
to append plot specifiers.  
Note the use of different data files, colors, and dual y-axes, as defined above.
```
QP_PLOT_SPECS["$QU_KEY"]="${QU_MBTRNPP_XT_GETALL_CSV},${QU_BLUE},1,,1,8,x1y1,getall"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_CYCLE_CSV},${QU_ORANGE},1,,1,8,x1y1,cycle"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_STATS_CSV},${QU_YELLOW},1,,1,8,x1y2,stats(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_FWRITE_CSV},${QU_PURPLE},1,,1,8,x1y2,fwrite(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_PING_CSV},${QU_GREEN},1,,1,8,x1y2,ping(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_TRNUPDATE_CSV},${QU_LTBLUE},1,,1,8,,trn-update"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_TRNBIASEST_CSV},${QU_RED},1,,1,8,x1y2,trn-biasest(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_TRNPROC_CSV},'#00ffff',1,,1,8,x1y2,trn-proc(y2)"
#QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_TRNPROC_CSV},${QU_GOLDENROD},1,,1,8,x1y2,trn-proc(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_PROCMB1_CSV},${QU_BLACK},1,,1,8,x1y2,proc-mb1(y2)"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_XT_TRNPROCTRN_CSV},${QU_GRAY},1,,1,8,x1y2,proc-trn(y2)"
```
Two more plots are defined in a similar manner.
Each sets QU_KEY to reference a key (defined above) to index into qplot parameter arrays. 
```
QU_KEY=${QU_KEYS[1]}
QP_OFILE_NAME["$QU_KEY"]="${QU_MBTRNPP_EVENTS_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_MBTRNPP_EVENTS_PTITLE}${QU_MBTRNPP_EVENTS_STITLE}"
QP_TFMT["$QU_KEY"]=${QU_ITIME}
QP_XTFMT["$QU_KEY"]=${QU_OTIME}
QP_ISTIME["$QU_KEY"]="Y"
QP_YFMT["$QU_KEY"]="%g" #${QU_OTIME}
QP_DSEP["$QU_KEY"]=","
#QP_KEY_FONT["$QU_KEY"]="arial"
#QP_KEY_SIZE["$QU_KEY"]="9"
#QP_TERM_FONT["$QU_KEY"]="arial"
#QP_TERM_SIZE["$QU_KEY"]="10"
QP_TERM_OSIZE["$QU_KEY"]=${QU_TERM_OSIZE[${QU_OTERM}]}
QP_XTITLE["$QU_KEY"]="Time (h:m:s)"
#QP_XRANGE_MIN["$QU_KEY"]="\"2019-11-10T00:00:00Z\""
#QP_XRANGE_MAX["$QU_KEY"]=
QP_YTITLE["$QU_KEY"]="mbtrnpp Events"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=1. #0.8
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=0
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
#QP_XSCALE["$QU_KEY"]=1.0
#QP_YSCALE["$QU_KEY"]=1.0
#QP_XOFS["$QU_KEY"]=0.0
#QP_YOFS["$QU_KEY"]=0.0
QP_PLOT_STYLE["$QU_KEY"]="points" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="N"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_PLOT_SPECS["$QU_KEY"]="${QU_MBTRNPP_E_TRNREINIT_CSV},${QU_BLUE},1,,1,5,,reinit"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_E_GAINLO_CSV},${QU_GREEN},1,,1,5,,gainlo"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_E_MBCON_CSV},${QU_ORANGE},1,,1,5,,mbcon"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_E_MBDIS_CSV},${QU_YELLOW},1,,1,5,,mbdis"

QU_KEY=${QU_KEYS[2]}
QP_OFILE_NAME["$QU_KEY"]="${QU_MBTRNPP_ERRORS_OIMG_NAME}"
QP_PTITLE["$QU_KEY"]="${QU_MBTRNPP_ERRORS_PTITLE}${QU_MBTRNPP_ERRORS_STITLE}"
QP_TFMT["$QU_KEY"]=${QU_ITIME}
QP_XTFMT["$QU_KEY"]=${QU_OTIME}
QP_ISTIME["$QU_KEY"]="Y"
QP_YFMT["$QU_KEY"]="%g" #${QU_OTIME}
QP_DSEP["$QU_KEY"]=","
#QP_KEY_FONT["$QU_KEY"]="arial"
#QP_KEY_SIZE["$QU_KEY"]="9"
#QP_TERM_FONT["$QU_KEY"]="arial"
#QP_TERM_SIZE["$QU_KEY"]="10"
QP_TERM_OSIZE["$QU_KEY"]=${QU_TERM_OSIZE[${QU_OTERM}]}
QP_XTITLE["$QU_KEY"]="Time (h:m:s)"
#QP_XRANGE_MIN["$QU_KEY"]="\"2019-11-10T00:00:00Z\""
#QP_XRANGE_MAX["$QU_KEY"]=
QP_YTITLE["$QU_KEY"]="mbtrnpp Errors"
#QP_YRANGE_MIN["$QU_KEY"]=6.5
#QP_YRANGE_MAX["$QU_KEY"]=1. #0.8
#QP_X2TITLE["$QU_KEY"]=""
#QP_X2RANGE_MIN["$QU_KEY"]=0
#QP_X2RANGE_MAX["$QU_KEY"]=10
#QP_Y2TITLE["$QU_KEY"]="Time (s)"
#QP_Y2RANGE_MIN["$QU_KEY"]=0
#QP_Y2RANGE_MAX["$QU_KEY"]=8.8
#QP_XSCALE["$QU_KEY"]=1.0
#QP_YSCALE["$QU_KEY"]=1.0
#QP_XOFS["$QU_KEY"]=0.0
#QP_YOFS["$QU_KEY"]=0.0
QP_PLOT_STYLE["$QU_KEY"]="points" #lines
QP_POINTSIZE["$QU_KEY"]=${QU_POINT_SIZE}
QP_POINTTYPE["$QU_KEY"]=${QU_POINT_TYPE}
QP_POINTCOLOR["$QU_KEY"]=${QU_POINT_COLOR}
QP_INC_POINTTYPE["$QU_KEY"]="N"
QP_INC_POINTCOLOR["$QU_KEY"]="N"
QP_USE_LINETYPES["$QU_KEY"]="N"
QP_LINETYPE["$QU_KEY"]=1
QP_INC_LINETYPE["$QU_KEY"]="N"
#QP_LINE_TYPES["$QU_KEY"]="${QU_LINE_TYPE_DFL}"
QP_PLOT_SPECS["$QU_KEY"]="${QU_MBTRNPP_E_EMBGETALL_CSV},${QU_BLUE},1,,1,5,,embgetall"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_E_EMBFAIL_CSV},${QU_GREEN},1,,1,5,,embfail"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_E_EMBSOCK_CSV},${QU_ORANGE},1,,1,5,,embsock"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_E_EMBCON_CSV},${QU_YELLOW},1,,1,5,,embcon"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_E_EMBFRAMERD_CSV},${QU_RED},1,,1,5,,embframerd"
QP_PLOT_SPECS["$QU_KEY"]+="+${QU_MBTRNPP_E_EMBLOGWR_CSV},${QU_PURPLE},1,,1,5,,emblogwr"
```
By default, plot specifiers assume x and y column values to be integer column numbers.
Scaling and offset may be applied using QP_XSCALE, QP_YSCALE, QP_XOFS, QP_YSCALE,  QP_YOFS.
The scaling parameters affect all traces in a plot.

Alternatively, plots specifiers support functions using data columns as parameters for the y column(s).
To do this, set QP_EXPR to "Y"; if addition is used in the expression, change the plot spec delimiter from '+' to '|'
or other character using QP_SPECDEL, and use that character if there are multiple  plot specifiers.
Note that the first specifier should not begin with the delimiter (errors may occur if it is included).  
Expressions must use correct gnuplot `plot using` expression syntax: 
* enclosed in parentheses
* '$' must be escaped "\$"
* operators must be separated by whitespace 
* gnuplot functions may be called, e.g. sin(\$3)
If QP_EXPR is set to "Y", all plot specifiers in that plot must use expression syntax (e.g. "(\$4)" not "4").
QP_EXPR is ignored for the xcol if QP_ISTIME=="Y".

Example:  
```
QP_EXPR["$QU_KEY"]="Y"
QP_SPECDEL["$QU_KEY"]="|"
# add small offset to separate traces; read nearest integer below trace
QP_PLOT_SPECS["$QU_KEY"]="${QU_TRNU_STATE_CSV},${QU_BLUE},14,,1,(\$4 + 0.1),,reinit"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNU_STATE_CSV},${QU_ORANGE},14,,1,(\$6 + 0.2),,fstate"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNU_STATE_CSV},${QU_YELLOW},14,,1,(\$8 + 0.3),,lms"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNU_STATE_CSV},${QU_PURPLE},14,,1,(\$18 + 0.4),,isconv"
QP_PLOT_SPECS["$QU_KEY"]+="|${QU_TRNU_STATE_CSV},${QU_BLACK},14,,1,(\$20 + 0.5),,isval"
```
