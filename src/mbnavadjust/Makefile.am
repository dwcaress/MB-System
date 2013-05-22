bin_PROGRAMS = mbnavadjust

include_HEADERS = 

AM_CPPFLAGS = -I.. -I../mbio -I../mbaux -I../surf -I../gsf @MOTIF_INC_DIR@

LIBS = @MOTIF_LIB_DIR@

mbnavadjust_SOURCES = mbnavadjust.c mbnavadjust_callbacks.c mbnavadjust_creation.c \
	mbnavadjust_prog.c mbnavadjust_util.c mbnavadjust_creation.h \
	mbnavadjust_extrawidgets.h mbnavadjust.h
	
mbnavadjust_LDADD = -L../mbio -L../gsf -L../proj -L../mr1pr -L../surf -L../mbaux @MOTIF_LIB_DIR@ -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux -lmbxgr -lXm -lXt -lX11
