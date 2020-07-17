################################################
# mframe
# top level makefile
# configures distribution, build targets
# and calls target build files
#
# Use make help to see target options
#
################################################


################################################
# Distribution Version
# By default, distribution archives are named
#
#  mframe-DIST_VER.tar.gz
#
# The DIST_VER value is set below or via the DIST_DATE environment variable.
# Optionally, a date may be set by by invoking
# make dist with the DIST_DATE environment variable set
#
#   DIST_DATE=01apr18 make dist
#
# which will produce a distribution named
#
#  mframe-DIST_VER-DIST_DATE.tar.gz


# distro version and date
# (internal, do not set; use DIST_DATE and DIST_VER environment variables)
DISTV=
DISTD=

ifndef DIST_VER
DIST_VER=1.0.0
DISTV=-$(DIST_VER)
endif

ifdef DIST_DATE
DISTD=-$(DIST_DATE)
#else
#DISTD=-$(shell date +%Y%m%dT%H%M%S)
endif

# dist archive options
DIST_OPTS= --exclude=*.o \
--exclude=*.DS_Store --exclude=doc/doxygen.out \
--exclude=*out --exclude=*.d --exclude=*.a --exclude=*/bin/* \
--exclude=*doxifier/test* --exclude=*xcworkspace* --exclude=*/.git \
--exclude=*/tmp --exclude=*/dep

# archive current working directory
DIST_WD=$(shell basename $(PWD))

# distribution file name (and distro directory name in archive)
DIST_NAME:=mframe$(DISTV)$(DISTD)

# archive output directory
DIST_DIR=$(shell dirname $(PWD))

################################################
# configuration

MAKEFILE =      Makefile

#       Note: defined here, these will override local Makefile;
#       dont' define these unelss they're common to all subdirectories
CFLAGS  =       #-O (or whatever you need)

#       Note: defined here, these will override local Makefile;
#       dont' define these unelss they're common to all subdirectories
LDFLAGS =      # -lm (or whatever you need)

################################################
# target definitions

all:    mframe

mframe:
	@echo "Building mframe..."
	cd src; make

doc:
	@echo "Building doc..."
	cd doc; make

help:
	@echo
	@echo "make [clean dist-clean all mbtrn doc dist]"
	@echo "mframe     mframe and apps"
	@echo "all        mframe and apps"
	@echo "doc        build doxygen doc (output in doc/doxygen.out)"
	@echo "dist       generate distro archive - optionally override DIST_VER, DIST_DATE"
	@echo "           [default $(DIST_DIR)/$(DIST_NAME).tar.gz]"
	@echo "clean      remove build products (not including doc)"
	@echo "dist-clean remove build products (including doc)"
	@echo

dist:
	@echo "Building tar.gz dist..."
	@echo "DIST_VER  $(DIST_VER)"
	@echo "DIST_DATE $(DIST_DATE)"
	@echo "DIST_WD   $(DIST_WD)"
	@echo "DIST_DIR  $(DIST_DIR)"
	@echo "DIST_NAME $(DIST_NAME)"
	@echo "DIST_OPTS $(DIST_OPTS)"
	@echo "archive   $(DIST_NAME).tar.gz"
	tar czvf $(DIST_DIR)/$(DIST_NAME).tar.gz -C $(DIST_DIR) --transform s/$(DIST_WD)/$(DIST_NAME)/ $(DIST_OPTS) $(DIST_WD)

install:
	@echo "Installing system...(not implemented)"

.PHONY: build
.PHONY: doc
.PHONY: clean
.PHONY: dist-clean
.PHONY: dist
.PHONY: help


clean:
	@echo "Clean mframe..."
	cd src; make clean

dist-clean: clean
	@echo "Clean doc..."
	cd doc; make clean
