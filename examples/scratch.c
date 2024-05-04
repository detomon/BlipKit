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
BKTrack noise, triangle;
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

	static BKInt notes[32] = {
		BK_C_4, -1, BK_F_4, -1,
		BK_C_6, BK_E_4, BK_G_SH_4, -1,
		-1, -1, -1, -1,
		BK_C_6, -1, BK_A_4, BK_A_4,

		-1, -1, BK_G_4, -1,
		BK_C_6, -1, -1, -1,
		-1, -1, BK_G_4, BK_G_4,
		BK_C_6, -1, -1, -1,
	};

	static BKInt notes2[32] = {
		BK_C_1, BK_C_1, BK_F_1, -1,
		BK_C_1, BK_C_1, BK_F_0, -1,
		BK_C_1, BK_C_1, BK_F_1, -1,
		BK_C_1, BK_C_1, BK_F_0, -1,

		BK_C_1, -1, BK_F_0, BK_C_0,
		BK_G_1, -1, -1, -1,
		BK_C_1, -1, BK_G_0, BK_C_0,
		BK_E_1, -1, -1, -1,
	};

	// clang-format on

	BKInt note = notes[i];
	BKInt note2 = notes2[i];

	if (note == BK_C_6) {
		BKSetAttr(&noise, BK_PANNING, 0);
		BKSetAttr(&noise, BK_PHASE_WRAP, 0);
	}
	else {
		if (i < 20) {
			BKSetAttr(&noise, BK_PANNING, 0.33 * BK_MAX_VOLUME);
		}
		else {
			BKSetAttr(&noise, BK_PANNING, -0.33 * BK_MAX_VOLUME);
		}

		BKSetAttr(&noise, BK_PHASE_WRAP, 64);
	}

	if (note >= 0)
		note *= BK_FINT20_UNIT;

	if (note2 >= 0)
		note2 *= BK_FINT20_UNIT;

	// Set track note
	BKSetAttr(&noise, BK_NOTE, note);

	// Set track note
	BKSetAttr(&triangle, BK_NOTE, note2);

	i++;

	if (i >= 32)
		i = 0;

	return 0;
}

int main(int argc, char* argv[]) {
	BKContextInit(&ctx, userData.numChannels, userData.sampleRate);

	BKTrackInit(&noise, BK_NOISE);

	BKSetAttr(&noise, BK_MASTER_VOLUME, 0.2 * BK_MAX_VOLUME);
	BKSetAttr(&noise, BK_VOLUME, 0.4 * BK_MAX_VOLUME);
	BKSetAttr(&noise, BK_PHASE_WRAP, 64);

	BKTrackAttach(&noise, &ctx);

	BKTrackInit(&triangle, BK_TRIANGLE);

	BKSetAttr(&triangle, BK_MASTER_VOLUME, 0.4 * BK_MAX_VOLUME);
	BKSetAttr(&triangle, BK_VOLUME, 1.0 * BK_MAX_VOLUME);

	BKSetAttr(&triangle, BK_EFFECT_PORTAMENTO, 6);

	BKTrackAttach(&triangle, &ctx);

	BKInt vibrato[] = { 20, 12 * BK_FINT20_UNIT };
	BKSetPtr(&noise, BK_EFFECT_VIBRATO, vibrato, sizeof(vibrato));

	// We want 180 BPM
	// The master clock ticks at 240 Hz
	// This results to an divider value of 80
	// Divide by 4 to get a 4/4 beat
	// Note that a divider only takes integer values
	// Certain BPM rates are not possible as integers may be rounded down
	BKInt dividerValue = (60.0 / 180.0 / 4) * 240;

	// Initialize divider with divider value and callback
	BKDividerInit(&divider, dividerValue, &(BKCallback){
											  .func = dividerCallback,
											  .userInfo = NULL,
										  });

	// Attach the divider to the master clock
	// When samples are generated the callback is called at the defined interval
	BKContextAttachDivider(&ctx, &divider, BK_CLOCK_TYPE_BEAT);

	SDL_Init(SDL_INIT_AUDIO);

	SDL_AudioSpec wanted = {
		.freq = userData.sampleRate,
		.format = AUDIO_S16SYS,
		.channels = userData.numChannels,
		.samples = 512,
		.callback = (void*)fill_audio,
		.userdata = &userData,
	};

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
		// BKSetPtr (& sawtooth, BK_EFFECT_VIBRATO, vibrato);

		SDL_UnlockAudio();

		if (c == 'q')
			break;
	}

	printf("\n");

	SDL_PauseAudio(1);
	SDL_CloseAudio();

	BKDispose(&divider);
	BKDispose(&noise);
	BKDispose(&triangle);
	BKDispose(&ctx);

	return 0;
}
