AM_CFLAGS = @AM_CFLAGS@ -I$(srcdir)/../src @SDL_CFLAGS@
LDADD = ../src/libblipkit.a @SDL_LDADD@ -lm

EXTRA_PROGRAMS = \
	tone \
	divider \
	stereo \
	scratch \
	waveform \
	envelope \
	sample \
	arpeggio \
	sample_sustain \
	clocks

tone_SOURCES = tone.c
divider_SOURCES = divider.c
stereo_SOURCES = stereo.c
scratch_SOURCES = scratch.c
waveform_SOURCES = waveform.c
envelope_SOURCES = envelope.c
sample_SOURCES = sample.c
arpeggio_SOURCES = arpeggio.c
sample_sustain_SOURCES = sample_sustain.c
clocks_SOURCES = clocks.c
