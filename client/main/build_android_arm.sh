#/bin/sh

#
# See: http://boinc.berkeley.edu/trac/wiki/AndroidBuildApp#
#

# Script to compile various BOINC libraries for Android to be used
# by science applications

COMPILEBOINC=
COMPILENTP=
CONFIGURE=yes
MAKECLEAN=yes

export BOINC="../../../boinc"

export TCBINARIES="$ANDROIDTC/bin"
export TCINCLUDES="$ANDROIDTC/arm-linux-androideabi"
export TCSYSROOT="$ANDROIDTC/sysroot"
export STDCPPTC="$TCINCLUDES/lib/libstdc++.a"

export PATH="$PATH:$TCBINARIES:$TCINCLUDES/bin"
export CC=arm-linux-androideabi-gcc
export CXX=arm-linux-androideabi-g++
export LD=arm-linux-androideabi-ld
export CFLAGS="--sysroot=$TCSYSROOT -DANDROID -DDECLARE_TIMEZONE -Wall -I$TCINCLUDES/include -O3 -fomit-frame-pointer"
export CXXFLAGS="--sysroot=$TCSYSROOT -DANDROID -Wall -I$TCINCLUDES/include -funroll-loops -fexceptions -O3 -fomit-frame-pointer"
export LDFLAGS="-L$TCSYSROOT/usr/lib -L$TCINCLUDES/lib -llog"
export GDB_CFLAGS="--sysroot=$TCSYSROOT -Wall -g -I$TCINCLUDES/include"
export PKG_CONFIG_SYSROOT_DIR=$TCSYSROOT




export QCNINC='-I./ -I../../ -I../util -I../sensor -I../../../boinc/api -I../../../boinc/lib -I../../../boinc/zip'
#export BOINC_LIBS="-L$BOINC/api -lboinc_api -L$BOINC/lib -lboinc -L$BOINC/zip -lboinc_zip"
export BOINC_LIBS="$BOINC/api/libboinc_api.a $BOINC/lib/libboinc.a $BOINC/zip/libboinc_zip.a"
export QCN_CFLAGS="-Wall -D_USE_NTPDATE_EXEC_ -DQCN -D_THREAD_SAFE -D_ZLIB -O2"

rm -f *.o
rm -f qcn_android

$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o main.o main.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o qcn_shmem.o qcn_shmem.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o qcn_thread_time.o qcn_thread_time.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o qcn_thread_sensor.o qcn_thread_sensor.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o qcn_thread_sensor_util.o qcn_thread_sensor_util.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o qcn_thread_sensor_loop.o qcn_thread_sensor_loop.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o qcn_shmem_usb.o ../qcnusb/qcn_shmem_usb.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o csensor.o ../sensor/csensor.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o sac.o ../util/sac.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o qcn_thread.o ../util/qcn_thread.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o trickleup.o ../util/trickleup.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o trickledown.o ../util/trickledown.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o md5.o ../util/md5.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o execproc.o ../util/execproc.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o qcn_util.o ../util/qcn_util.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o qcn_signal.o ../util/qcn_signal.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o cserialize.o ../util/cserialize.cpp
$CXX $QCNINC $CXXFLAGS $QCN_CFLAGS -c -o gzstream.o ../util/gzstream.cpp

$LD $LDFLAGS \
  $STDCPPTC \
  $BOINC_LIBS \
  -lz -lm \
  csensor.o \
  md5.o	\
  qcn_thread_sensor.o \
  trickledown.o \
  cserialize.o \
  qcn_shmem.o \
  qcn_thread_sensor_loop.o \
  trickleup.o \
  execproc.o \
  qcn_shmem_usb.o \
  qcn_thread_sensor_util.o \
  gzstream.o \
  qcn_signal.o \
  qcn_util.o \
  main.o \
  qcn_thread.o \
  sac.o \
  -o qcn_android


