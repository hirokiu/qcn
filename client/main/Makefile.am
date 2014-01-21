include $(top_srcdir)/Makefile.incl

AUTOMAKE_OPTIONS = std-options

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

QCNUSBDIR = $(BASEDIR)/client/qcnusb

##   $(BOINC_DIR)/zip/libboinc_zip.la
##BOINC_LIBS = $(BOINC_DIR)/api/libboinc_api.a $(BOINC_DIR)/lib/libboinc.a $(BOINC_DIR)/zip/libboinc_zip.a
##BOINC_LIBS = -L$(BOINC_DIR)/api -lboinc_api -L$(BOINC_DIR)/lib -lboinc -L$(BOINC_DIR)/zip -lboinc_zip
##BOINC_LIBS = $(BOINC_DIR)/api/libboinc_api.la $(BOINC_DIR)/lib/libboinc.la $(BOINC_DIR)/zip/libboinc_zip.la
BOINC_LIBS = -L$(BOINC_DIR)/api -lboinc_api -L$(BOINC_DIR)/lib -lboinc -L$(BOINC_DIR)/zip -lboinc_zip


BOINC_EXTRA_LIBS =  -L/home/carlgt1/androidarm-tc/arm-linux-androideabi/lib -lcurl -lssl -lcrypto -lz -L/home/carlgt1/androidarm-tc/arm-linux-androideabi -L/home/carlgt1/androidarm-tc/arm-linux-androideabi/lib -lssl -lcrypto   -ldl -lz

CLIENTLIBS = -landroid -lz -lm $(STDCPPTC)
SENSORS = $(SENSORDIR)/csensor_android_built_in.cpp

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
qcn_LDFLAGS = $(AM_LDFLAGS)
qcn_LDADD = $(CLIENTLIBS) $(PTHREAD_LIBS) $(BOINC_LIBS)

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
qcndemo_LDFLAGS = $(qcn_LDFLAGS) -L$(BOINC_DIR)/api -lboinc_api
#qcndemo_LDADD = -L$(BOINC_DIR)/api -lboinc_api -L$(BOINC_DIR)/lib -lboinc $(CLIENTLIBS) $(PTHREAD_LIBS)
qcndemo_LDADD = -L$(BOINC_DIR)/lib -lboinc -L$(BOINC_DIR)/zip -lboinc_zip $(CLIENTLIBS) $(PTHREAD_LIBS)

qcndir = $(BASEDIR)/bin

all-local: qcn qcndemo 

clean-local:
	rm -f @CLIENT_BIN_FILENAME@ ../test/qcndemo $(top_srcdir)/client/bin/*qcn_* $(top_srcdir)/client/test/projects/qcn.edu_qcn/*qcn_* 


