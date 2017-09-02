AM_CFLAGS = @AM_CFLAGS@ -I$(srcdir)/../src -L$(srcdir)/../src
BK_LDADD = ../src/libblipkit.a @SDL_CFLAGS@ -lm

check_PROGRAMS = \
	test_context \
	test_track \
	test_fft \
	test_wave

test_context_SOURCES = test_context.c
test_context_LDADD = $(BK_LDADD)

test_track_SOURCES = test_track.c
test_track_LDADD = $(BK_LDADD)

test_fft_SOURCES = test_fft.c
test_fft_LDADD = $(BK_LDADD)

test_wave_SOURCES = test_wave.c
test_wave_LDADD = $(BK_LDADD)

TESTS_ENVIRONMENT = \
	top_builddir=$(top_builddir); \
	# Enable malloc debugging where available
	export MallocScribble=1; \
	export MallocPreScribble=1; \
	export MallocGuardEdges=1;

TESTS = \
	test_context \
	test_track \
	test_fft \
	test_wave
