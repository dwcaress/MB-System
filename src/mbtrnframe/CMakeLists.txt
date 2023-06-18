#------------------------------------------------------------------------------
#
# Configurable error and debugging output library generally called mframe but
# here named mbtrnframe (mbsystem/src/mbtrnframe) and used by code in
# mbsystem/src/mbtrn mbsystem/src/mbtrnav mbsystem/src/mbtrnutils
#
message("In mbtrnutils")
#
#------------------------------------------------------------------------------
message("In mbtrnframe")
#------------------------------------------------------------------------------
#
add_library(
  mbtrnframe
  mframe.c
  mbbuf.c
  mcbuf.c
  msocket.c
  mthread.c
  mfile.c
  mtime.c
  mmdebug.c
  merror.c
  mlog.c
  mlist.c
  mstats.c
  mswap.c
  mutils.c
  mkvconf.c
  mconfig.c)
target_link_libraries(mbtrnframe PRIVATE m)
#
#------------------------------------------------------------------------------
#
# install it all
install(TARGETS mbtrnframe DESTINATION ${CMAKE_INSTALL_LIBDIR})
#
#------------------------------------------------------------------------------
