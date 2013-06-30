AM_CFLAGS = @BK_STD_CFLAGS@

pkginclude_HEADERS = BKChip.h

bin_PROGRAMS = blipchip
blipchip_SOURCES = main.c BKChip.c

blipchip_CFLAGS = $(AM_CFLAGS) -I$(srcdir)/../src -L$(srcdir)/../src -lblipkit
