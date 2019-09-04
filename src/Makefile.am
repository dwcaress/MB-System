if BUILD_MOTIF
  XBUILD_SUB_MOTIF = mbedit mbnavedit mbvelocitytool
endif
if BUILD_GL
  XBUILD_SUB_GL = mbview mbgrdviz mbeditviz mbnavadjust
endif
if BUILD_MBTRN
  XBUILD_SUB_MBTRN = mbtrn
endif

SUBDIRS =
SUBDIRS += gsf
SUBDIRS += surf
SUBDIRS += bsio
SUBDIRS += $(XBUILD_SUB_MBTRN)
SUBDIRS += mbio
SUBDIRS += mbaux
SUBDIRS += utilities
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
