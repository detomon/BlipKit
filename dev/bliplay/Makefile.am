AM_CFLAGS = @BK_STD_CFLAGS@ -I$(srcdir)/../../src -L$(srcdir)/../../src

pkginclude_HEADERS = \
	BKSDLTrack.h \
	BKBlipReader.h \
	BKBlipWriter.h \
	BKCompiler.h \
	BKInterpreter.h \
	item_list.h

EXTRA_LIBRARIES = libbliparser.a

libbliparser_a_SOURCES = \
	BKSDLTrack.c \
	BKBlipReader.c \
	BKBlipWriter.c \
	BKCompiler.c \
	BKInterpreter.c

EXTRA_PROGRAMS = bliplay

bliplay_SOURCES = \
	bliplay.c

# LDFLAGS should be used for linking libraries not LDADD
# But some newer GCC versions need to have the linkings at the end

bliplay_CFLAGS = $(AM_CFLAGS) -DPROGRAM_NAME=\"bliplay\"
#bliplay_LDFLAGS = -lblipkit @SDL_CFLAGS@
bliplay_LDADD = libbliparser.a -lblipkit @SDL_CFLAGS@

EXTRA_DIST = \
	examples/template.blip \
	examples/test1.blip \
	examples/test2.blip \
	examples/test3.blip \
	examples/test4.blip \
	examples/test5.blip \
	examples/test6.blip \
	examples/test7.blip \
	examples/test8.blip
