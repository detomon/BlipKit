AM_CFLAGS = @BK_STD_CFLAGS@ -I$(srcdir)/../src -L$(srcdir)/../src
#AM_LDFLAGS = -lblipkit @SDL_CFLAGS@
BK_LDADD = -lblipkit @SDL_CFLAGS@

EXTRA_PROGRAMS = \
	tone \
	divider \
	stereo \
	scratch \
	waveform \
	envelope \
	sample

# LDFLAGS should be used for linking libraries not LDADD
# But some newer GCC versions need to have the linkings at the end

tone_SOURCES = tone.c
tone_LDADD = $(BK_LDADD)

divider_SOURCES = divider.c
divider_LDADD = $(BK_LDADD)

stereo_SOURCES = stereo.c
stereo_LDADD = $(BK_LDADD)

scratch_SOURCES = scratch.c
scratch_LDADD = $(BK_LDADD)

waveform_SOURCES = waveform.c
waveform_LDADD = $(BK_LDADD)

envelope_SOURCES = envelope.c
envelope_LDADD = $(BK_LDADD)

sample_SOURCES = sample.c
sample_LDADD = $(BK_LDADD)
