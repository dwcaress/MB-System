##########################################################################
# Copyright 1999 MBARI                                             
##########################################################################
# Summary  : Makefile for taskInterface subdirectory
# Filename : Makefile
# Author   : Andrew Pearce
# Project  : 
# Version  : Version 1.0
# Created  : 12/06/99
# Modified : 12/06/99
# Archived :
##########################################################################
# Modification History:
# $Header$
#
##########################################################################

CC	    = gcc
CCPP	= gcc
LD	    = gcc
AR 	    = ar -r
DEPEND_TOOL = makedepend
JAVAC		= javac
LCM_GEN		= lcm-gen
JAR			= jar
LCM_CPP_OPTS	= -x --c-typeinfo -j
LCM_PYT_OPTS	= -p 
LCM_OPTS	= -x --c-typeinfo -j
JAVAC_OPTS	= -cp $(CLASSPATH):/usr/local/share/java/lcm.jar
JAR_OPTS	= cvf

# Module name used for output directory and JAR
MODULE		= lcmMessages
BIN_DIR 	= $(MODULE)
JAR_FILE	= $(MODULE).jar

DEBUG 	    = -g
EXCEPTION_HANDLING =

# LCM source files to process
LCM_SRC	= dataVectors.lcm


AUTOCODE = lcmMessages/DataVectors.hpp \
	lcmMessages/IntVector.hpp \
	lcmMessages/FloatVector.hpp \
	lcmMessages/DoubleVector.hpp \
	lcmMessages/StringVector.hpp \

all: $(AUTOCODE)

# Generate C++ and Python code from .lcm file
$(AUTOCODE): $(LCM_SRC)
	$(LCM_GEN) $(LCM_CPP_OPTS) $(LCM_SRC)
	$(LCM_GEN) $(LCM_PYT_OPTS) $(LCM_SRC)

##########################################################################
# Do not modify anything below this line unless absolutely necessary

.SUFFIXES:
.SUFFIXES:	.o .c .cc .d .idl .lib .hpp .lcm

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@
	@echo

%.o: %.cc
	$(CCPP) $(CPPFLAGS) $(INCLUDES) $< -o $@
	@echo Done compiling

DEST	    = .

MAKEFILE    = Makefile

depend:

clean:
	rm -f core *.o *.d *.lib *.yy *.tab.* *.cc *.h *.jar $(BIN_DIR)/*.hpp $(BIN_DIR)/*.class $(BIN_DIR)/*.java $(BIN_DIR)/*.py

##########################################################################

