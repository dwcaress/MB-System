########################################
# opt/lrauv Build Environment
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

# Build directories
OUTPUT_DIR=${LIBTRNAV_TOP}/bin
BUILD_DIR=${LIBTRNAV_TOP}/build
QNX_DIR=${LIBTRNAV_TOP}/qnx-utils
NEWMAT_DIR=${LIBTRNAV_TOP}/newmat
TRN_DIR=${LIBTRNAV_TOP}/terrain-nav
UTILS_DIR=${LIBTRNAV_TOP}/utils

# Platform-specific options
# extend OPT_OS_LIBS (set in Makefile.common)
ifeq ($(shell uname),Linux)
OPT_OS_LIBS += -larmadillo
endif

# C standard e.g. -std=c99 -std=gnu99
# may be needed for Cygwin (e.g. for loop declare/init)
STD= -std=c++11

# Build options
GIT_VERS = -D__GIT_VERSION__="\"`git describe --tags`\""
BUILD_OPTIONS = $(STD) -D_GNU_SOURCE $(GIT_VERS)  $(OPT_TRN_VER) $(OPT_TRN_BUILD)


# Compilation Options
CC = gcc
CXX = g++
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
INC_PATHS =  -I.  -I$(NEWMAT_DIR) -I$(QNX_DIR) -I$(TRN_DIR) $(OPT_OS_INC)

# Linker flags
LD_FLAGS = -g $(GPROF) $(OPT_OS_LDFLAGS)
LIB_PATHS = -L$(OUTPUT_DIR) -L/usr/local/lib $(OPT_OS_LIB)

# C++ lint tool
CPPLINT=cppcheck --enable=all

# Set to 1 to build lrtrn-server
# Requires LCM and lrauv-lcmtypes
BUILD_LRTRN=1

# Add includes for lrtrn-server if required
ifeq ($(BUILD_LRTRN),1)
LRAUVLCM_DIR=${LIBTRNAV_TOP}/../lrauv-lcmtypes/include
INC_PATHS += -I$(LRAUVLCM_DIR)
endif

ifdef buildDebug
$(info "INFO +++ lrauv - OS_PREFIX          ${OS_PREFIX}")
$(info "INFO +++ lrauv - OPT_OS_CFLAGS      ${OPT_OS_CFLAGS}")
$(info "INFO +++ lrauv - OPT_OS_LDFLAGS     ${OPT_OS_LDFLAGS}")
$(info "INFO +++ lrauv - OPT_OS_INC         ${OPT_OS_INC}")
$(info "INFO +++ lrauv - OPT_OS_LIB         ${OPT_OS_LIB}")
$(info "INFO +++ lrauv - OPT_OS_LIBS        ${OPT_OS_LIBS}")
#	$(info "INFO +++       - CXXFLAGS           ${CXXFLAGS}")
#	$(info "INFO +++       - CPPFLAGS           ${CPPFLAGS}")
$(info "INFO +++       - CFLAGS             ${CFLAGS}")
$(info "INFO +++       - INC_PATHS          ${INC_PATHS}")
$(info "INFO +++       - LD_FLAGS           ${LD_FLAGS}")
$(info "INFO +++       - LIB_PATHS          ${LIB_PATHS}")
endif

########################################
# Target Definitions
LRTRN_APP=lrtrn-app
LRTRN_APP_SRC=LcmTrn.cpp
LRTRN_APP_OBJ = $(LRTRN_APP_SRC:%.cpp=$(BUILD_DIR)/%.o)
LRTRN_APP_PP = $(LRTRN_APP_SRC:%.cpp=./%.cpp-pp) $(LRTRN_APP_SRC:%.h=./%.h-pp)
LRTRN_APP_LIBS = -llcm -lconfig++ -lzf_log -ltrn  -lnewmat -lqnx -lgeolib -lnetcdf -lm $(OPT_OS_LIBS)

LRCONAV_APP=lrconav-app
LRCONAV_APP_SRC=lrconav_app.cpp EgoRobot.cpp MRFilter.cpp MRFilterLog.cpp EgoRobotLog.cpp
LRCONAV_APP_OBJ = $(LRCONAV_APP_SRC:%.cpp=$(BUILD_DIR)/%.o)
LRCONAV_APP_PP = $(LRCONAV_APP_SRC:%.cpp=./%.cpp-pp) $(LRCONAV_APP_SRC:%.h=./%.h-pp)
LRCONAV_APP_LIBS = -llcm -lzf_log -ltrn  -lnewmat -lqnx -lgeolib -lnetcdf -lm $(OPT_OS_LIBS)

#CONAV_MSG_PP = $(CONAV_MSG_SRC:%.cpp=./%.cpp-pp) $(CONAV_MSG_SRC:%.h=./%.h-pp)

CONAV_MSG_APP=mr_msg-app
CONAV_MSG_SRC=mr_msg_app.cpp
CONAV_MSG_OBJ = $(CONAV_MSG_SRC:%.cpp=$(BUILD_DIR)/%.o)
CONAV_MSG_LIBS = -llcm -lzf_log -ltrn  -lnewmat -lqnx -lgeolib -lnetcdf -lm $(OPT_OS_LIBS)

DECODER_TEST_APP=trn-decoder-test
DECODER_TEST_SRC=trn-decoder-test.cpp
DECODER_TEST_OBJ = $(DECODER_TEST_SRC:%.cpp=$(BUILD_DIR)/%.o)
DECODER_TEST_LIBS = -llcm -lzf_log -ltrn  -lnewmat -lqnx -lgeolib -lnetcdf -lm $(OPT_OS_LIBS)

########################################
# Build Files (mostly for cleanup)
#SOURCES = $(LRTRN_APP) $(LRCONAV_APP) $(CONAV_MSG_APP)
SOURCES = $(LRTRN_APP_SRC) $(LRCONAV_APP_SRC) $(CONAV_MSG_SRC) $(DECODER_TEST_SRC)

OBJECTS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.o)
DEPENDS = $(SOURCES:%.cpp=$(BUILD_DIR)/%.d)
BINARIES = $(OUTPUT_DIR)/$(LRTRN_APP) $(OUTPUT_DIR)/$(LRCONAV_APP) $(OUTPUT_DIR)/$(CONAV_MSG_APP) \
	$(OUTPUT_DIR)/$(DECODER_TEST_APP)


CLEANUP = gmon.out
# dSYMs : XCode debug symbol file folders
#DSYMS = $(BINARIES:%=%.dSYM)
#RM_DSYMS = rm -rf $(DSYMS)

########################################
# Rules: build targets

all: $(BINARIES)

# build LRAUV TRN utility
$(OUTPUT_DIR)/$(LRTRN_APP): lrtrn_app.cpp $(LRTRN_APP_OBJ)
	@echo building $@...
	$(CXX) $(CFLAGS) $(INC_PATHS) $(LIB_PATHS) $^ -o $@ $(LD_FLAGS) $(LRTRN_APP_LIBS)
	@echo

# build LR Terrain-Relative CoNav utility
$(OUTPUT_DIR)/$(LRCONAV_APP): $(LRCONAV_APP_OBJ)
	@echo building $@...
	$(CXX) $(CFLAGS) $(INC_PATHS) $(LIB_PATHS) $^ -o $@ $(LD_FLAGS) $(LRCONAV_APP_LIBS)
	@echo

# build LR Terrain-Relative CoNav message test program
$(OUTPUT_DIR)/$(CONAV_MSG_APP): $(CONAV_MSG_OBJ)
	@echo building $@...
	$(CXX) $(CFLAGS) $(INC_PATHS) $(LIB_PATHS) $^ -o $@ $(LD_FLAGS) $(CONAV_MSG_LIBS)
	@echo

# build decodertest program
$(OUTPUT_DIR)/$(DECODER_TEST_APP): $(DECODER_TEST_OBJ)
	@echo building $@...
	$(CXX) $(CFLAGS) $(INC_PATHS) $(LIB_PATHS) $^ -o $@ $(LD_FLAGS) $(DECODER_TEST_LIBS)
	@echo

lint: $(LRTRN_APP_SRC) $(LRCONAV_APP_SRC) $(CONAV_MSG_SRC)
	@echo running cppcheck on source...
	$(CPPLINT) $^
	@echo

format: $(CONAV_MSG_PP) $(LRTRN_APP_PP) $(LRCONAV_APP_PP)
	@echo Formatting source to clang standard...

# generate dependency files
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),purge)
-include $(DEPENDS)
endif
endif


# rule: build object files from source files
$(BUILD_DIR)/%.o :%.cpp
	@echo compiling $<...
	$(CPPLINT) $<
	$(CXX) $(CFLAGS) $(INC_PATHS) -c $< -o $@
	@echo

# rule: build object files from source files
$(BUILD_DIR)/%.o :%.c
	@echo compiling $<...
	$(CPPLINT) $<
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

# rule: build pretty-print files from source files
CLANG_FORMAT=clang-format -style="{BasedOnStyle: llvm, IndentWidth: 4, BreakBeforeBraces: Linux, AlignConsecutiveAssignments: true}"

./%.cpp-pp :%.cpp
	@echo formatting $<...
	$(CLANG_FORMAT) $< > $@
	@echo

./%.h-pp :%.h
	@echo formatting $<...
	$(CLANG_FORMAT) $< > $@
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

