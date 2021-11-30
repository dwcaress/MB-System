########################################
# Build Environment
# version and build configuration

# uncomment and optionally define on command line
# to override value defined in source, e.g.
#   trn_ver=1.0.1 make
trn_ver ?=0.0.1
trn_build=`date +%Y/%m/%dT%H:%M:%S%z`
OPT_TRN_VER=-DLIBTRN_VERSION=$(trn_ver)
OPT_TRN_BUILD=-DLIBTRN_BUILD=$(trn_build)

# C standard e.g. -std=c99 -std=gnu99
# may be needed for Cygwin (e.g. for loop declare/init)
#STD= -std=c99

# Platform-specific options
ifeq ($(shell uname),Linux)
 OS_CFLAGS=
 OS_INC_PATH=
 OS_LIB_PATH=
 OS_LIBS=-lrt
endif

ifeq ($(shell uname),Darwin)
OS_CFLAGS=
OS_INC_PATH=-I/opt/local/include
OS_LIB_PATH=-L/opt/local/lib
OS_LIBS=
endif

ifeq ($(shell uname),Cygwin)
OS_CFLAGS=
OS_INC_PATH=-I/opt/local/include
OS_LIB_PATH=-L/opt/local/lib
OS_LIBS=-lrt
endif

# Build options
BUILD_OPTIONS = $(STD) -D_GNU_SOURCE  $(OPT_TRN_VER) $(OPT_TRN_BUILD)

# Build directories
OUTPUT_DIR=../../bin
BUILD_DIR=../../build
QNX_DIR=../../qnx-utils
NEWMAT_DIR=../../newmat
TRN_DIR=../../terrain-nav
UTILS_DIR=../../utils

# Compilation Options
CXX = g++ #gcc
AR = ar
# compile and link with -pg for profiling support
# then do gprof <application> gmon.out for profile output to console
#GPROF= -pg
# use -Wall unless you know what you're doing
WARN_FLAGS=-Wall

SANI_FLAG =
#SANI_FLAG += -fsanitize=address

# Compiler flags
CFLAGS = -g -O2 $(SANI_FLAG) $(WARN_FLAGS) $(GPROF) $(BUILD_OPTIONS) $(OS_CFLAGS)
INC_PATHS =  -I.  -I$(NEWMAT_DIR) -I$(QNX_DIR) -I$(TRN_DIR) $(OS_INC_PATH)

# Set to 1 to build lrtrn-server
# Requires LCM and lrauv-lcmtypes
BUILD_LRTRN=0

# Add includes for lrtrn-server if required
ifeq ($(BUILD_LRTRN),1)
LRAUVLCM_DIR=../../lrauv-lcmtypes/include
INC_PATHS += -I$(LRAUVLCM_DIR)
endif

# Linker flags
LD_FLAGS = -g $(GPROF)
LIB_PATHS = -L/usr/local/lib -L$(OUTPUT_DIR) $(OS_LIB_PATH)

########################################
# Target Definitions

LRTRN_SERVER=lrtrn-server
LRTRN_SERVER_SRC=lrtrn_app.cpp LcmTrn.cpp
LRTRN_SERVER_OBJ = $(LRTRN_SERVER_SRC:%.cpp=$(BUILD_DIR)/%.o)
LRTRN_SERVER_LIBS = -llcm -lconfig++ -ltrn  -lnewmat -lqnx -lgeolib -lnetcdf -lm $(OS_LIBS)

########################################
# Build Files (mostly for cleanup)
SOURCES = $(LRTRN_SERVER)

OBJECTS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
DEPENDS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.d)
BINARIES = $(OUTPUT_DIR)/$(LRTRN_SERVER)


CLEANUP = gmon.out
# dSYMs : XCode debug symbol file folders
#DSYMS = $(BINARIES:%=%.dSYM)
#RM_DSYMS = rm -rf $(DSYMS)

########################################
# Rules: build targets

all: $(BINARIES)

# build LRAUV TRN server utility
$(OUTPUT_DIR)/$(LRTRN_SERVER): $(LRTRN_SERVER_OBJ)
	@echo building $@...
	$(CXX) $(CFLAGS) $(INC_PATHS) $(LIB_PATHS) $^ -o $@ $(LD_FLAGS) $(LRTRN_SERVER_LIBS)
	@echo

# generate dependency files
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),purge)
-include $(DEPENDS)
endif
endif

# rule: build object files from source files
$(BUILD_DIR)/%.o :%.cpp
	@echo compiling $<...
	$(CXX) $(CFLAGS) $(INC_PATHS) -c $< -o $@
	@echo

# rule: build dependency files from source files
$(BUILD_DIR)/%.d :%.cpp
	@[ -d $(BUILD_DIR) ] || mkdir -p $(BUILD_DIR)
	@[ -d $(OUTPUT_DIR) ] || mkdir -p $(OUTPUT_DIR)
	@echo generating dependency file for $<
	@set -e; $(CXX) -MM $(CFLAGS) $(INC_PATHS) $< \
	| awk '/o:/ {printf "%s", "$@ $(BUILD_DIR)/"} {print}' > $@; \
	[ -s $@ ] || rm -f $@
	@echo

install:
	@echo "make install...(not implemented)"


########################################
# Rules:

.PHONY: clean
.PHONY: purge

# clean : delete object, dependency, binary files
clean:
	rm -f $(OBJECTS) $(DEPENDS) $(BINARIES)
	$(RM_DSYMS)

# purge : delete delete object, dependency, binary files, build directories
purge:
	rm -f $(BINARIES) $(OBJECTS) $(DEPENDS) $(CLEANUP)
	rm -rf $(OUTPUT_DIR) $(BUILD_DIR) $(DSYMS)

