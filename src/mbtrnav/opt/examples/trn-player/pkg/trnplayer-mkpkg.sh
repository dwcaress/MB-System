#!/bin/bash

# Default canfiguration
SESSION=$(date +%Y%m%d_%H%M%S)
MFRAME_BRANCH=master
LIBTRNAV_BRANCH=master
LIBTRNAV_DATA_BRANCH=main
PKG_NAME=pkg
STAGE_NAME=trnplayer
VERBOSE=N
KEEP_WORK=N

# set paths
BUILD_PARENT=$PWD

WORK_DIR=$BUILD_PARENT/mktrnp-$$
MFRAME_BUILD=$WORK_DIR/mframe/build
MFRAME_INSTALL=$MFRAME_BUILD/$PKG_NAME
LIBTRNAV_BUILD=$WORK_DIR/libtrnav/build
LIBTRNAV_INSTALL=$LIBTRNAV_BUILD/$PKG_NAME
LIBTRNAV_DATA_BUILD=$WORK_DIR/libtrnav-data/build
LIBTRNAV_DATA_INSTALL=$LIBTRNAV_DATA_BUILD/$PKG_NAME

STAGE_DIR=$WORK_DIR/$STAGE_NAME

# function: verbose output
vout() {
    if [ $VERBOSE == "Y" ]; then
        echo $1
    fi
}

print_help() {
    echo ""
    echo " trnplayer-mkpkg.sh : build trn-player binary distribution"
    echo ""
    echo " Use: trnplayer-mkpkg.sh [options...]"
    echo " Options"
    echo ""
    echo "   -h : print help message"
    echo "   -k : keep working files"
    echo "   -v : enable verbose output"
    echo ""
}

# function: command line parser
parse_opts() {
    while getopts "hkv" opt ; do
      case $opt in
        h) print_help
            exit 1
        ;;
        v) VERBOSE="Y"
          ;;
        k) KEEP_WORK="Y"
          ;;
        :) # Catch missing arguments for options that expect one
          echo
          echo "Error: Option -${OPTARG} requires an argument." >&2
          echo
          print_help
          exit 1
          ;;
        ?) # Catch invalid options
          echo
          echo "Error: Invalid option -${OPTARG}." >&2
          echo
          print_help
          exit 1
          ;;
        esac
    done
}

# get command line options
parse_opts $*

# show config (verbose)
vout "VERBOSE $VERBOSE"
vout "KEEP_WORK $KEEP_WORK"

# make working directory
vout "mkdir $WORK_DIR"

mkdir -p $WORK_DIR
cd $WORK_DIR

# clone, build install libtrnav
vout "cloning mframe..."

git clone git@bitbucket.org:mbari/mframe.git

vout "building mframe branch ${MFRAME_BRANCH}..."

cd mframe
git checkout ${MFRAME_BRANCH}
mkdir build

cd build
cmake -DCMAKE_INSTALL_PREFIX=$MFRAME_INSTALL ..
cmake --build .
cmake --install .

# clone, build install libtrnav
vout "cloning libtrnav..."

git clone git@bitbucket.org:mbari/libtrnav.git

vout "building libtrnav branch ${LIBTRNAV_BRANCH}..."

cd libtrnav
git checkout ${LIBTRNAV_BRANCH}
mkdir build

cd build
cmake -DCMAKE_INSTALL_PREFIX=$LIBTRNAV_INSTALL -DCMAKE_PREFIX_PATH=$MFRAME_INSTALL ..
cmake --build .
cmake --install .

vout "making staging directory $STAGE_DIR..."

mkdir -p $STAGE_DIR/bin
mkdir -p $STAGE_DIR/share

#  clone libtrnav-data into staging directory
vout "cloning libtrnav-data branch $LIBTRNAV_DATA_BRANCH..."

cd $STAGE_DIR
git clone git@bitbucket.org:mbari/libtrnav-data.git

cd libtrnav-data
git checkout $LIBTRNAV_DATA_BRANCH

# copy files to staging directory
vout "copying files to staging directory..."

cd $STAGE_DIR
cp $LIBTRNAV_INSTALL/bin/* $STAGE_DIR/bin/.
cp $LIBTRNAV_INSTALL/share/README* $STAGE_DIR/share/.
cp $LIBTRNAV_INSTALL/share/README* $STAGE_DIR/.
cp ./libtrnav-data/trn-player/trnplayer.cfg $STAGE_DIR/.

for APP in $(ls $STAGE_DIR/bin/*)
do
# change netcdf library load path to use RPATHs
# to make it relocatable
install_name_tool -change /opt/local/lib/libnetcdf.22.dylib @rpath/libnetcdf.dylib $APP

# codesign executable so it won't be rejected by SIP/Gatekeeper
codesign --force --deep --sign - $APP
done

# tar package
vout "building tar.gz archive $BUILD_PARENT/trnplayer-$SESSION.tar.gz..."

cd $WORK_DIR
tar czvf $BUILD_PARENT/trnplayer-$SESSION.tar.gz --exclude "libtrnav-data/.git" $(basename $STAGE_DIR)

# return to top directory, cleanup
cd $BUILD_PARENT

if [ $KEEP_WORK == "N" ]; then
    vout "removing work directory $WORK_DIR"
    rm -rf $WORK_DIR
fi

vout "package created: $(ls -la $BUILD_PARENT/trnplayer-$SESSION.tar.gz)"
vout "done"
