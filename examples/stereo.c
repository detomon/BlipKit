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
BKTrack left, right, square1, square2;
BKSDLUserData userData = {
	.numChannels = 2,
	.sampleRate = 44100,
};

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

int main(int argc, char* argv[]) {
	BKContextInit(&ctx, userData.numChannels, userData.sampleRate);

	BKTrackInit(&left, BK_NOISE);

	BKSetAttr(&left, BK_MASTER_VOLUME, 0.03 * BK_MAX_VOLUME);
	BKSetAttr(&left, BK_VOLUME, 1.0 * BK_MAX_VOLUME);
	BKSetAttr(&left, BK_PANNING, -BK_MAX_VOLUME);
	BKSetAttr(&left, BK_NOTE, BK_A_1 * BK_FINT20_UNIT);

	BKTrackAttach(&left, &ctx);

	BKTrackInit(&right, BK_NOISE);

	BKSetAttr(&right, BK_MASTER_VOLUME, 0.03 * BK_MAX_VOLUME);
	BKSetAttr(&right, BK_VOLUME, 1.0 * BK_MAX_VOLUME);
	BKSetAttr(&right, BK_PANNING, +BK_MAX_VOLUME);
	BKSetAttr(&right, BK_NOTE, BK_A_1 * BK_FINT20_UNIT);
	BKSetAttr(&right, BK_PHASE, 16000);

	BKTrackAttach(&right, &ctx);

	BKTrackInit(&square1, BK_SQUARE);

	BKSetAttr(&square1, BK_MASTER_VOLUME, 0.025 * BK_MAX_VOLUME);
	BKSetAttr(&square1, BK_VOLUME, 1.0 * BK_MAX_VOLUME);
	BKSetAttr(&square1, BK_DUTY_CYCLE, 5);
	BKSetAttr(&square1, BK_PANNING, -0.5 * BK_MAX_VOLUME);
	BKSetAttr(&square1, BK_NOTE, BK_A_3 * BK_FINT20_UNIT);

	BKTrackAttach(&square1, &ctx);

	BKTrackInit(&square2, BK_SQUARE);

	BKSetAttr(&square2, BK_MASTER_VOLUME, 0.025 * BK_MAX_VOLUME);
	BKSetAttr(&square2, BK_VOLUME, 1.0 * BK_MAX_VOLUME);
	BKSetAttr(&square2, BK_DUTY_CYCLE, 7);
	BKSetAttr(&square2, BK_PANNING, +0.5 * BK_MAX_VOLUME);
	BKSetAttr(&square2, BK_NOTE, BK_A_3 * BK_FINT20_UNIT);

	BKTrackAttach(&square2, &ctx);

	BKInt vibrato[2] = { 1300, 12 * BK_FINT20_UNIT };
	BKSetPtr(&left, BK_EFFECT_VIBRATO, vibrato, sizeof(vibrato));

	BKInt vibrato2[2] = { 1700, 12 * BK_FINT20_UNIT };
	BKSetPtr(&right, BK_EFFECT_VIBRATO, vibrato2, sizeof(vibrato2));

	BKInt tremolo[2] = { 400, 0.8 * BK_MAX_VOLUME };
	BKSetPtr(&square1, BK_EFFECT_TREMOLO, tremolo, sizeof(tremolo));

	BKInt vibrato3[2] = { 700, 3 * BK_FINT20_UNIT };
	BKSetPtr(&square1, BK_EFFECT_VIBRATO, vibrato3, sizeof(vibrato3));

	BKInt tremolo2[2] = { 800, 0.8 * BK_MAX_VOLUME };
	BKSetPtr(&square2, BK_EFFECT_TREMOLO, tremolo2, sizeof(tremolo2));

	BKInt vibrato4[2] = { 200, 3 * BK_FINT20_UNIT };
	BKSetPtr(&square2, BK_EFFECT_VIBRATO, vibrato4, sizeof(vibrato4));

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

		if (c == 'q')
			break;
	}

	printf("\n");

	SDL_PauseAudio(1);
	SDL_CloseAudio();

	BKDispose(&left);
	BKDispose(&right);
	BKDispose(&square1);
	BKDispose(&square2);
	BKDispose(&ctx);

	return 0;
}
