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

SUBDIRS =
if BUILD_GSF
SUBDIRS += gsf
endif
SUBDIRS += surf
SUBDIRS += bsio
SUBDIRS += $(XBUILD_SUB_MBTNAV)
SUBDIRS += $(XBUILD_SUB_MBTRN)
SUBDIRS += mbio
SUBDIRS += mbaux
SUBDIRS += utilities
SUBDIRS += $(XBUILD_SUB_MBTRNBIN)
SUBDIRS += gmt
SUBDIRS += otps
SUBDIRS += macros
SUBDIRS += share
SUBDIRS += man
SUBDIRS += html
SUBDIRS += ps
SUBDIRS += $(XBUILD_SUB_MOTIF)
SUBDIRS += $(XBUILD_SUB_GL)

CLEANFILES =
DISTCLEANFILES =
