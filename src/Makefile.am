AM_CFLAGS = @AM_CFLAGS@ -lm

lib_LIBRARIES = libblipkit.a

libblipkit_a_SOURCES = \
	BKArray.c \
	BKBase.c \
	BKBlockPool.c \
	BKBuffer.c \
	BKByteBuffer.c \
	BKClock.c \
	BKContext.c \
	BKData.c \
	BKFFT.c \
	BKHashTable.c \
	BKInstrument.c \
	BKInterpolation.c \
	BKObject.c \
	BKSequence.c \
	BKString.c \
	BKTone.c \
	BKTrack.c \
	BKUnit.c \
	BKWaveFileReader.c \
	BKWaveFileWriter.c

HEADER_LIST = \
	BKArray.h \
	BKBase.h \
	BKBlockPool.h \
	BKBuffer.h \
	BKByteBuffer.h \
	BKClock.h \
	BKComplex.h \
	BKContext.h \
	BKContext_internal.h \
	BKData.h \
	BKData_internal.h \
	BKFFT.h \
	BKHashTable.h \
	BKInstrument.h \
	BKInstrument_internal.h \
	BKInterpolation.h \
	BKObject.h \
	BKSequence.h \
	BKString.h \
	BKTime.h \
	BKTone.h \
	BKTrack.h \
	BKUnit.h \
	BKUnit_internal.h \
	BKWaveFile_internal.h \
	BKWaveFileReader.h \
	BKWaveFileWriter.h \
	BlipKit.h

pkginclude_HEADERS = $(HEADER_LIST)

otherincludedir = $(includedir)/blipkit

otherinclude_HEADERS = $(HEADER_LIST)
