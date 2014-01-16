include $(top_srcdir)/Makefile.incl

client-bin: @CLIENT_BIN_FILENAME@

BINARY = @CLIENT_BIN_FILENAME@_$(QCN_VERSION_STRING)_$(build)

QCNUSBDIR = $(BASEDIR)/client/qcnusb

bin_PROGRAMS = qcn qcndemo

           #$(UTILDIR)/qcn_ntpdate.c 
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
qcn_CPPFLAGS = -D_USE_NTPDATE_EXEC_ -I$(QCNUSBDIR) $(AM_CPPFLAGS) $(PTHREAD_CFLAGS)
qcn_LDFLAGS = $(BOINC_LIB) $(PTHREAD_LDFLAGS) $(STACKSIZE)
qcn_LDADD = -lstdc++ -lz

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


