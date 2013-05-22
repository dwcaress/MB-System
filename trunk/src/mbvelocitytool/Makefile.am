bin_PROGRAMS = mbvelocitytool

include_HEADERS = 

AM_CPPFLAGS = -I.. -I../mbio -I../mbaux -I../surf -I../gsf @MOTIF_INC_DIR@

LIBS = @MOTIF_LIB_DIR@

mbvelocitytool_SOURCES = mbvelocity_bxutils.c mbvelocity.c mbvelocity_callbacks.c \
mbvelocity_creation.c mbvelocity_prog.c mbvelocity_creation.h mbvelocity.h
mbvelocitytool_LDADD = -L../mbio -L../gsf -L../proj -L../mr1pr -L../surf -L../mbaux @MOTIF_LIB_DIR@ -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbxgr -lXm -lXt -lX11
