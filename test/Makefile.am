AM_CFLAGS = @AM_CFLAGS@ \
	-I$(srcdir)/../src

BK_LDADD = ../src/libblipkit.a @SDL_CFLAGS@ -lm

check_PROGRAMS = \
	context \
	track \
	wave

context_SOURCES = context.c
context_LDADD = $(BK_LDADD)

track_SOURCES = track.c
track_LDADD = $(BK_LDADD)

wave_SOURCES = wave.c
wave_LDADD = $(BK_LDADD)

# Enable malloc debugging where available
TESTS_ENVIRONMENT = \
	top_builddir=$(top_builddir); \
	export MallocScribble=1; \
	export MallocPreScribble=1; \
	export MallocGuardEdges=1;

TESTS = \
	context \
	track \
	wave
