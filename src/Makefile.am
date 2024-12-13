lib_LIBRARIES = libblipkit.a

if ENABLE_WAV
extra_src = \
	BKWaveFileReader.c \
	BKWaveFileWriter.c
extra_hdr = \
	BKWaveFile_internal.h \
	BKWaveFileReader.h \
	BKWaveFileWriter.h
endif

libblipkit_a_SOURCES = \
	BKBase.c \
	BKBuffer.c \
	BKClock.c \
	BKContext.c \
	BKData.c \
	BKInstrument.c \
	BKInterpolation.c \
	BKObject.c \
	BKSequence.c \
	BKTone.c \
	BKTrack.c \
	BKUnit.c \
	$(extra_src)

HEADER_LIST = \
	BKBase.h \
	BKBuffer.h \
	BKClock.h \
	BKContext.h \
	BKContext_internal.h \
	BKData.h \
	BKData_internal.h \
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
	BlipKit.h \
	$(extra_hdr)

pkginclude_HEADERS = $(HEADER_LIST)

otherincludedir = $(includedir)/blipkit

otherinclude_HEADERS = $(HEADER_LIST)
