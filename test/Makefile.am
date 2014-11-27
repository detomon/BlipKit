AM_CFLAGS = @BK_STD_CFLAGS@ -I$(srcdir)/../src -L$(srcdir)/../src
BK_LDADD = -lblipkit @SDL_CFLAGS@

check_PROGRAMS = \
	test_context \
	test_track \
	test_fft

test_context_SOURCES = test_context.c
test_context_LDADD = $(BK_LDADD)

test_track_SOURCES = test_track.c
test_track_LDADD = $(BK_LDADD)

test_fft_SOURCES = test_fft.c
test_fft_LDADD = $(BK_LDADD)

TESTS_ENVIRONMENT = top_builddir=$(top_builddir)

TESTS = \
	test_context \
	test_track \
	test_fft
