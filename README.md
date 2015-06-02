BlipKit
=======

BlipKit is a C library for creating the beautiful sound of old sound chips.

- Generate waveforms: square, triangle, noise, sawtooth, sine and custom waveforms
- Use an unlimited number of individual tracks
- Use stereo output or up to 8 channels
- Define instruments to create envelopes and other interesting effects
- Use effects: portamento, tremolo, vibrato and some more
- Load multi-channel samples and play them at different pitches

Manual: <http://blipkit.audio>

Basic Example
-------------

This code demonstrates the basic steps to generate audio data of a square wave in the note A with enabled tremolo effect:

```C
// Context object
BKContext ctx;

// Initialize context with 2 channels (stereo)
// and a sample rate of 44100 Hz
BKContextInit (& ctx, 2, 44100);

// Track object to generate the waveform
BKTrack track;

// Initialize track with square wave
// By default, the square wave has a duty cycle of 12.5%
BKTrackInit (& track, BK_SQUARE);

// Set mix and note volume
BKSetAttr (& track, BK_MASTER_VOLUME, 0.15 * BK_MAX_VOLUME);
BKSetAttr (& track, BK_VOLUME,        1.0 * BK_MAX_VOLUME);

// Set note A in octave 3
BKSetAttr (& track, BK_NOTE, BK_A_3 * BK_FINT20_UNIT);

// Enable tremolo effect
BKInt tremolo [2] = {20, 0.66 * BK_MAX_VOLUME};
BKTrackSetEffect (& track, BK_EFFECT_TREMOLO, tremolo, sizeof (tremolo));

// Attach track to context
BKTrackAttach (& track, & ctx);

// Define audio data buffer
// As there are 2 channels used, the buffer actually must be
// two times the size than number of frames are requested
BKFrame frames [512 * 2];

// Generate 512 frames e.g. as they would be requested by an audio output function
// Subsequent calls to this function generate the next requested number of frames
BKContextGenerate (& ctx, frames, 512);

// The channels are interlaced into the buffer in the form: LRLR...
// Which means that the first frame of the left channel is at frames[0],
// the first frame of the right channel at frames[1] and so on
```

Building the Library
--------------------

First execute `autogen.sh` in the base directory to generate the build system:

```Shell
blipkit$ sh ./autogen.sh
```

Next execute `configure` in the base directory:

```Shell
blipkit$ ./configure
```

Then execute `make` to build `libblipkit.a` in the `src` directory:

```Shell
blipkit$ make
```

Optionally, you may want to execute `sudo make install` to install the library
and headers on your system:

```Shell
blipkit$ sudo make install
```

Building and Running Examples
-----------------------------

All examples use SDL (<http://www.libsdl.org>) to output sound, so you have to
install it first. Execute `make examplename` to build an example in the
`examples` directory.

```Shell
blipkit/examples$ make tone
blipkit/examples$ make divider
blipkit/examples$ make stereo
blipkit/examples$ make scratch
blipkit/examples$ make waveform
blipkit/examples$ make envelope
```

Finally, run examples like this:

```Shell
blipkit/examples$ ./tone
```

License
-------

This library is distributed under the MIT license. See `LICENSE`.
