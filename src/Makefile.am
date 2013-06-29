AM_CFLAGS = @BK_STD_CFLAGS@

lib_LIBRARIES = libblipkit.a

libblipkit_a_SOURCES = \
	BKBuffer.c \
	BKClock.c \
	BKContext.c \
	BKData.c \
	BKInstrument.c \
	BKSequence.c \
	BKTone.c \
	BKTrack.c \
	BKUnit.c

HEADER_LIST = \
	BlipKit.h \
	BKBase.h \
	BKBuffer.h \
	BKClock.h \
	BKContext.h \
	BKData.h \
	BKData_internal.h \
	BKInstrument.h \
	BKInstrument_internal.h \
	BKSequence.h \
	BKTone.h \
	BKTime.h \
	BKTrack.h \
	BKUnit.h \
	BKUnit_internal.h

pkginclude_HEADERS = $(HEADER_LIST)

otherincludedir = $(includedir)/blipkit

otherinclude_HEADERS = $(HEADER_LIST)
