#!/bin/bash

#########################################
# Name: mkdoc.sh
#
# Summary: convert markdown to html, PDF using pandoc
#
# Description: Build HTML and PDF doc from markdown
#  
#  ./mkdoc/mkdoc.sh -vi hw-checkout-guide/hw-checkout-guide.md  -a hw-checkout-guide/media
#  ./mkdoc/mkdoc.sh -vi controller-help/controller-help.md
#  ./mkdoc/mkdoc.sh -vi system-guide/system-guide.md  -a system-guide/examples -a system-guide/media
#
# Author: kheadley
#
# Copyright 2020 MBARI
#
#########################################

#########################################
# Script configuration defaults
# casual users should not need to change
# anything below this section
#########################################
description="Convert markdown to HTML, PDF using pandoc"

MKDOC_HOME_DFL=$(cd "$(dirname ${0})" && pwd)
MKDOC_HOME=${MKDOC_HOME:-${MKDOC_HOME_DFL}}
VERBOSE="N"
STYLE=radar.css
IFILE=""
OFILE=""
TITLE=""
MKDOC_OPATH=${MKDOC_OPATH:-""}
STYLE_PATH="${MKDOC_HOME}/../doc/styles"
OPT_STANDALONE="-s"
TEST="N"
declare -a assets
MARGIN_L=""
MARGIN_R=""
MARGIN_T="20mm"
MARGIN_B="20mm"
OPT_MARGIN_L=""
OPT_MARGIN_R=""
OPT_MARGIN_T="--margin-top 20mm"
OPT_MARGIN_B="--margin-bottom 20mm"
TXT_ASSET_PATTERN="*txt"
PDF_ASSET_PATTERN="*pdf"
PANDOC=$(which pandoc)
HTMLTOPDF=$(which wkhtmltopdf)
GS=$(which gs)

MKDOC_SESSION=${MKDOC_SESSION:-$(date +%d%m%y-%H%M%S)}

#################################
# name: printUsage
# description: print use message
# args: none
#################################
printUsage(){
    echo
    echo " Description: $description"
    echo
    echo " usage: `basename $0` [options]"
    echo " Options:"
    echo "  -h    : print use message"
    echo "  -a p  : asset (file or directory)"
    echo "          may include more than once"
    echo "  -i f  : input file              [${IFILE}]"
    echo "  -o f  : output file             [${OFILE}]"
    echo "  -s s  : style                   [${STYLE%%.css}]"
	echo "           options                [${STYLE_LIST}]"
    echo "  -S p  : style path              [${STYLE_PATH}]"
	echo "  -t s  : title                   [${TITLE}]"
    echo "  -L m  : left margin   e.g. 20mm [${MARGIN_L}]"
    echo "  -R m  : right margin  e.g. 20mm [${MARGIN_R}]"
    echo "  -T m  : top margin    e.g. 20mm [${MARGIN_T}]"
    echo "  -B m  : bottom margin e.g. 20mm [${MARGIN_B}]"
    echo "  -N    : test                    [${TEST}]"
    echo ""
    echo " Examples:"
    echo "  src/mbtrn/tools/bin/mkdoc.sh -vi README-mbtrncfg.md"
    echo ""
    echo
}


########################################
# name: vout
# description: print verbose message to stderr
# args:
#     msg: message
########################################
vout(){
if [ "${VERBOSE}" == "Y" ] || [ "${VERBOSE}" == "TRUE" ]
then
	echo "$1" >&2
fi
}

########################################
# name: processCmdLine
# description: do command line processsing
# args:
#     args:       positional paramters
#     returnCode: none
########################################
processCmdLine(){
    OPTIND=1
    vout "`basename $0` all args[$*]"

while getopts a:hi:L:R:T:B:o:S:s:Nt:v Option
    do
        vout "processing $Option[$OPTARG]"
        case $Option in
			a ) assets[${#assets}]=$OPTARG
            ;;
            i ) IFILE=$OPTARG
            ;;
            o ) OFILE=$OPTARG
            ;;
            s ) STYLE=$OPTARG
            ;;
            S ) STYLE_PATH=$OPTARG
                mkStyleList
            ;;
            t ) TITLE=$OPTARG
            ;;
            L ) MARGIN_L="$OPTARG"
				OPT_MARGIN_L="--margin-left $OPTARG"
            ;;
            R ) MARGIN_R="$OPTARG"
                OPT_MARGIN_R="--margin-right $OPTARG"
            ;;
            T ) MARGIN_T="$OPTARG"
                OPT_MARGIN_T="--margin-top $OPTARG"
            ;;
            B ) MARGIN_B="$OPTARG"
                OPT_MARGIN_B="--margin-bottom $OPTARG"
            ;;
            N ) TEST="Y"
            ;;
            h) mkStyleList
                printUsage
                exit 0
            ;;
            v) VERBOSE="Y"
            ;;
            *) exit 0 # getopts outputs error message
            ;;
        esac
        done
}

########################################
# name: mkStyleList
# description: generate list of styles
# args:
########################################
mkStyleList(){
    STYLE_LIST=""
    for style in $(ls ${STYLE_PATH}/*.css)
    do
        sname=$(basename ${style})
        STYLE_LIST="${STYLE_LIST} ${sname%%.css} "
    done
}

# Main Entry Point

processCmdLine "$@"

mkStyleList
IS_VALID="Y"

# validate input
if [ -z "${IFILE}" ] || [ ! -f "${IFILE}" ]
then
# input file required
echo ""
echo " ERR - input file missing/invalid [$IFILE]"
IS_VALID="N"
else
if [ -z "${OFILE}" ]
then
TMP=`basename $IFILE`
OFILE=${TMP%%.*}.html
fi

if [ -z "${TITLE}" ]
then
TMP=`basename $IFILE`
TITLE=${TMP%%.*}
fi

fi



if [ -z "${MKDOC_OPATH}" ]
then
MKDOC_OPATH=${MKDOC_OPATH:-"cwgdoc-${MKDOC_SESSION}"}
fi

OFNAME=${OFILE%%.*}
XFNAME=${OFNAME}-xa

CMD_LINE=" pandoc ${OPT_STANDALONE}  -c ${STYLE}  ${IFILE} --metadata title=${TITLE} -o ${HTML_OPATH}/${OFNAME}.html"

vout ""
vout " MKDOC_SESSION [${MKDOC_SESSION}]"
vout " PANDOC        [${PANDOC}]"
vout " HTMLTOPDF     [${HTMLTOPDF}]"
vout " GS            [${GS}]"
vout " IFILE         [${IFILE}]"
vout " OFILE         [${OFILE}]"
vout " TITLE         [${TITLE}]"
vout " STYLE         [${STYLE%%.css}]"
vout " MARGIN_L      [${MARGIN_L}]"
vout " MARGIN_R      [${MARGIN_R}]"
vout " MARGIN_T      [${MARGIN_T}]"
vout " MARGIN_B      [${MARGIN_B}]"
vout " MKDOC_OPATH   [${MKDOC_OPATH}]"
vout " STYLE_PATH    [${STYLE_PATH}]"
vout " STYLE_LIST    [${STYLE_LIST}]"
vout " assets        [ ${assets[*]} ]"

if [ "${TEST}" == "Y" ]
then
    echo ""
    echo "using ${CMD_LINE}"
    echo ""
else
    vout "using ${CMD_LINE}"
    vout ""
fi

if [ ${IS_VALID} != "Y" ]
then
    echo "There are errors - exiting"
    echo ""
	exit -1
fi

if [ "${TEST}" != "Y" ]
then

    if [ ! -d "${MKDOC_OPATH}" ]
    then
        mkdir -p ${MKDOC_OPATH}
        if [ ! -d "${MKDOC_OPATH}" ]
        then
            echo "Could not create directory ${MKDOC_OPATH}"
        exit -1
        fi
    fi

	# make subdirs for html, pdf output
    HTML_OPATH=${MKDOC_OPATH}/html
    PDF_OPATH=${MKDOC_OPATH}/pdf
    mkdir -p ${PDF_OPATH}
    mkdir -p ${HTML_OPATH}

	# translate style path to styles in output dir
	mkdir ${MKDOC_OPATH}/styles
	cp -R ${STYLE_PATH}/${STYLE} ${MKDOC_OPATH}/styles/${STYLE}

	# generate HTML
    ${PANDOC} ${OPT_STANDALONE} -c ${STYLE_PATH}/${STYLE}  ${IFILE} --metadata title="${TITLE}" -o ${HTML_OPATH}/${OFNAME}.html

    # copy assets
    for asset in ${assets[@]}
    do
        vout "copying asset[$asset] "
        if [ -d "${asset}" ]
        then
            cp -R ${asset} ${MKDOC_OPATH}
        elif [ -f  "${asset}" ]
        then
            cp ${asset} ${MKDOC_OPATH}
        else
			echo "asset [$asset] not regular file or directory - skipping"
        fi
    done

	# make temporary directory and asset arrays
    declare -a pdf_assets
    MKDOC_TMP=${MKDOC_OPATH}/mkdoc-tmp
    mkdir ${MKDOC_TMP}

    # iterate over assets,
    for asset in ${assets[@]}
    do
        vout "processing asset[$asset]"

        # add PDF assets to array
        for pdf_asset in $(find ${asset} -name ${PDF_ASSET_PATTERN})
        do
            vout "pdf asset[$pdf_asset]"
            if [ -f $pdf_asset ]
            then
                pdf_assets[${#pdf_assets}]=$pdf_asset
            fi
        done

        # convert scripts to HTML/PDF
        for text_asset in $(find ${asset} -name ${TXT_ASSET_PATTERN})
        do
            vout "text asset[$text_asset]"
            asset_bname=`basename ${text_asset}`
            asset_name=${asset_bname%%.*}
            if [ -f $text_asset ]
            then
                echo "<html><head></head><body><div># ${asset_bname}</div><pre>$(cat ${text_asset})</pre></body></html>" > ${MKDOC_TMP}/${asset_name}.html
                ${PANDOC} ${MKDOC_TMP}/${asset_name}.html -o ${MKDOC_TMP}/${asset_name}.pdf
            fi
        done
    done

    vout "pdf_assets ${#pdf_assets} ${pdf_assets}"
    vout "text_assets ${#text_assets} ${text_assets}"

    # generate output PDF
    URL_PATH=$(cd "$(dirname ${HTML_OPATH}/${OFNAME}.html)" && pwd)

    ${HTMLTOPDF} -s Letter ${OPT_MARGIN_L} ${OPT_MARGIN_R} ${OPT_MARGIN_T} ${OPT_MARGIN_B} --enable-local-file-access  ${URL_PATH}/${OFNAME}.html  ${PDF_OPATH}/${XFNAME}.pdf

    # combine PDF media resources (include PDF and converted script assets)
    ${GS} -sDEVICE=pdfwrite -dNOPAUSE -dBATCH -dSAFER -sOutputFile=${PDF_OPATH}/${OFNAME}.pdf ${PDF_OPATH}/${XFNAME}.pdf ${pdf_assets[*]} $(ls ${MKDOC_TMP}/*pdf)

    # remove production products
    rm -rf ${MKDOC_TMP}
    rm -rf ${PDF_OPATH}/${XFNAME}.pdf
fi

