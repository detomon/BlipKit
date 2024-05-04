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

typedef struct {
	BKTrack track;
	BKClock clock;
	BKUInt tick;
	BKUInt notesCount;
	BKInt const* notes;
} TrackContext;

BKContext ctx;
TrackContext square, sawtooth, triangle;
BKInstrument instrument;

BKSDLUserData userData = {
	.numChannels = 2,
	.sampleRate = 44100,
};

// clang-format off

static BKInt const notes[16] = {
	BK_A_1, -1, BK_F_1, -1,
	BK_E_1, -1, BK_G_SH_1, -1,
	BK_A_1, -1, BK_D_1, -1,
	BK_F_1, -1, BK_G_SH_1, -1,
};

// clang-format on

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

static BKEnum clockCallback(BKCallbackInfo* info, TrackContext* trackCtx) {
	BKTrack* track = &trackCtx->track;
	BKInt note = trackCtx->notes[trackCtx->tick];

	if (note >= 0) {
		note *= BK_FINT20_UNIT;
	}

	BKSetAttr(track, BK_NOTE, note);

	trackCtx->tick++;

	if (trackCtx->tick >= trackCtx->notesCount) {
		trackCtx->tick = 0;
	}

	return BK_SUCCESS;
}

static void trackContextInit(TrackContext* trackCtx, BKContext* ctx, BKEnum waveform, BKUInt masterVolume, BKTime clockPeriod) {
	BKTrack* track = &trackCtx->track;
	BKClock* clock = &trackCtx->clock;

	trackCtx->notesCount = 16;
	trackCtx->notes = notes;

	BKTrackInit(track, waveform);

	BKSetAttr(track, BK_MASTER_VOLUME, masterVolume);
	BKSetAttr(track, BK_VOLUME, 1.0 * BK_MAX_VOLUME);
	BKSetPtr(track, BK_INSTRUMENT, &instrument, sizeof(void*));

	BKTrackAttach(track, ctx);

	BKClockInit(clock, clockPeriod, &(BKCallback){
										.func = (BKEnum(*)(BKCallbackInfo*, void*))clockCallback,
										.userInfo = trackCtx,
									});

	BKClockAttach(clock, ctx, NULL);
}

static void trackContextDispose(TrackContext* trackCtx) {
	BKDispose(&trackCtx->track);
	BKDispose(&trackCtx->clock);
}

int main(int argc, char* argv[]) {
	BKInstrumentInit(&instrument);

	BKInstrumentSetEnvelopeADSR(&instrument, 2, 0, BK_MAX_VOLUME, 20);

	BKContextInit(&ctx, userData.numChannels, userData.sampleRate);

	trackContextInit(&square, &ctx, BK_SQUARE, 0.15 * BK_MAX_VOLUME, BKTimeFromSeconds(&ctx, 1.0 / 4.0));
	trackContextInit(&sawtooth, &ctx, BK_SAWTOOTH, 0.15 * BK_MAX_VOLUME, BKTimeFromSeconds(&ctx, 1.0 / 3.0));
	trackContextInit(&triangle, &ctx, BK_TRIANGLE, 0.30 * BK_MAX_VOLUME, BKTimeFromSeconds(&ctx, 1.0 / 2.0));

	BKSetAttr(&square.track, BK_PITCH, 24 * BK_FINT20_UNIT);
	BKSetAttr(&sawtooth.track, BK_PITCH, 12 * BK_FINT20_UNIT);

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

		if (c == 'q') {
			break;
		}
	}

	printf("\n");

	SDL_PauseAudio(1);
	SDL_CloseAudio();

	trackContextDispose(&square);
	trackContextDispose(&sawtooth);
	trackContextDispose(&triangle);
	BKDispose(&ctx);
	BKDispose(&instrument);

	return 0;
}
