########################################
# libtrnav/opt Top Level Makefile
# configures distribution, build targets
# and calls target build files
#
# Use make help to see target options
#
########################################

########################################
# Directories
BUILD_DIR=../build
BIN_DIR=../bin
TRN_DIR=../terrain-nav
QNX_DIR=../qnx-utils
GCTP_DIR=../gctp/source
NEWMAT_DIR=../newmat
UTILS_DIR=../utils
TRNW_DIR=../trnw
DOC_DIR=../doc
DORADO_DIR=./dorado
LRAUV_DIR=./lrauv
ROV_DIR=./rov
REPO_TOP_DIR=..

ifeq ($(shell uname),Darwin)
TAR=gnutar
else
TAR=tar
endif

########################################
# Distribution Version
# By default, distribution archives are named
#
#  DIST_NAME-DIST_VER.tar.gz
#
# The DIST_VER value is set below or via the DIST_DATE environment variable.
# Optionally, a date may be set by by invoking
# make dist with the DIST_DATE environment variable set
#
#  DIST_DATE=01apr18 make dist
#
# which will produce a distribution named
#
#  DIST_NAME-DIST_VER-DIST_DATE.tar.gz

# distro version and date
# (internal, do not set; use DIST_DATE and DIST_VER environment variables)
DISTV=
DISTD=

ifndef DIST_DATE
#DIST_DATE=$(shell date +%Y%m%dT%H%M%S)
DIST_DATE=$(shell date +%Y%m%dT%H%M)
endif
DISTD=$(DIST_DATE)

ifndef DIST_VER
DIST_VER=dev-$(DISTD)
endif
DISTV=$(DIST_VER)

# dist archive options
DIST_OPTS= --exclude=*.o \
--exclude=*.DS_Store --exclude=doc/doxygen.out \
--exclude=*out --exclude=*.d --exclude=*.a --exclude=*/bin/* \
--exclude=*/build/* --exclude=*/.git --exclude=*/dep \
--exclude=*/tmp --exclude=*/dep --exclude=*.gz \
--exclude=*/CVS

# archive current working directory
DIST_WD=$(shell basename $(PWD))

# distribution file name (and distro directory name in archive)
DIST_NAME:=libtrnav

# archive top directory
DIST_TOP_DIR=$(shell dirname $(PWD))
# archive output directory
DIST_OUT_DIR=$(BIN_DIR)

# archive name
DIST_ARCH=$(DIST_NAME)-$(DISTV).tar.gz

# archive directory name (in tar file)
DIST_XDIR=$(DIST_NAME)-$(DISTV)


########################################
# Build Environment

MAKEFILE = Makefile

# Note: defined here, these will override local Makefile;
#       dont' define these unelss they're common to all subdirectories
CFLAGS  =       #-O (or whatever you need)

#       Note: defined here, these will override local Makefile;
#       dont' define these unelss they're common to all subdirectories
LDFLAGS =      # -lm (or whatever you need)

$(info "INFO +++ opt - OPT_USE_PROJ ${OPT_USE_PROJ}")
$(info "INFO +++ opt - PROJ_LIB ${PROJ_LIB}")
$(info "INFO +++ opt - OPT_PROJ_INC ${OPT_PROJ_INC}")
$(info "INFO +++ opt - OPT_PROJ_LIB ${OPT_PROJ_LIB}")

########################################
# Target Definitions

all: apps

libs:
	@echo "Building libs..."
	cd $(REPO_TOP_DIR); make

apps: dorado lrauv rov

dorado: libs
	@echo "Building dorado..."
	cd $(DORADO_DIR); make

lrauv: libs
	@echo "Building lrauv..."
	cd $(LRAUV_DIR); make

rov: libs
	@echo "Building rov..."
	cd $(ROV_DIR); make

help:
	@echo
	@echo "make [clean all dorado lrauv]"
	@echo "apps        build libraries and applications"
	@echo "dorado      build libraries and dorado apps"
	@echo "lrauv       build libraries and lrauv apps"
	@echo "rov         build libraries and rov apps"
	@echo "clean       remove build products (not including doc)"
#	@echo "dist-clean remove build products (including doc)"
	@echo ""
#	@echo "[1] use make trnc MFRAME_INC_DIR=/mframe/header/path MFRAME_LIB_DIR=/mframe/lib/path "
	@echo

dist:
	@echo "Building tar.gz dist..."
	@echo "DIST_ARCH $(DIST_ARCH)"
	@echo "DIST_NAME $(DIST_NAME)"
	@echo "DISTV     $(DISTV)"
	@echo "DISTD     $(DISTD)"
	@echo "DIST_TOP_DIR  $(DIST_TOP_DIR)"
	@echo "DIST_WD   $(DIST_WD)"
	@echo "DIST_XDIR $(DIST_XDIR)"
	@echo "DIST_OPTS $(DIST_OPTS)"
	$(TAR) czvf $(DIST_OUT_DIR)/$(DIST_ARCH) -C $(DIST_TOP_DIR) --transform s/$(DIST_WD)/$(DIST_XDIR)/ $(DIST_OPTS) $(DIST_WD)

dist-dev:
	@echo "Building tar.gz dist-dev..."
	$(eval DIST_XDIR=$(DIST_NAME))
	$(TAR) czvf $(DIST_OUT_DIR)/$(DIST_ARCH) -C $(DIST_TOP_DIR) --transform s/$(DIST_WD)/$(DIST_XDIR)/ $(DIST_OPTS) $(DIST_WD)

install:
	@echo "Installing system...(not implemented)"

.PHONY: build
.PHONY: libs
.PHONY: apps
.PHONY: dorado
.PHONY: lrauv
.PHONY: rov
.PHONY: clean
.PHONY: dist-clean
#.PHONY: dist
.PHONY: help

clean:
	@echo "Clean app..."
	cd $(DORADO_DIR); make clean
	cd $(LRAUV_DIR); make clean
	cd $(ROV_DIR); make clean

dist-clean: clean
	@echo "Clean doc..."
	cd $(DOC_DIR); make clean
