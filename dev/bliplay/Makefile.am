AM_CFLAGS = @BK_STD_CFLAGS@

pkginclude_HEADERS = \
	BKSDLTrack.h \
	BKParser.h \
	BKBlipReader.h \
	BKBlipWriter.h \
	BKCompiler.h \
	BKInterpreter.h \
	item_list.h

EXTRA_PROGRAMS = bliplay

bliplay_SOURCES = \
	BKSDLTrack.c \
	BKParser.c \
	BKBlipReader.c \
	BKBlipWriter.c \
	BKCompiler.c \
	BKInterpreter.c \
	bliplay.c

COMPILER_FLAGS = $(AM_CFLAGS) -I$(srcdir)/../../src -L$(srcdir)/../../src -lblipkit
LINKER_FLAGS = @SDL_CFLAGS@

bliplay_CFLAGS = $(COMPILER_FLAGS)
bliplay_LDFLAGS = $(LINKER_FLAGS)
