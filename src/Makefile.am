if BUILD_MBUTILS
if BUILD_MOTIF
  XBUILD_SUB_MOTIF = mbedit mbnavedit mbvelocitytool
endif
if BUILD_GL
  XBUILD_SUB_GL = mbview mbgrdviz mbeditviz mbnavadjust
endif
endif
if BUILD_MBTRN
  XBUILD_SUB_MBTRN = mbtrn
  XBUILD_SUB_MBTRNBIN = mbtrnutils
endif
if BUILD_MBTNAV
  XBUILD_SUB_MBTNAV = mbtrnav
  XBUILD_SUB_MBTRNBIN = mbtrnutils
endif

SUBDIRS = gsf surf bsio $(XBUILD_SUB_MBTNAV) $(XBUILD_SUB_MBTRN) mbio mbaux  \
	utilities $(XBUILD_SUB_MBTRNBIN) gmt otps macros share man html ps \
	$(XBUILD_SUB_MOTIF) $(XBUILD_SUB_GL)

CLEANFILES =
DISTCLEANFILES =
