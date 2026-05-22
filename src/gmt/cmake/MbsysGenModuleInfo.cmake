# MbsysGenModuleInfo.cmake — generate gmt_mbsystem_moduleinfo.h for the
# MB-System out-of-tree GMT supplement.
#
# Invoke with `cmake -P MbsysGenModuleInfo.cmake -D SRC_DIR=<dir>
#   -D OUTPUT_FILE=<path> -D PROGS_SRCS="<a.c;b.c;...>"`.
#
# Scrapes THIS_MODULE_MODERN_NAME / CLASSIC_NAME / LIB / PURPOSE / KEYS from
# each source. Falls back to THIS_MODULE_NAME when the modern/classic dual
# macros are absent (most MB-System ports still use the legacy single-name
# macro). Output is the body of a `static struct GMT_MODULEINFO modules[]`
# initializer; gmt_mbsystem_glue.c #includes it directly.

if (NOT DEFINED OUTPUT_FILE)
    message (FATAL_ERROR "MbsysGenModuleInfo: OUTPUT_FILE not set")
endif ()
if (NOT DEFINED SRC_DIR)
    message (FATAL_ERROR "MbsysGenModuleInfo: SRC_DIR not set")
endif ()
if (NOT DEFINED PROGS_SRCS)
    message (FATAL_ERROR "MbsysGenModuleInfo: PROGS_SRCS not set")
endif ()

separate_arguments (PROGS_SRCS)

set (_moduleinfo "")
foreach (_prog_src ${PROGS_SRCS})
    file (READ "${SRC_DIR}/${_prog_src}" _src)

    set (_modern "")
    set (_classic "")
    set (_legacy "")
    set (_lib "")
    set (_purpose "")
    set (_keys "")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_MODERN_NAME[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_modern "${CMAKE_MATCH_1}")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_CLASSIC_NAME[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_classic "${CMAKE_MATCH_1}")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_NAME[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_legacy "${CMAKE_MATCH_1}")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_LIB[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_lib "${CMAKE_MATCH_1}")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_PURPOSE[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_purpose "${CMAKE_MATCH_1}")

    string (REGEX MATCH "#define[ \t]+THIS_MODULE_KEYS[ \t]+\"([^\"\n]*)\"" _m "${_src}")
    set (_keys "${CMAKE_MATCH_1}")

    if ("${_modern}" STREQUAL "")
        set (_modern "${_legacy}")
    endif ()
    if ("${_classic}" STREQUAL "")
        set (_classic "${_legacy}")
    endif ()

    if ("${_modern}" STREQUAL "")
        message (WARNING "MbsysGenModuleInfo: ${_prog_src} has no THIS_MODULE_*_NAME — skipping")
        continue ()
    endif ()
    if ("${_lib}" STREQUAL "")
        message (WARNING "MbsysGenModuleInfo: ${_prog_src} has no THIS_MODULE_LIB — skipping")
        continue ()
    endif ()
    if ("${_purpose}" STREQUAL "")
        message (WARNING "MbsysGenModuleInfo: ${_prog_src} has no THIS_MODULE_PURPOSE")
    endif ()

    set (_row "\t{\"${_modern}\", \"${_classic}\", \"${_lib}\", \"${_purpose}\", \"${_keys}\"},")
    if (_moduleinfo)
        set (_moduleinfo "${_moduleinfo}\n${_row}")
    else ()
        set (_moduleinfo "${_row}")
    endif ()
endforeach ()

file (WRITE "${OUTPUT_FILE}" "${_moduleinfo}\n")
