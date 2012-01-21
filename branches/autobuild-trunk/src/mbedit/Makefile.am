bin_PROGRAMS = mbedit

include_HEADERS = 

INCLUDES = -I.. -I../mbio -I../mbaux -I../surf -I../gsf @MOTIF_INC_DIR@

LIBS = @MOTIF_LIB_DIR@

mbedit_SOURCES = mbedit.c mbedit_bxutils.c mbedit_callbacks.c mbedit_creation.c mbedit_prog.c mbedit_creation.h mbedit.h
mbedit_LDADD = -L../mbio -L../gsf -L../proj -L../mr1pr -L../surf -L../mbaux @MOTIF_LIB_DIR@ -lmbio -lmbproj -lmbgsf -lmbmr1pr -lmbsapi -lmbxgr @MOTIF_LIB@

