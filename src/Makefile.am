if BUILD_MOTIF
  XBUILD_SUB_MOTIF = mbedit mbnavedit mbvelocitytool mbnavadjust
endif
if BUILD_GL
  XBUILD_SUB_GL = mbview mbgrdviz mbeditviz
endif
if BUILD_PROJ
  XBUILD_SUB_PROJ = proj
endif

SUBDIRS = $(XBUILD_SUB_PROJ) gsf surf bsio mbio mbaux \
	utilities gmt otps macros \
	$(XBUILD_SUB_MOTIF) $(XBUILD_SUB_GL) \
	man html ps share

