########################################
# /opt/dorado Makefile
# Build Environment
# version and build configuration

# uncomment and optionally define on command line
# to override value defined in source, e.g.
#   trn_ver=1.0.1 make
trn_ver ?=0.0.1
trn_build=`date +%Y/%m/%dT%H:%M:%S%z`
OPT_TRN_VER=-DLIBTRN_VERSION=$(trn_ver)
OPT_TRN_BUILD=-DLIBTRN_BUILD=$(trn_build)

LIBTRNAV_TOP = ../..
ifndef COMMON_DEFS
include ${LIBTRNAV_TOP}/Makefile.common
endif

# C standard e.g. -std=c99 -std=gnu99
# may be needed for Cygwin (e.g. for loop declare/init)
#STD= -std=c99

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
CFLAGS = -g -O2 $(SANI_FLAG) $(WARN_FLAGS) $(GPROF) $(BUILD_OPTIONS) $(OPT_OS_CFLAGS)
INC_PATHS =  -I.  -I$(NEWMAT_DIR) -I$(QNX_DIR) -I$(TRN_DIR) -I$(UTILS_DIR)  $(OPT_OS_INC)

# Linker flags
LD_FLAGS = -g $(GPROF) $(OPT_OS_LDFLAGS)
LIB_PATHS = -L$(OUTPUT_DIR) $(OPT_OS_LIB)

ifdef buildDebug
$(info "INFO +++ dorado - OS_PREFIX          ${OS_PREFIX}")
$(info "INFO +++ dorado - OPT_OS_CFLAGS      ${OPT_OS_CFLAGS}")
$(info "INFO +++ dorado - OPT_OS_LDFLAGS     ${OPT_OS_LDFLAGS}")
$(info "INFO +++ dorado - OPT_OS_INC         ${OPT_OS_INC}")
$(info "INFO +++ dorado - OPT_OS_LIB         ${OPT_OS_LIB}")
$(info "INFO +++ dorado - OPT_OS_LIBS        ${OPT_OS_LIBS}")
$(info "INFO +++        - CFLAGS             ${CFLAGS}")
$(info "INFO +++        - INC_PATHS          ${INC_PATHS}")
$(info "INFO +++        - LD_FLAGS           ${LD_FLAGS}")
$(info "INFO +++        - LIB_PATHS          ${LIB_PATHS}")
endif

########################################
# Target Definitions

LIB_TNAVC=libtnavc.a
LIB_TNAVC_OBJ=$(BUILD_DIR)/TerrainNavClient.o $(BUILD_DIR)/TRNUtils.o

MBARI_MAIN=mbari-main
MBARI_MAIN_SRC=mainMbari.cpp
MBARI_MAIN_OBJ = $(MBARI_MAIN_SRC:%.cpp=$(BUILD_DIR)/%.o)
MBARI_MAIN_LIBS = -ltnavc -ltrn  -lnewmat -lqnx -lnetcdf -lm -lpthread $(OPT_OS_LIBS)

REPLAY=trn-replay
REPLAY_SRC=Replay.cpp trn_replay.cpp
REPLAY_OBJ = $(REPLAY_SRC:%.cpp=$(BUILD_DIR)/%.o)
REPLAY_LIBS = -ltrn  -lnewmat -lqnx -lnetcdf -lm -lpthread -lgeolib -ltrncli $(OPT_OS_LIBS)

########################################
# Build Files (mostly for cleanup)
SOURCES =  $(MBARI_MAIN_SRC) \
		$(REPLAY_SRC)

OBJECTS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
DEPENDS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.d)
LIBS = $(OUTPUT_DIR)/$(LIB_TNAVC)
BINARIES =  $(OUTPUT_DIR)/$(MBARI_MAIN) \
			$(OUTPUT_DIR)/$(REPLAY)

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

