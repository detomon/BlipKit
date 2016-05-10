/*
 * Copyright (c) 2012-2015 Simon Schoenenberger
 * http://blipkit.audio
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

#include "common.h"

typedef struct {
	BKInt numChannels;
	BKInt sampleRate;
} BKSDLUserData;

BKContext     ctx;
BKTrack       track;
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

static BKEnum dividerCallback (BKCallbackInfo * info, void * userData)
{
	static BKInt notes [16] = {
		BK_D_2, -3, -3, -3,
		-3, BK_D_3, -3, -3,
		-1, -3, BK_C_1, -3,
		-3, -3, -1, -3,
	};

	BKInt note = notes [i];

	if (note >= 0)
		note *= BK_FINT20_UNIT;

	// Set track note
	if (note != -3)
		BKSetAttr (& track, BK_NOTE, note);

	i ++;

	if (i >= 16)
		i = 0;

	return 0;
}

int main (int argc, char * argv [])
{
	BKInt const numChannels = 2;
	BKInt const sampleRate  = 44100;

	BKContextInit (& ctx, numChannels, sampleRate);

	BKTrackInit (& track, BK_SQUARE);

	BKSetAttr (& track, BK_MASTER_VOLUME, 0.2 * BK_MAX_VOLUME);
	BKSetAttr (& track, BK_VOLUME,        0.5 * BK_MAX_VOLUME);

	// portamento
	BKSetAttr (& track, BK_EFFECT_PORTAMENTO, 35);

	BKTrackAttach (& track, & ctx);


	//// instrument with release sequence
	BKInstrumentInit (& instrument);


	BKInstrumentSetEnvelopeADSR (& instrument, 30, 10, 0.5 * BK_MAX_VOLUME, 80);

	BKSequencePhase const panning [] = {
		{10, 0},
		{50, -BK_MAX_VOLUME},
		{50, +BK_MAX_VOLUME},
		{10, 0},
	};

	BKInstrumentSetEnvelope (& instrument, BK_SEQUENCE_PANNING, panning, 4, 1, 2);

	BKSetPtr (& track, BK_INSTRUMENT, & instrument, 0);
	////

	// Callback struct used for initializing divider
	BKCallback callback;

	callback.func     = dividerCallback;
	callback.userInfo = NULL;

	// We want 150 BPM
	// The master clock ticks at 240 Hz
	// This results to an divider value of 96
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
		//BKSetPtr (& sawtooth, BK_EFFECT_VIBRATO, vibrato);

		SDL_UnlockAudio ();

		if (c == 'q')
			break;
	}

	printf ("\n");

	SDL_PauseAudio (1);
	SDL_CloseAudio ();


	BKDispose (& divider);
	BKDispose (& instrument);
	BKDispose (& track);
	BKDispose (& ctx);

    return 0;
}
