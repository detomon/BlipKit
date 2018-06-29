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
BKTrack       sampleTrack;
BKData        sample;
BKInstrument  instrument;
BKSDLUserData userData = {
	.numChannels = 2,
	.sampleRate  = 44100,
};
BKDivider     divider;
BKInt         i = 0;

BKInt offset = 0;

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
	static BKInt notes [32] = {
		BK_D_2, -3, -3, BK_NOTE_RELEASE,
		BK_E_3, -3, -3, BK_NOTE_RELEASE,
		BK_D_4, -3, BK_NOTE_RELEASE, -3,
		-3, -3, -3, -3,
		BK_A_2, -3, -3, -3,
		BK_NOTE_RELEASE, -3, -3, -3,
		BK_F_2, -3, -3, -3,
		BK_NOTE_RELEASE, -3, -3, -3,
	};

	BKInt note = notes [i];

	if (note >= 0)
		note *= BK_FINT20_UNIT;

	// Set track note
	if (note != -3)
		BKSetAttr (& sampleTrack, BK_NOTE, note);

	i ++;

	if (i >= 32)
		i = 0;

	return 0;
}

int main (int argc, char * argv [])
{
	BKContextInit (& ctx, userData.numChannels, userData.sampleRate);

	BKTrackInit (& sampleTrack, BK_SQUARE);

	BKSetAttr (& sampleTrack, BK_MASTER_VOLUME, 0.2 * BK_MAX_VOLUME);
	BKSetAttr (& sampleTrack, BK_VOLUME,        1.0 * BK_MAX_VOLUME);

	BKTrackAttach (& sampleTrack, & ctx);

	FILE * file = fopen ("itemland.wav", "rb");

	BKDataInit (& sample);

	// Load raw sound data
	BKDataLoadWAVE (& sample, file);

	fclose (file);

	// Normalize frames to maximum amplitude
	BKDataNormalize (& sample);

	// Tune sample pitch to BK_C_4 (approximately)
	BKSetAttr (& sample, BK_SAMPLE_PITCH, -0.9 * BK_FINT20_UNIT);


	// Set data object as sample
	BKSetPtr (& sampleTrack, BK_SAMPLE, & sample, 0);

	// set play range
	BKInt range [2] = {500, 800};
	BKSetPtr (& sampleTrack, BK_SAMPLE_SUSTAIN_RANGE, range, sizeof (range));

	//BKSetAttr (& sampleTrack, BK_SAMPLE_REPEAT, BK_REPEAT);

	//// instrument with release sequence
	BKInstrumentInit (& instrument);

	#define NUM_VOLUME_PHASES 15

	BKInt volumeSequence [NUM_VOLUME_PHASES];

	// Create descending sequence
	for (BKInt i = 0; i < NUM_VOLUME_PHASES; i ++)
		volumeSequence [i] = ((float) BK_MAX_VOLUME * (NUM_VOLUME_PHASES - i) / NUM_VOLUME_PHASES);

	// Set volume sequence of instrument
	BKInstrumentSetSequence (& instrument, BK_SEQUENCE_VOLUME, volumeSequence, NUM_VOLUME_PHASES, 0, 1);

	// Attach instrument to track
	BKSetPtr (& sampleTrack, BK_INSTRUMENT, & instrument, 0);

	//BKSetAttr (& sampleTrack, BK_NOTE, BK_C_4 * BK_FINT20_UNIT);
	//BKInt const arpeggio [] = {6, 0, 0, 4 * BK_FINT20_UNIT, 4 * BK_FINT20_UNIT, 7 * BK_FINT20_UNIT, 7 * BK_FINT20_UNIT};
	//BKSetPtr (& sampleTrack, BK_ARPEGGIO, arpeggio, sizeof (arpeggio));
	//BKSetAttr (& sampleTrack, BK_EFFECT_PORTAMENTO, 2000);
	//BKSetAttr (& sampleTrack, BK_NOTE, BK_C_0 * BK_FINT20_UNIT);

	// We want 150 BPM
	// The master clock ticks at 240 Hz
	// This results to an divider value of 96
	// Divide by 4 to get a 4/4 beat
	// Note that a divider only takes integer values
	// Certain BPM rates are not possible as integers may be rounded down
	BKInt dividerValue = (60.0 / 150.0 / 4) * 240;

	// Initialize divider with divider value and callback
	BKDividerInit (& divider, dividerValue, &(BKCallback) {
		.func     = dividerCallback,
		.userInfo = NULL,
	});

	// Attach the divider to the master clock
	// When samples are generated the callback is called at the defined interval
	BKContextAttachDivider (& ctx, & divider, BK_CLOCK_TYPE_BEAT);

	SDL_Init (SDL_INIT_AUDIO);

	SDL_AudioSpec wanted = {
		.freq     = userData.sampleRate,
		.format   = AUDIO_S16SYS,
		.channels = userData.numChannels,
		.samples  = 512,
		.callback = (void *) fill_audio,
		.userdata = & userData,
	};

	if (SDL_OpenAudio (& wanted, NULL) < 0) {
		fprintf (stderr, "Couldn't open audio: %s\n", SDL_GetError ());
		return 1;
	}

	SDL_PauseAudio (0);

	printf ("Press [q] to stop\n");

	while (1) {
		int c = getchar_nocanon (0);

		// Use lock when setting attributes outside of divider callbacks
		//SDL_LockAudio ();

		// Do stuff ...
		//BKInt vibrato [2] = {16, 3 * BK_FINT20_UNIT};
		//BKSetPtr (& sawtooth, BK_EFFECT_VIBRATO, vibrato);

		//SDL_UnlockAudio ();

		if (c == 'q')
			break;
	}

	printf ("\n");

	SDL_PauseAudio (1);
	SDL_CloseAudio ();

	BKDispose (& divider);
	BKDispose (& sample);
	BKDispose (& instrument);
	BKDispose (& sampleTrack);
	BKDispose (& ctx);

    return 0;
}
