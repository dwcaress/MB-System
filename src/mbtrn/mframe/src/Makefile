#############################################################################
# build environment

# version and build configuration

# uncomment and optionally define on command line
# to override value defined in source, e.g.
#   MF_VER=1.0.1 make
# MF_VER ?=1.0.0
MF_BUILD ?=`date +%Y/%m/%dT%H:%M:%S%z`
#LIB_VER=-DMFRAME_VER=$(MF_VER)
LIB_BUILD=-DMFRAME_BUILD=$(MF_BUILD)
BUILD_OPTS=

# build mstats support
# [use WITH_MST_MSTATS=1 make...]
ifdef WITH_MST_MSTATS
BUILD_OPTS+= -DWITH_MST_MSTATS
endif

# include medebug optional macros
# [use WITH_MEDEBUG_OPTIONAL=1 make...]
ifdef WITH_MEDEBUG_OPTIONAL
BUILD_OPTS+= -DWITH_MEDEBUG_OPTIONAL
endif

# include medebug debug macros
# [use WITH_MEDEBUG_DEBUG=1 make...]
ifdef WITH_MEDEBUG_DEBUG
BUILD_OPTS+= -DWITH_MEDEBUG_DEBUG
endif

# include medebug required macros
# [use WITHOUT_MEDEBUG_REQUIRED=1 make...]
ifdef WITHOUT_MEDEBUG_REQUIRED
BUILD_OPTS+= -DWITHOUT_MEDEBUG_REQUIRED
endif

# build mmdebug disable
# [use WITHOUT_MMDEBUG=1 make...]
ifdef WITHOUT_MMDEBUG
BUILD_OPTS+= -DWITHOUT_MMDEBUG
endif

# build mmdebug unit test
# [use WITH_MMDEBUG_TEST=1 make...]
ifdef WITH_MMDEBUG_TEST
BUILD_OPTS+= -DWITH_MMDEBUG_TEST
endif

# include mmdebug unit test optional macros
# [use WITH_MMT_OPTIONAL=1 make...]
ifdef WITH_MMT_OPTIONAL
BUILD_OPTS+= -DWITH_MMT_OPTIONAL
endif

# include mmdebug unit test debug macros
# [use WITH_MMT_DEBUG=1 make...]
ifdef WITH_MMT_DEBUG
BUILD_OPTS+= -DWITH_MMT_DEBUG
endif

# include portable debug macros
# [use WITH_PDEBUG=1 make...]
ifdef WITH_PDEBUG
BUILD_OPTS+= -DWITH_PDEBUG
endif

# exclude mmdebug unit test required macros
# [use WITHOUT_MMT_REQUIRED=1 make...]
ifdef WITHOUT_MMT_REQUIRED
BUILD_OPTS+= -DWITHOUT_MMT_REQUIRED
endif

# build msocket unit test
# [use WITH_MSOCKET_TEST=1 make...]
ifdef WITH_MSOCKET_TEST
BUILD_OPTS+= -DWITH_MSOCKET_TEST
endif

# build mserial unit test
# [use WITH_MSERIAL_TEST=1 make...]
ifdef WITH_MSERIAL_TEST
BUILD_OPTS+= -DWITH_MSERIAL_TEST
endif

# build mstats unit test
# [use WITH_MSTATS_TEST=1 make...]
ifdef WITH_MSTATS_TEST
BUILD_OPTS+= -DWITH_MSTATS_TEST -DMST_STATS_EN
endif

# build medebug unit test
# [use WITH_MEDEBUG_TEST=1 make...]
ifdef WITH_MEDEBUG_TEST
BUILD_OPTS+= -DWITH_MEDEBUG_TEST
endif

# build mtime unit test
# [use WITH_MTIME_TEST=1 make...]
ifdef WITH_MTIME_TEST
BUILD_OPTS+= -DWITH_MTIME_TEST
endif

# build mbbuf unit test
# [use WITH_MBBUF_TEST=1 make...]
ifdef WITH_MBBUF_TEST
BUILD_OPTS+= -DWITH_MBBUF_TEST
endif

# build mlog unit test
# [use WITH_MLOG_TEST=1 make...]
ifdef WITH_MLOG_TEST
BUILD_OPTS+= -DWITH_MLOG_TEST
endif

# build mswap unit test
# [use WITH_MSWAP_TEST=1 make...]
ifdef WITH_MSWAP_TEST
BUILD_OPTS+= -DWITH_MSWAP_TEST
endif

# build mutils unit test
# [use WITH_MUTILS_TEST=1 make...]
ifdef WITH_MUTILS_TEST
BUILD_OPTS+= -DWITH_MUTILS_TEST
endif

# build mkvconf unit test
# [use WITH_MKVCONF_TEST=1 make...]
ifdef WITH_MKVCONF_TEST
BUILD_OPTS+= -DWITH_MKVCONF_TEST
endif

# build all unit tests
# [use WITH_TESTS=1 make...]
ifdef WITH_TESTS
BUILD_OPTS+= -DWITH_MST_MSTATS -DWITH_MMDEBUG -DWITH_MEDEBUG_TEST \
	-DWITH_MEDEBUG_DEBUG -DWITH_MEDEBUG_OPTIONAL -DWITH_MSOCKET_TEST \
	-DWITH_MSERIAL_TEST -DWITH_MMDEBUG_TEST -DWITH_MSTATS_TEST \
	-DWITH_MTIME_TEST -DWITH_MBBUF_TEST -DWITH_MLOG_TEST \
	-DWITH_MSWAP_TEST -DWITH_MUTILS_TEST -DWITH_MKVCONF_TEST \
	-DWITH_STOPWATCH_EN -DMST_STATS_EN  -DWITH_PDEBUG
endif

# user symbols
# C standard e.g. -std=c99 -std=gnu99
# may be needed for Cygwin (e.g. for loop declare/init)
# not currently C89 compatible
STD= #-std=c99 -std=gnu99 -std=c89
OPTIONS= $(STD) -D_GNU_SOURCE $(BUILD_OPTS) $(LIB_VER) $(LIB_BUILD)

# dependency paths

# build directories
OUTPUT_DIR=../bin
BUILD_DIR=../build

# compilation
CXX = gcc 
# compile and link with -pg for profiling support
# then do gprof <application> gmon.out for profile output to console
GPROF= #-pg # for gprofile support
CFLAGS = -g -Wall $(GPROF) -Wno-deprecated -Wsign-compare -O2 $(OPTIONS) 
LD_FLAGS = $(GPROF) -g -lm
QNX_PATHS = -I/usr/include:
INC_PATHS =  -I. $(QNX_PATHS)
LIB_PATHS = -L$(OUTPUT_DIR)

#############################################################################
# target definitions

# libmframe : main target
LIBMFRAME=libmframe.a
LIBMFRAME_SRC=mframe.c mcbuf.c mlist.c merror.c  \
mlog.c msocket.c mtime.c mlist.c mfile.c mthread.c mutils.c mserial.c \
mbbuf.c mmem.c mqueue.c mstats.c mmdebug.c mconfig.c mswap.c mkvconf.c #mdebug.c mhash.c mtree.h

LIBMFRAME_OBJ=$(LIBMFRAME_SRC:%.c=$(BUILD_DIR)/%.o)

# mframe-test : test libmframe
MFRAME_TEST=mframe-test
MFRAME_TEST_SRC=mframe-test.c 
MFRAME_TEST_OBJ=$(MFRAME_TEST_SRC:%.c=$(BUILD_DIR)/%.o)
MFRAME_TEST_LIBS = -lmframe -lpthread
MFRAME_TEST_CFLAGS = -DWITH_MSOCKET_TEST

MSTATS_TEST=mstats-test
MSTATS_TEST_SRC=mstats-test.c
MSTATS_TEST_OBJ=$(MSTATS_TEST_SRC:%.c=$(BUILD_DIR)/%.o)
MSTATS_TEST_LIBS = -lmframe -lpthread

MMDEBUG_TEST=mmdebug-test
MMDEBUG_TEST_SRC=mmdebug-test.c
MMDEBUG_TEST_OBJ=$(MMDEBUG_TEST_SRC:%.c=$(BUILD_DIR)/%.o)
MMDEBUG_TEST_LIBS = -lmframe -lpthread

MEDEBUG_TEST=medebug-test
MEDEBUG_TEST_SRC=medebug-test.c
MEDEBUG_TEST_OBJ=$(MEDEBUG_TEST_SRC:%.c=$(BUILD_DIR)/%.o)
MEDEBUG_TEST_LIBS = -lmframe -lpthread

MSOCKET_TEST=msocket-test
MSOCKET_TEST_SRC=msocket-test.c
MSOCKET_TEST_OBJ=$(MSOCKET_TEST_SRC:%.c=$(BUILD_DIR)/%.o)
MSOCKET_TEST_LIBS = -lmframe -lpthread

MTIME_TEST=mtime-test
MTIME_TEST_SRC=mtime-test.c
MTIME_TEST_OBJ=$(MTIME_TEST_SRC:%.c=$(BUILD_DIR)/%.o)
MTIME_TEST_LIBS = -lmframe -lpthread -lm

MKVCONF_TEST=mkvconf-test
MKVCONF_TEST_SRC=mkvconf-test.c
MKVCONF_TEST_OBJ=$(MKVCONF_TEST_SRC:%.c=$(BUILD_DIR)/%.o)
MKVCONF_TEST_LIBS = -lmframe -lpthread

# doxyen: Doxygen documentation

#############################################################################
# build files (mostly for cleanup)
SOURCES = $(MFRAME_TEST_SRC) $(LIBMFRAME_SRC) $(MSTATS_TEST_SRC) $(MMDEBUG_TEST_SRC) $(MEDEBUG_TEST_SRC) $(MSOCKET_TEST_SRC) $(MTIME_TEST_SRC) $(MKVCONF_TEST_SRC)
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o) $(OUTPUT_DIR)/$(LIBMFRAME)
DEPENDS = $(SOURCES:%.c=$(BUILD_DIR)/%.d)
BINARIES = $(OUTPUT_DIR)/$(MFRAME_TEST) $(OUTPUT_DIR)/$(MSTATS_TEST) $(OUTPUT_DIR)/$(MMDEBUG_TEST) $(OUTPUT_DIR)/$(MEDEBUG_TEST) $(OUTPUT_DIR)/$(MSOCKET_TEST) $(OUTPUT_DIR)/$(MTIME_TEST) $(OUTPUT_DIR)/$(MKVCONF_TEST)
DSYMS   = $(BINARIES:%=%.dSYM)
CLEANUP = gmon.out

#############################################################################
# rules: build targets

all: $(OBJECTS) $(OUTPUT_DIR)/$(LIBMFRAME) $(OUTPUT_DIR)/$(MFRAME_TEST) $(OUTPUT_DIR)/$(MSTATS_TEST) $(OUTPUT_DIR)/$(MMDEBUG_TEST) $(OUTPUT_DIR)/$(MEDEBUG_TEST) $(OUTPUT_DIR)/$(MSOCKET_TEST) $(OUTPUT_DIR)/$(MTIME_TEST) \
 	$(OUTPUT_DIR)/$(MKVCONF_TEST)

# build mframe library
$(OUTPUT_DIR)/$(LIBMFRAME):	$(LIBMFRAME_OBJ)
	@echo building $@...
	ar -r $@ $(LIBMFRAME_OBJ)

# build mframe_test utility
$(OUTPUT_DIR)/$(MFRAME_TEST): $(MFRAME_TEST_OBJ) $(OUTPUT_DIR)/$(LIBMFRAME)
	@echo building $@...
	$(CXX) $(INC_PATHS) $(OPTIONS) $(MFRAME_TEST_CFLAGS) $(LIB_PATHS) $^ -o $@ $(MFRAME_TEST_LIBS) $(LD_FLAGS)
	@echo

# build mstats_test utility
$(OUTPUT_DIR)/$(MSTATS_TEST): $(MSTATS_TEST_OBJ) $(OUTPUT_DIR)/$(LIBMFRAME)
	@echo building $@...
	$(CXX) $(INC_PATHS) $(OPTIONS) $(LIB_PATHS) $^ -o $@ $(MSTATS_TEST_LIBS) $(LD_FLAGS)
	@echo

# build mmdebug-test utility
$(OUTPUT_DIR)/$(MMDEBUG_TEST): $(MMDEBUG_TEST_OBJ) $(OUTPUT_DIR)/$(LIBMFRAME)
	@echo building $@...
	$(CXX) $(INC_PATHS) $(OPTIONS) $(LIB_PATHS) $^ -o $@ $(MMDEBUG_TEST_LIBS) $(LD_FLAGS)
	@echo

# build medebug-test utility
$(OUTPUT_DIR)/$(MEDEBUG_TEST): $(MEDEBUG_TEST_OBJ) $(OUTPUT_DIR)/$(LIBMFRAME)
	@echo building $@...
	$(CXX) $(INC_PATHS) $(OPTIONS) $(LIB_PATHS) $^ -o $@ $(MEDEBUG_TEST_LIBS) $(LD_FLAGS)
	@echo

# build msocket-test utility
$(OUTPUT_DIR)/$(MSOCKET_TEST): $(MSOCKET_TEST_OBJ) $(OUTPUT_DIR)/$(LIBMFRAME)
	@echo building $@...
	$(CXX) $(INC_PATHS) $(OPTIONS) $(LIB_PATHS) $^ -o $@ $(MSOCKET_TEST_LIBS) $(LD_FLAGS)
	@echo

# build mtime-test utility
$(OUTPUT_DIR)/$(MTIME_TEST): $(MTIME_TEST_OBJ) $(OUTPUT_DIR)/$(LIBMFRAME)
	@echo building $@...
	$(CXX) $(INC_PATHS) $(OPTIONS) $(LIB_PATHS) $^ -o $@ $(MTIME_TEST_LIBS) $(LD_FLAGS)
	@echo

# build mkvconfg-test utility
$(OUTPUT_DIR)/$(MKVCONF_TEST): $(MKVCONF_TEST_OBJ) $(OUTPUT_DIR)/$(LIBMFRAME)
	@echo building $@...
	$(CXX) $(INC_PATHS) $(OPTIONS) $(LIB_PATHS) $^ -o $@ $(MKVCONF_TEST_LIBS) $(LD_FLAGS)
	@echo

# uncomment to enable/disable dependency generation
#include $(DEPENDS)
-include $(DEPENDS)

# rule: build object files from source files
$(BUILD_DIR)/%.o :%.c 
	@echo compiling $<...
	$(CXX) $(CFLAGS) $(INC_PATHS) -c $< -o $@
	@echo

# rule: build dependency files from source files
$(BUILD_DIR)/%.d :%.c
	@[ -d $(BUILD_DIR) ] || mkdir -p $(BUILD_DIR)
	@[ -d $(OUTPUT_DIR) ] || mkdir -p $(OUTPUT_DIR)
	@echo generating dependency file for $<
	@set -e; $(CXX) -MM $(CFLAGS) $(INC_PATHS) $< \
	| awk '/o:/ {printf "%s", "$@ $(BUILD_DIR)/"} {print}' > $@; \
	[ -s $@ ] || rm -f $@
	@echo

install:
	@echo "Installing...(not implemented)"


###########################################################################
# rules:

.PHONY: clean
.PHONY: purge

# clean : delete object, dependency, binary files
clean:
	rm -f $(OBJECTS) $(DEPENDS) $(BINARIES)
	rm -rf $(DSYMS)

# purge : delete delete object, dependency, binary files, build directories
purge:
	rm -f $(BINARIES) $(OBJECTS) $(DEPENDS) $(DSYMS) $(CLEANUP)
	rm -rf $(OUTPUT_DIR) $(BUILD_DIR)

# include the dependencies
ifneq ($(MAKECMDGOALS),purge)
ifneq ($(MAKECMDGOALS),clean)
-include $(DEPENDS)
endif
endif
