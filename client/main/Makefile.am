include $(top_srcdir)/Makefile.incl

if ENABLE_CLIENT_RELEASE
  AM_LDFLAGS += -static-libtool-libs
## for an entirely statically linked library, you may want to try
## -all-static instead.  There's a good chance it won't work properly,
## so we'll use the safer "-static-libtool-libs" by default.
else
if DYNAMIC_CLIENT
## if libtool starts to need flags for dynamic linking, add them here
else
  AM_LDFLAGS += -static
endif
endif ## ENABLE_CLIENT_RELEASE

CLIENTLIBS = -lm /home/carlgt1/androidarm-tc/arm-linux-androideabi/lib/libstdc++.a
LIBS += $(CLIENTLIBS)
QCNUSBDIR = $(BASEDIR)/client/qcnusb

if OS_DARWIN
   LIBS += -framework IOKit -framework Foundation -framework ScreenSaver -framework Cocoa -framework CoreServices
endif

bin_PROGRAMS = qcn qcndemo

qcn_SOURCES = \
           $(UTILDIR)/sac.cpp \
           $(UTILDIR)/qcn_thread.cpp \
           $(UTILDIR)/trickleup.cpp \
           $(UTILDIR)/trickledown.cpp \
           $(UTILDIR)/md5.cpp \
           $(UTILDIR)/execproc.cpp \
           $(UTILDIR)/qcn_util.cpp \
           $(UTILDIR)/qcn_signal.cpp \
           $(UTILDIR)/cserialize.cpp \
           $(UTILDIR)/gzstream.cpp \
           $(MAINDIR)/qcn_shmem.cpp \
           $(SENSORDIR)/csensor.cpp \
           $(GRAPHICS) \
           $(SENSORS) \
           $(QCNUSBDIR)/qcn_shmem_usb.cpp \
           $(MAINDIR)/qcn_thread_sensor.cpp \
           $(MAINDIR)/qcn_thread_sensor_util.cpp \
           $(MAINDIR)/qcn_thread_sensor_loop.cpp \
           $(MAINDIR)/qcn_thread_time.cpp \
           $(MAINDIR)/main.cpp

qcn_DEPENDENCIES = 
qcn_CPPFLAGS = $(AM_CPPFLAGS)
qcn_LDFLAGS = $(AM_LDFLAGS) -lz
qcn_LDADD = $(CLIENTLIBS) $(BOINC_EXTRA_LIBS) $(PTHREAD_LIBS) 

qcndemo_SOURCES = \
           $(UTILDIR)/qcn_thread.cpp \
           $(UTILDIR)/qcn_util.cpp \
           $(UTILDIR)/qcn_signal.cpp \
           $(UTILDIR)/cserialize.cpp \
           $(UTILDIR)/gzstream.cpp \
           $(UTILDIR)/md5.cpp \
           $(MAINDIR)/qcn_shmem.cpp \
           $(MAINDIR)/qcndemo.cpp

qcndemo_DEPENDENCIES =
qcndemo_CPPFLAGS = -DQCNDEMO $(qcn_CPPFLAGS)
qcndemo_LDFLAGS = $(qcn_LDFLAGS)
qcndemo_LDADD = $(qcn_LDADD)

.PHONY: moveover

qcndir = $(BASEDIR)/bin

all-local: client-bin qcndemo moveover 

moveover:
	mv @CLIENT_BIN_FILENAME@ $(BINARY)
	$(STRIP) $(BINARY)
	mkdir -p $(top_srcdir)/client/test/projects/qcn.edu_qcn
	mv $(BINARY) $(top_srcdir)/client/bin/
	cp $(top_srcdir)/client/bin/$(BINARY) $(top_srcdir)/client/test/projects/qcn.edu_qcn/
	$(STRIP) qcndemo
	mv qcndemo ../test/

## these source files need to be specified because no rule uses them.

#EXTRA_DIST = *.h \
#    mac \
#    translation \
#    win
#
clean-local:
	rm -f @CLIENT_BIN_FILENAME@ $(BINARY) ../test/qcndemo $(top_srcdir)/client/bin/*qcn_* $(top_srcdir)/client/test/projects/qcn.edu_qcn/*qcn_* 


