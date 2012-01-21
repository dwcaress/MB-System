include_HEADERS = mbsystem_config.h

if BUILD_MOTIF
  XBUILD_SUB = mbedit mbnavedit mbvelocitytool mbnavadjust
endif
if BUILD_GL
  XBUILD_SUB_GL = mbview
endif
SUBDIRS = gsf surf mr1pr proj mbio mbaux utilities gmt macros $(XBUILD_SUB) otps $(XBUILD_SUB_GL)
