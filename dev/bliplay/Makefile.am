AM_CFLAGS = @BK_STD_CFLAGS@  -I$(srcdir)/../../src
AM_LDFLAGS = -L$(srcdir)/../../src -lblipkit @SDL_CFLAGS@

pkginclude_HEADERS = \
	BKSDLTrack.h \
	BKParser.h \
	BKBlipReader.h \
	BKBlipWriter.h \
	BKCompiler.h \
	BKInterpreter.h \
	item_list.h

EXTRA_LIBRARIES = libbliparser.a

libbliparser_a_SOURCES = \
	BKSDLTrack.c \
	BKParser.c \
	BKBlipReader.c \
	BKBlipWriter.c \
	BKCompiler.c \
	BKInterpreter.c

EXTRA_PROGRAMS = bliplay

bliplay_SOURCES = \
	bliplay.c

bliplay_CFLAGS = $(AM_CFLAGS) -DPROGRAM_NAME=\"bliplay\"
<<<<<<< HEAD
bliplay_LDADD = libbliparser.a
=======
>>>>>>> c1448de46c06967bdb3f7349795a67c33671db28

EXTRA_DIST = \
	examples/test1.blip \
	examples/test2.blip \
	examples/test3.blip \
	examples/test4.blip \
	examples/test5.blip \
	examples/test6.blip
