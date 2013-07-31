BlipKit
=======

BlipKit is a C library for simulating old sound chips. It generates waveforms
like square and triangle waves on different tracks with multiple channels. The
library itself is not able to output audio, it only creates the audio data which
then can be used otherwise.

The behaviour of a track is controlled by attributes which includes waveform,
tone, volume, panning, duty cycle and some more. A track can only play one note
at a time. To play chords for example, each note must be on its own track.  Or
arpeggio can be used to get the feeling of a chord.

There are some effects which affects volume and tone. Instruments can be used to
create envelopes and other interesting things. You may also use custom waveforms
and play samples.

<http://blipkit.monoxid.net>

Building the library
--------------------

First execute `configure` in the base directory:

	blipkit$ ./configure

Then execute `make` to build `libblipkit.a` in the `src` directory:

	blipkit$ make

Optionally, you may want to execute `sudo make install` to install the library
and headers on your system:

	blipkit$ sudo make install

Building and running examples
-----------------------------

All examples use SDL (<http://www.libsdl.org>) to output sound, so you have to
install it first. Execute `make examplename` to build an example in the
`examples` directory.

	blipkit/examples$ make tone
	blipkit/examples$ make divider
	blipkit/examples$ make stereo
	blipkit/examples$ make scratch
	blipkit/examples$ make waveform
	blipkit/examples$ make envelope

Finally, run examples like this:

	blipkit/examples$ ./tone

License
-------

This library is distributed under the MIT license. See `LICENSE`.
