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
BKTrack       left, right, square1, square2;
BKSDLUserData userData;

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

#ifdef main
#undef main
#endif

int main (int argc, char * argv [])
{
	BKInt const numChannels = 2;
	BKInt const sampleRate  = 44100;

	BKContextInit (& ctx, numChannels, sampleRate);

	BKTrackInit (& left, BK_NOISE);

	BKTrackSetAttr (& left, BK_MASTER_VOLUME, 0.03 * BK_MAX_VOLUME);
	BKTrackSetAttr (& left, BK_VOLUME,        1.0 * BK_MAX_VOLUME);
	BKTrackSetAttr (& left, BK_PANNING,      -BK_MAX_VOLUME);
	BKTrackSetAttr (& left, BK_NOTE,          BK_A_1 * BK_FINT20_UNIT);

	BKTrackAttach (& left, & ctx);


	BKTrackInit (& right, BK_NOISE);

	BKTrackSetAttr (& right, BK_MASTER_VOLUME, 0.03 * BK_MAX_VOLUME);
	BKTrackSetAttr (& right, BK_VOLUME,        1.0 * BK_MAX_VOLUME);
	BKTrackSetAttr (& right, BK_PANNING,      +BK_MAX_VOLUME);
	BKTrackSetAttr (& right, BK_NOTE,          BK_A_1 * BK_FINT20_UNIT);
	BKTrackSetAttr (& right, BK_PHASE,         16000);

	BKTrackAttach (& right, & ctx);


	BKTrackInit (& square1, BK_SQUARE);

	BKTrackSetAttr (& square1, BK_MASTER_VOLUME, 0.025 * BK_MAX_VOLUME);
	BKTrackSetAttr (& square1, BK_VOLUME,        1.0 * BK_MAX_VOLUME);
	BKTrackSetAttr (& square1, BK_DUTY_CYCLE,    5);
	BKTrackSetAttr (& square1, BK_PANNING,      -0.5 * BK_MAX_VOLUME);
	BKTrackSetAttr (& square1, BK_NOTE,          BK_A_3 * BK_FINT20_UNIT);

	BKTrackAttach (& square1, & ctx);


	BKTrackInit (& square2, BK_SQUARE);

	BKTrackSetAttr (& square2, BK_MASTER_VOLUME, 0.025 * BK_MAX_VOLUME);
	BKTrackSetAttr (& square2, BK_VOLUME,        1.0 * BK_MAX_VOLUME);
	BKTrackSetAttr (& square2, BK_DUTY_CYCLE,    7);
	BKTrackSetAttr (& square2, BK_PANNING,      +0.5 * BK_MAX_VOLUME);
	BKTrackSetAttr (& square2, BK_NOTE,          BK_A_3 * BK_FINT20_UNIT);

	BKTrackAttach (& square2, & ctx);


	BKInt vibrato [2] = {1300, 12 * BK_FINT20_UNIT};
	BKTrackSetPtr (& left, BK_EFFECT_VIBRATO, vibrato);

	BKInt vibrato2 [2] = {1700, 12 * BK_FINT20_UNIT};
	BKTrackSetPtr (& right, BK_EFFECT_VIBRATO, vibrato2);

	BKInt tremolo [2] = {400, 0.8 * BK_MAX_VOLUME};
	BKTrackSetPtr (& square1, BK_EFFECT_TREMOLO, tremolo);

	BKInt vibrato3 [2] = {700, 3 * BK_FINT20_UNIT};
	BKTrackSetPtr (& square1, BK_EFFECT_VIBRATO, vibrato3);

	BKInt tremolo2 [2] = {800, 0.8 * BK_MAX_VOLUME};
	BKTrackSetPtr (& square2, BK_EFFECT_TREMOLO, tremolo2);

	BKInt vibrato4 [2] = {200, 3 * BK_FINT20_UNIT};
	BKTrackSetPtr (& square2, BK_EFFECT_VIBRATO, vibrato4);



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

		if (c == 'q')
			break;
	}

	printf ("\n");

	SDL_PauseAudio (1);
	SDL_CloseAudio ();


	BKTrackDispose (& left);
	BKTrackDispose (& right);
	BKTrackDispose (& square1);
	BKTrackDispose (& square2);
	BKContextDispose (& ctx);

    return 0;
}
