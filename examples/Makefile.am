AM_CFLAGS = @BK_STD_CFLAGS@ -I$(srcdir)/../src
AM_LDFLAGS = -L$(srcdir)/../src -lblipkit @SDL_CFLAGS@

#bin_PROGRAMS = tone divider stereo scratch waveform envelope

EXTRA_PROGRAMS = \
	tone \
	divider \
	stereo \
	scratch \
	waveform \
	envelope

tone_SOURCES = tone.c
divider_SOURCES = divider.c
stereo_SOURCES = stereo.c
scratch_SOURCES = scratch.c
waveform_SOURCES = waveform.c
envelope_SOURCES = envelope.c
