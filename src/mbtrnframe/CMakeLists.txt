add_library(
  mframe
  mframe/src/mframe.c
  mframe/src/mbbuf.c
  mframe/src/mcbuf.c
  mframe/src/msocket.c
  mframe/src/mthread.c
  mframe/src/mfile.c
  mframe/src/mtime.c
  mframe/src/mmdebug.c
  mframe/src/merror.c
  mframe/src/mlog.c
  mframe/src/mlist.c
  mframe/src/mstats.c
  mframe/src/mswap.c
  mframe/src/mutils.c
  mframe/src/mkvconf.c
  usr/mconfig.c)
target_include_directories(mframe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/usr
                                         ${CMAKE_CURRENT_SOURCE_DIR}/mframe/src)
target_link_libraries(mframe)

install(TARGETS mframe DESTINATION ${CMAKE_INSTALL_LIBDIR})
