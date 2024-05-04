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

BKContext ctx;
BKTrack square, sawtooth, triangle;
BKSDLUserData userData = {
	.numChannels = 2,
	.sampleRate = 44100,
};
BKDivider divider;
BKInt i = 0;

static int getchar_nocanon(unsigned tcflags) {
	int c;
	struct termios oldtc, newtc;

	tcgetattr(STDIN_FILENO, &oldtc);

	newtc = oldtc;
	newtc.c_lflag &= ~(ICANON | ECHO | tcflags);

	tcsetattr(STDIN_FILENO, TCSANOW, &newtc);
	c = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldtc);

	return c;
}

static void fill_audio(BKSDLUserData* info, Uint8* stream, int len) {
	// calculate needed frames for one channel
	BKUInt numFrames = len / sizeof(BKFrame) / info->numChannels;

	BKContextGenerate(&ctx, (BKFrame*)stream, numFrames);
}

static BKEnum dividerCallback(BKCallbackInfo* info, void* userData) {
	// clang-format off

	static BKInt notes[16] = {
		BK_A_1, -1, BK_F_1, -1,
		BK_E_1, -1, BK_G_SH_1, -1,
		BK_A_1, -1, BK_D_1, -1,
		BK_F_1, -1, BK_G_SH_1, -1,
	};

	static BKInt notes2[16] = {
		-1, BK_A_3, -1, -1,
		BK_G_3, BK_G_3, -1, -1,
		-1, BK_D_3, -1, -1,
		BK_D_4, BK_D_4, BK_E_4, -1,
	};

	// clang-format om

	BKInt note = notes [i];
	BKInt note2 = notes2 [i];

	if (note >= 0)
		note *= BK_FINT20_UNIT;

	// Set track note
	BKSetAttr (& sawtooth, BK_NOTE, note);

	if (note >= 0)
		note += 7 * BK_FINT20_UNIT;

	BKSetAttr (& triangle, BK_NOTE, note);

	if (note2 >= 0)
		note2 *= BK_FINT20_UNIT;

	BKSetAttr (& square, BK_NOTE, note2);


	i ++;

	if (i >= 16)
		i = 0;

	return 0;
}

int main (int argc, char * argv [])
{
	BKContextInit (& ctx, userData.numChannels, userData.sampleRate);

	BKTrackInit (& square, BK_SQUARE);

	BKSetAttr (& square, BK_MASTER_VOLUME, 0.15 * BK_MAX_VOLUME);
	BKSetAttr (& square, BK_VOLUME,        1.0 * BK_MAX_VOLUME);
	BKSetAttr (& square, BK_DUTY_CYCLE,    2);

	BKTrackInit (& sawtooth, BK_SAWTOOTH);

	BKSetAttr (& sawtooth, BK_MASTER_VOLUME, 0.15 * BK_MAX_VOLUME);
	BKSetAttr (& sawtooth, BK_VOLUME,        1.0 * BK_MAX_VOLUME);

	BKTrackInit (& triangle, BK_TRIANGLE);

	BKSetAttr (& triangle, BK_MASTER_VOLUME, 0.3 * BK_MAX_VOLUME);
	BKSetAttr (& triangle, BK_VOLUME,        1.0 * BK_MAX_VOLUME);

	BKInstrument instrument;

	BKInstrumentInit (& instrument);

	BKInt sequence [6] = {
		1.0 * BK_MAX_VOLUME, 0.8 * BK_MAX_VOLUME, 0.4 * BK_MAX_VOLUME,
		0.4 * BK_MAX_VOLUME, 0.2 * BK_MAX_VOLUME, 0.0 * BK_MAX_VOLUME,
	};

	BKInstrumentSetSequence (& instrument, BK_SEQUENCE_VOLUME, sequence, 6, 0, 1);

	BKInt vibrato [2] = {12, 0.25 * BK_FINT20_UNIT};
	BKSetPtr (& square, BK_EFFECT_VIBRATO, vibrato, sizeof (vibrato));


	BKTrackAttach (& square, & ctx);
	BKTrackAttach (& sawtooth, & ctx);
	BKTrackAttach (& triangle, & ctx);

	BKSetPtr (& square, BK_INSTRUMENT, & instrument, 0);
	BKSetPtr (& sawtooth, BK_INSTRUMENT, & instrument, 0);

	// We want 150 BPM
	// The master clock ticks at 240 Hz
	// This results to an divider value of 96
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

		/*
		// Use lock when setting attributes outside of divider callbacks
		SDL_LockAudio ();

		//BKInt vibrato [2] = {16, 3 * BK_FINT20_UNIT};
		//BKSetPtr (& sawtooth, BK_EFFECT_VIBRATO, vibrato, sizeof (vibrato));

		SDL_UnlockAudio ();
		*/

		if (c == 'q')
			break;
	}

	printf ("\n");

	SDL_PauseAudio (1);
	SDL_CloseAudio ();

    return 0;
}
