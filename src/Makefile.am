if BUILD_MOTIF
  XBUILD_SUB_MOTIF = mbedit mbnavedit mbvelocitytool
endif
if BUILD_GL
  XBUILD_SUB_GL = mbview mbgrdviz mbeditviz mbnavadjust
endif
if BUILD_MBTRN
  XBUILD_SUB_MBTRN = mbtrn
endif
if BUILD_MBTNAV
  XBUILD_SUB_MBTNAV = mbtnav
endif

SUBDIRS = gsf surf bsio $(XBUILD_SUB_MBTRN) $(XBUILD_SUB_MBTNAV) mbio mbaux \
	utilities gmt otps macros share man html ps \
	$(XBUILD_SUB_MOTIF) $(XBUILD_SUB_GL)

CLEANFILES =
DISTCLEANFILES =
