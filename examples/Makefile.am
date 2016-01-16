AM_CFLAGS = @AM_CFLAGS@ -I$(srcdir)/../src -L$(srcdir)/../src
BK_LDADD = ../src/libblipkit.a @SDL_CFLAGS@

EXTRA_PROGRAMS = \
	tone \
	divider \
	stereo \
	scratch \
	waveform \
	envelope \
	sample \
	arpeggio \
	sample_sustain

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

arpeggio_SOURCES = arpeggio.c
arpeggio_LDADD = $(BK_LDADD)

sample_sustain_SOURCES = sample_sustain.c
sample_sustain_LDADD = $(BK_LDADD)
