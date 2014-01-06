/**
 * Copyright (c) 2012-2014 Simon Schoenenberger
 * http://blipkit.monoxid.net/
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "BlipKit.h"
#include <SDL/SDL.h>

typedef struct {
	BKInt numChannels;
	BKInt sampleRate;
} BKSDLUserData;

BKContext     ctx;
BKTrack       organ;
BKData        organWaveform;
BKInstrument  instrument;
BKSDLUserData userData;
BKDivider     divider;
BKInt         i = 0;

static int getchar_nocanon (unsigned tcflags)
{
	int c;
	struct termios oldtc, newtc;

	tcgetattr (STDIN_FILENO, & oldtc);

	newtc = oldtc;
	newtc.c_lflag &= ~(ICANON | ECHO | tcflags);

	tcsetattr (STDIN_FILENO, TCSANOW, & newtc);
	c = getchar ();
	tcsetattr (STDIN_FILENO, TCSANOW, & oldtc);

	return c;
}

static void fill_audio (BKSDLUserData * info, Uint8 * stream, int len)
{
	// calculate needed frames for one channel
	BKUInt numFrames = len / sizeof (BKFrame) / info -> numChannels;

	BKContextGenerate (& ctx, (BKFrame *) stream, numFrames);
}

BKEnum dividerCallback (BKCallbackInfo * info, void * userData)
{
	static BKInt notes [32] = {
		BK_D_2, BK_NOTE_RELEASE, BK_F_2, -3,
		BK_A_0, BK_NOTE_RELEASE,     -3, -3,
		    -3,              -3, BK_C_3, BK_NOTE_RELEASE,
		BK_G_1, BK_NOTE_RELEASE,     -3, -3,
		BK_F_3,              -3, BK_D_3, BK_NOTE_RELEASE,
		BK_A_0, BK_NOTE_RELEASE,     -3, -3,
		    -3,              -3, BK_F_2, BK_NOTE_RELEASE,
		BK_G_1, BK_NOTE_RELEASE,     -3, -3,
	};

	BKInt note = notes [i];

	if (note >= 0)
		note *= BK_FINT20_UNIT;

	// Set track note
	if (note != -3)
		BKTrackSetAttr (& organ, BK_NOTE, note);

	i ++;

	if (i >= 32)
		i = 0;

	return 0;
}

#ifdef main
#undef main
#endif

int main (int argc, char * argv [])
{
	BKInt const numChannels = 2;
	BKInt const sampleRate  = 44100;

	BKContextInit (& ctx, numChannels, sampleRate);

	BKTrackInit (& organ, BK_SQUARE);

	BKTrackSetAttr (& organ, BK_MASTER_VOLUME, 0.2 * BK_MAX_VOLUME);
	BKTrackSetAttr (& organ, BK_VOLUME,        0.5 * BK_MAX_VOLUME);

	// portamento
	BKTrackSetAttr (& organ, BK_EFFECT_PORTAMENTO, 15);

	// tremolo
	BKInt const tremolo [2] = {12, 0.66 * BK_MAX_VOLUME};
	BKTrackSetPtr (& organ, BK_EFFECT_TREMOLO, tremolo);

	BKTrackAttach (& organ, & ctx);

	//// custom waveform
	#define NUM_PHASES 16

	BKFrame const phases [NUM_PHASES] = {
		 0, +1, +3, +6,
		+4, +5, +6, +7,
		 0, +1, +2, +3,
		-1,  0, -5, -6,
	};

	BKDataInitWithFrames (& organWaveform, phases, NUM_PHASES, 1, 1);

	// Normalize frames to maximum amplitude
	BKDataNormalize (& organWaveform);

	// Set data object as waveform
	BKTrackSetPtr (& organ, BK_WAVEFORM, & organWaveform);
	////

	//// instrument with release sequence
	BKInstrumentInit (& instrument);

	#define NUM_SEQUENCE_PHASES 11

	BKInt const volumeSequence [NUM_SEQUENCE_PHASES] = {
		1.0 * BK_MAX_VOLUME,
		0.9 * BK_MAX_VOLUME, 0.8 * BK_MAX_VOLUME, 0.7 * BK_MAX_VOLUME,
		0.6 * BK_MAX_VOLUME, 0.5 * BK_MAX_VOLUME, 0.4 * BK_MAX_VOLUME,
		0.3 * BK_MAX_VOLUME, 0.2 * BK_MAX_VOLUME, 0.1 * BK_MAX_VOLUME,
		0.0 * BK_MAX_VOLUME,
	};

	BKInstrumentSetSequence (& instrument, BK_SEQUENCE_VOLUME, volumeSequence, NUM_SEQUENCE_PHASES, 0, 1);

	BKTrackSetPtr (& organ, BK_INSTRUMENT, & instrument);
	////

	// Callback struct used for initializing divider
	BKCallback callback;

	callback.func     = dividerCallback;
	callback.userInfo = NULL;

	// We want 120 BPM
	// The master clock ticks at 240 Hz
	// This results to an divider value of 80
	// Divide by 4 to get a 4/4 beat
	// Note that a divider only takes integer values
	// Certain BPM rates are not possible as integers may be rounded down
	BKInt dividerValue = (60.0 / 150.0 / 4) * 240;

	// Initialize divider with divider value and callback
	BKDividerInit (& divider, dividerValue, & callback);

	// Attach the divider to the master clock
	// When samples are generated the callback is called at the defined interval
	BKContextAttachDivider (& ctx, & divider, BK_CLOCK_TYPE_BEAT);

	SDL_Init (SDL_INIT_AUDIO);

	SDL_AudioSpec wanted;

	userData.numChannels = numChannels;
	userData.sampleRate  = sampleRate;

	wanted.freq     = sampleRate;
	wanted.format   = AUDIO_S16SYS;
	wanted.channels = numChannels;
	wanted.samples  = 512;
	wanted.callback = (void *) fill_audio;
	wanted.userdata = & userData;

	if (SDL_OpenAudio (& wanted, NULL) < 0) {
		fprintf (stderr, "Couldn't open audio: %s\n", SDL_GetError ());
		return 1;
	}

	SDL_PauseAudio (0);

	printf ("Press [q] to stop\n");

	while (1) {
		int c = getchar_nocanon (0);

		// Use lock when setting attributes outside of divider callbacks
		SDL_LockAudio ();

		//BKInt vibrato [2] = {16, 3 * BK_FINT20_UNIT};
		//BKTrackSetPtr (& sawtooth, BK_EFFECT_VIBRATO, vibrato);

		SDL_UnlockAudio ();

		if (c == 'q')
			break;
	}

	printf ("\n");

	SDL_PauseAudio (1);
	SDL_CloseAudio ();


	BKDividerDispose (& divider);
	BKDataDispose (& organWaveform);
	BKInstrumentDispose (& instrument);
	BKTrackDispose (& organ);
	BKContextDispose (& ctx);

    return 0;
}
