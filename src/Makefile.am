if BUILD_MOTIF
  XBUILD_SUB_MOTIF = mbedit mbnavedit mbvelocitytool
else
  XBUILD_SUB_MOTIF = 
endif
if BUILD_GL
  XBUILD_SUB_GL = mbview mbgrdviz mbeditviz mbnavadjust
else
  XBUILD_SUB_GL = 
endif
if BUILD_MBTRN
  XBUILD_SUB_MBTRN = mbtrn
else
  XBUILD_SUB_MBTRN = 
endif

SUBDIRS = gsf surf bsio mbio mbaux \
	utilities gmt otps macros share man html ps \
	$(XBUILD_SUB_MOTIF) $(XBUILD_SUB_GL) $(XBUILD_SUB_MBTRN)

CLEANFILES =
DISTCLEANFILES = 
