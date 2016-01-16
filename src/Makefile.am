AM_CFLAGS = @AM_CFLAGS@

lib_LIBRARIES = libblipkit.a

libblipkit_a_SOURCES = \
	BKBase.c \
	BKBuffer.c \
	BKClock.c \
	BKContext.c \
	BKData.c \
	BKFFT.c \
	BKInstrument.c \
	BKInterpolation.c \
	BKObject.c \
	BKSequence.c \
	BKTone.c \
	BKTrack.c \
	BKUnit.c \
	BKWaveFileReader.c \
	BKWaveFileWriter.c

HEADER_LIST = \
	BlipKit.h \
	BKBase.h \
	BKBuffer.h \
	BKClock.h \
	BKComplex.h \
	BKContext.h \
	BKContext_internal.h \
	BKData.h \
	BKData_internal.h \
	BKFFT.h \
	BKInstrument.h \
	BKInstrument_internal.h \
	BKInterpolation.h \
	BKObject.h \
	BKSequence.h \
	BKTime.h \
	BKTone.h \
	BKTrack.h \
	BKUnit.h \
	BKUnit_internal.h \
	BKWaveFileReader.h \
	BKWaveFileWriter.h \
	BKWaveFile_internal.h

pkginclude_HEADERS = $(HEADER_LIST)

otherincludedir = $(includedir)/blipkit

otherinclude_HEADERS = $(HEADER_LIST)
