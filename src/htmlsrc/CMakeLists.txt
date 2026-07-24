#------------------------------------------------------------------------------
#    The MB-system:  CMakeLists.txt   23 July 2026
#
#    Copyright (c) 2026 by
#    David W. Caress (caress@mbari.org)
#      Monterey Bay Aquarium Research Institute
#      Moss Landing, California, USA
#    Dale N. Chayes
#      Center for Coastal and Ocean Mapping
#      University of New Hampshire
#      Durham, New Hampshire, USA
#    Christian dos Santos Ferreira
#      MARUM
#      University of Bremen
#      Bremen Germany
#
#    MB-System was created by Caress and Chayes in 1992 at the
#      Lamont-Doherty Earth Observatory
#      Columbia University
#      Palisades, NY 10964
#
#    See README.md file for copying and redistribution conditions.
#------------------------------------------------------------------------------

message("In src/htmlsrc")

# man2html: converts troff man pages to html. Used only by the optional
# "htmldocs" target below - not part of the MB-System distribution itself.
add_executable(man2html man2html.c)

# Optional target to regenerate src/html/*.html and src/html/*.pdf from the
# man pages in src/man/ plus ChangeLog.md/COPYING.md/GPL.md, replacing the
# previously-manual make_mbhtml workflow. Not part of the default build -
# run explicitly via:
#   cmake --build . --target htmldocs
find_program(PERL_EXECUTABLE perl)
find_program(GROFF_EXECUTABLE groff)
find_program(PANDOC_EXECUTABLE pandoc)
find_program(WEASYPRINT_EXECUTABLE weasyprint)
if(PERL_EXECUTABLE AND GROFF_EXECUTABLE AND PANDOC_EXECUTABLE AND WEASYPRINT_EXECUTABLE)
  file(GLOB MBHTML_MAN_PAGES
      ${CMAKE_SOURCE_DIR}/src/man/man1/mb*
      ${CMAKE_SOURCE_DIR}/src/man/man3/mb*)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/htmldocs.stamp
    COMMAND ${PERL_EXECUTABLE} make_mbhtml.cmd
            $<TARGET_FILE:man2html>
            $<TARGET_FILE:mbformat>
            ${GROFF_EXECUTABLE}
            ${PANDOC_EXECUTABLE}
            ${WEASYPRINT_EXECUTABLE}
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/htmldocs.stamp
    DEPENDS man2html mbformat
            ${MBHTML_MAN_PAGES}
            ${CMAKE_CURRENT_SOURCE_DIR}/mbsystem_man_hdr.frag
            ${CMAKE_CURRENT_SOURCE_DIR}/mbsystem_man_hdr2.frag
            ${CMAKE_CURRENT_SOURCE_DIR}/mbsystem_man_ftr.frag
            ${CMAKE_SOURCE_DIR}/ChangeLog.md
            ${CMAKE_SOURCE_DIR}/COPYING.md
            ${CMAKE_SOURCE_DIR}/GPL.md
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Regenerating src/html man page HTML/PDF documentation via make_mbhtml"
    VERBATIM)
  add_custom_target(htmldocs DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/htmldocs.stamp)
else()
  message(STATUS "perl, groff, pandoc, and/or weasyprint not found - 'htmldocs' build "
                  "target unavailable (install via: brew install groff pandoc weasyprint)")
endif()
