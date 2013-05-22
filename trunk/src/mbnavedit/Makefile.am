bin_PROGRAMS = mbnavedit

include_HEADERS = 

AM_CPPFLAGS = -I.. -I../mbio -I../mbaux -I../surf -I../gsf @MOTIF_INC_DIR@

LIBS = @MOTIF_LIB_DIR@

mbnavedit_SOURCES = mbnavedit.c mbnavedit_creation.c mbnavedit_util.c mbnavedit_callbacks.c \
	mbnavedit_prog.c mbnavedit_creation.h mbnavedit_extrawidgets.h mbnavedit.h
	
mbnavedit_LDADD = -L../mbio -L../gsf -L../proj -L../mr1pr -L../surf -L../mbaux @MOTIF_LIB_DIR@ -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbaux -lmbxgr -lXm -lXt -lX11

