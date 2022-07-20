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
INC_PATHS =  -I.  -I$(NEWMAT_DIR) -I$(QNX_DIR) -I$(TRN_DIR) -I$(UTILS_DIR) $(OS_INC_PATH)

# Linker flags
LD_FLAGS = -g $(GPROF)
LIB_PATHS = -L$(OUTPUT_DIR) $(OS_LIB_PATH) -L/usr/local/lib

########################################
# Target Definitions

LIB_TNAVC=libtnavc.a
LIB_TNAVC_OBJ=$(BUILD_DIR)/TerrainNavClient.o $(BUILD_DIR)/TRNUtils.o

MBARI_MAIN=mbari-main
MBARI_MAIN_SRC=mainMbari.cpp
MBARI_MAIN_OBJ = $(MBARI_MAIN_SRC:%.cpp=$(BUILD_DIR)/%.o)
MBARI_MAIN_LIBS = -ltnavc -ltrn  -lnewmat -lqnx -lnetcdf -lm -lpthread $(OS_LIBS)

REPLAY=trn-replay
REPLAY_SRC=Replay.cpp trn_replay.cpp
REPLAY_OBJ = $(REPLAY_SRC:%.cpp=$(BUILD_DIR)/%.o)
REPLAY_LIBS = -ltrn  -lnewmat -lqnx -lnetcdf -lm -lpthread -lgeolib -ltnavc $(OS_LIBS)

########################################
# Build Files (mostly for cleanup)
SOURCES =  $(MBARI_MAIN_SRC) \
		$(REPLAY_SRC)

OBJECTS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
DEPENDS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.d)
LIBS = $(OUTPUT_DIR)/$(LIB_TNAVC)
BINARIES =  $(OUTPUT_DIR)/$(MBARI_MAIN) \
			$(OUTPUT_DIR)/$(REPLAY) \

CLEANUP = gmon.out
# dSYMs : XCode debug symbol file folders
#DSYMS = $(BINARIES:%=%.dSYM)
#RM_DSYMS = rm -rf $(DSYMS)

########################################
# Rules: build targets

all: $(LIBS) $(BINARIES)

# build TerrainNavClient lib
$(OUTPUT_DIR)/$(LIB_TNAVC):
	@echo building $@...
	ar -rcs $@ $(LIB_TNAVC_OBJ)

# build mbari_main utility
$(OUTPUT_DIR)/$(MBARI_MAIN): $(MBARI_MAIN_OBJ)
	@echo building $@...
	$(CXX) $(CFLAGS) $(INC_PATHS) $(LIB_PATHS) $^ -o $@ $(LD_FLAGS) $(MBARI_MAIN_LIBS)
	@echo

# build replay utility
$(OUTPUT_DIR)/$(REPLAY): $(REPLAY_OBJ)
	@echo building $@...
	$(CXX) $(CFLAGS) $(INC_PATHS) $(LIB_PATHS) $^ -o $@ $(LD_FLAGS) $(REPLAY_LIBS)
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
	rm -f $(OBJECTS) $(DEPENDS) $(LIBS) $(BINARIES)
	$(RM_DSYMS)

# purge : delete delete object, dependency, binary files, build directories
purge:
	rm -f $(BINARIES) $(OBJECTS) $(DEPENDS) $(CLEANUP)
	rm -rf $(OUTPUT_DIR) $(BUILD_DIR) $(DSYMS)

