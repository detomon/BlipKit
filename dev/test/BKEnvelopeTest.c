#if 0
clang -Wall -O2 -I ../../src/ -o /tmp/`basename $0` $0 -lblipkit -lSDL2 && /tmp/`basename $0` $@; exit
#endif

/**
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

#include "BlipKit.h"
#include "SDL.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

typedef struct {
	BKInt numChannels;
	BKInt sampleRate;
} BKSDLUserData;

BKContext ctx;
BKTrack organ;
BKInstrument instrument;
BKSDLUserData userData;
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

BKEnum dividerCallback(BKCallbackInfo* info, void* userData) {
	// clang-format off

	static BKInt notes [32] = {
		BK_D_3, -3, -3, -3,
		-3, -3, -3, -3,
		-3, -3, -3, -3,
		-3, -3, -3, -3,
		BK_D_1, -3, -3, -3,
		-3, -3, -3, -1,
		-3, -3, -3, -3,
		-3, -3, -3, -3,
	};

	// clang-format on

	BKInt note = notes[i];

	if (note >= 0) {
		note *= BK_FINT20_UNIT;
	}

	// Set track note
	if (note != -3) {
		BKSetAttr(&organ, BK_NOTE, note);
	}

	i++;

	if (i >= 32) {
		i = 0;
	}

	return 0;
}

#ifdef main
#undef main
#endif

FILE* out;

BKInt framesWrite(BKFrame inFrames[], BKUInt size, void* info) {
	fwrite(inFrames, sizeof(BKFrame), size * 2, out);

	printf("*write %d frames\n", size);

	return 0;
}

int main(int argc, char* argv[]) {
	BKInt const numChannels = 2;
	BKInt const sampleRate = 44100;

	BKContextInit(&ctx, numChannels, sampleRate);

	BKTrackInit(&organ, BK_SQUARE);

	BKSetAttr(&organ, BK_MASTER_VOLUME, 0.2 * BK_MAX_VOLUME);
	BKSetAttr(&organ, BK_VOLUME, 1.0 * BK_MAX_VOLUME);

	BKTrackAttach(&organ, &ctx);

	//// instrument with release sequence
	BKInstrumentInit(&instrument);

#define NUM_SEQUENCE_PHASES 19

	// clang-format off

	BKSequencePhase const arpeggio[5] = {
		{24, 4 * BK_FINT20_UNIT},
		{24, 3 * BK_FINT20_UNIT},
		{24, 12 * BK_FINT20_UNIT},
		{24, 0 * BK_FINT20_UNIT},
		{200, 0 * BK_FINT20_UNIT},
	};

	BKSequencePhase const dutyCycle[2] = {
		{1, 0},
		{120, 8},
	};

	BKSequencePhase const panning[3] = {
		{120, +BK_MAX_VOLUME},
		{120, -BK_MAX_VOLUME},
		{80, 0},
	};

	// clang-format on

	BKSetAttr(&organ, BK_DUTY_CYCLE, 8);

	BKInstrumentSetEnvelopeADSR(&instrument, 20, 20, 0.75 * BK_MAX_VOLUME, 200);
	BKInstrumentSetEnvelope(&instrument, BK_SEQUENCE_PITCH, arpeggio, 5, 0, 4);
	BKInstrumentSetEnvelope(&instrument, BK_SEQUENCE_DUTY_CYCLE, dutyCycle, 2, 0, 1);
	BKInstrumentSetEnvelope(&instrument, BK_SEQUENCE_PANNING, panning, 3, 0, 2);

	BKSetAttr(&organ, BK_EFFECT_PORTAMENTO, 30);

	BKSetPtr(&organ, BK_INSTRUMENT, &instrument, sizeof(&instrument));

	////

	// Callback struct used for initializing divider
	BKCallback callback;

	callback.func = dividerCallback;
	callback.userInfo = NULL;

	// We want 120 BPM
	// The master clock ticks at 240 Hz
	// This results to an divider value of 80
	// Note that a divider only takes integer values
	// Certain BPM rates are not possible as integers may be rounded down
	BKInt dividerValue = (60.0 / 150.0 / 4) * 240;

	// Initialize divider with divider value and callback
	BKDividerInit(&divider, dividerValue, &callback);

	// Attach the divider to the master clock
	// When samples are generated the callback is called at the defined interval
	BKContextAttachDivider(&ctx, &divider, BK_CLOCK_TYPE_BEAT);

	/*BKTime time;

	out = fopen ("/Users/simon/Desktop/bla.raw", "w");

	time = BKTimeFromSeconds (& ctx, 1.0);
	BKContextGenerateToTime (& ctx, time, framesWrite, NULL);
	time = BKTimeAdd (time, BKTimeFromSeconds (& ctx, 1.0));
	BKContextGenerateToTime (& ctx, time, framesWrite, NULL);*/

	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioSpec wanted;

	userData.numChannels = numChannels;
	userData.sampleRate = sampleRate;

	wanted.freq = sampleRate;
	wanted.format = AUDIO_S16SYS;
	wanted.channels = numChannels;
	wanted.samples = 512;
	wanted.callback = (void*)fill_audio;
	wanted.userdata = &userData;

	if (SDL_OpenAudio(&wanted, NULL) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		return 1;
	}

	SDL_PauseAudio(0);

	printf("Press [q] to stop\n");

	while (1) {
		int c = getchar_nocanon(0);

		// Use lock when setting attributes outside of divider callbacks
		SDL_LockAudio();

		// BKInt vibrato [2] = {16, 3 * BK_FINT20_UNIT};
		// BKTrackSetPtr (& sawtooth, BK_EFFECT_VIBRATO, vibrato);

		SDL_UnlockAudio();

		if (c == 'q') {
			printf("stop\n");
			break;
		}
	}

	printf("\n");

	SDL_PauseAudio(1);
	SDL_CloseAudio();

	BKDispose(&divider);
	BKDispose(&instrument);
	BKDispose(&organ);
	BKDispose(&ctx);

	return 0;
}
