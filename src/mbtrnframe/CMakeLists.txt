message("In mbtrnframe")

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

install(TARGETS mbtrnframe DESTINATION ${CMAKE_INSTALL_LIBDIR})
