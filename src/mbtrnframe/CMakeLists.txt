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
add_compile_options(
  -DHAVE_CONFIG_H  
  -D_GNU_SOURCE 
  -DTRNC_VER=1.0 
  -DFRAMES7K_VER=1.0 
  -DSTREAM7K_VER=1.0 
  -DTBINX_VER=1.0 
  -DEMU7K_VER=1.0 
  -DWITH_TESTS 
  -DWITH_R7KR_TEST 
  -DWITH_MB1R_TEST 
  -DWITH_MST_MSTATS 
  -DMST_STATS_EN 
  -DMXDEBUG_C_API)
add_library(
  mbtrnframe
  mbbuf.c
  mcbuf.c
  mconfig.c
  merror.c
  mfile.c
  mkvconf.c
  mlist.c
  mlog.c
  mmdebug.c
  msocket.c
  mstats.c
  mswap.c
  mthread.c
  mtime.c
  mutils.c
  mxdebug.cpp)
target_link_libraries(mbtrnframe PRIVATE m pthread)
#
#------------------------------------------------------------------------------
#
# install it all
install(TARGETS mbtrnframe DESTINATION ${CMAKE_INSTALL_LIBDIR})
#
#------------------------------------------------------------------------------
