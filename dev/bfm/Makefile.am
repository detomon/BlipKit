AM_CFLAGS = @BK_STD_CFLAGS@ -I$(srcdir)/../../src -L$(srcdir)/../../src

pkginclude_HEADERS = \
	BKBFMReader.h \
	BKBFMWriter.h \
	BKByteBuffer.h \
	BKByteBufferSource.h \
	BKInstruction.h

EXTRA_LIBRARIES = libblipkitbfm.a

libblipkitbfm_a_SOURCES = \
	BKBFMReader.c \
	BKBFMWriter.c \
	BKByteBuffer.c \
	BKByteBufferSource.c \
	BKInstruction.c

EXTRA_PROGRAMS = blipbfmtest

blipbfmtest_SOURCES = \
	blipbfmtest.c

blipbfmtest_LDADD = libblipkitbfm.a -lblipkit @SDL_CFLAGS@
