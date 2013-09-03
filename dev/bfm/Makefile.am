AM_CFLAGS = @BK_STD_CFLAGS@ -I$(srcdir)/../../src -L$(srcdir)/../../src

pkginclude_HEADERS = \
	BKBFMReader.h \
	BKBFMWriter.h \
	BKByteBuffer.h \
	BKByteBufferSource.h

EXTRA_LIBRARIES = libblipkitbfm.a

libblipkitbfm_a_SOURCES = \
	BKBFMReader.c \
	BKBFMWriter.c \
	BKByteBuffer.c \
	BKByteBufferSource.c

EXTRA_PROGRAMS = blipbfmtest

blipbfmtest_SOURCES = \
	blipbfmtest.c

# LDFLAGS should be used for linking libraries not LDADD
# But some newer GCC versions need to have the linkings at the end

#bliplay_LDFLAGS = -lblipkit @SDL_CFLAGS@
blipbfmtest_LDADD = libblipkitbfm.a -lblipkit @SDL_CFLAGS@
