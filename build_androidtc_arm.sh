#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildClient#
#

# Script to setup Android toolchain

#setup NDKROOT as the base path to the android NDK e.g. /usr/local/android-ndk-r9c

export ANDROIDTC="$HOME/androidarm-tc"

if [ ! -d $ANDROIDTC/arm-linux-androideabi ]; then
    $NDKROOT/build/tools/make-standalone-toolchain.sh --platform=android-9 --install-dir=$ANDROIDTC
fi
