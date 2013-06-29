AM_CFLAGS = @BK_STD_CFLAGS@

#bin_PROGRAMS = tone divider stereo scratch

EXTRA_PROGRAMS = \
	tone \
	divider \
	stereo \
	scratch \
	waveform \
	envelope

tone_SOURCES = tone.c
divider_SOURCES = divider.c
stereo = stereo.c
scratch = scratch.c
waveform = waveform.c
envelope = envelope.c

COMPILER_FLAGS = $(AM_CFLAGS) -I$(srcdir)/../src -L$(srcdir)/../src -lblipkit
LINKER_FLAGS = @SDL_CFLAGS@

tone_CFLAGS = $(COMPILER_FLAGS)
divider_CFLAGS = $(COMPILER_FLAGS)
stereo_CFLAGS = $(COMPILER_FLAGS)
scratch_CFLAGS = $(COMPILER_FLAGS)
waveform_CFLAGS = $(COMPILER_FLAGS)
envelope_CFLAGS = $(COMPILER_FLAGS)

tone_LDFLAGS = $(LINKER_FLAGS)
divider_LDFLAGS = $(LINKER_FLAGS)
stereo_LDFLAGS = $(LINKER_FLAGS)
scratch_LDFLAGS = $(LINKER_FLAGS)
waveform_LDFLAGS = $(LINKER_FLAGS)
envelope_LDFLAGS = $(LINKER_FLAGS)
