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
BKTrack       square;
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

	BKTrackInit (& square, BK_SQUARE);

	BKSetAttr (& square, BK_MASTER_VOLUME, 0.1 * BK_MAX_VOLUME);
	BKSetAttr (& square, BK_VOLUME,        1.0 * BK_MAX_VOLUME);
	BKSetAttr (& square, BK_DUTY_CYCLE,    8);
	BKSetAttr (& square, BK_NOTE,          BK_C_4 * BK_FINT20_UNIT);

	// set tremolo effect
	BKInt tremolo [2] = {18, 0.5 * BK_FINT20_UNIT};
	BKSetPtr (& square, BK_EFFECT_VIBRATO, tremolo, sizeof (tremolo));

	BKInstrument instr;

	BKInstrumentInit (&instr);

	BKInt dutyCycle [9] = {4, 6, 4, 2, 2, 1, 7, 8, 7};
	BKInt arpeggio [4] = {0, 4 * BK_FINT20_UNIT, 7 * BK_FINT20_UNIT, -12 * BK_FINT20_UNIT};
	BKInt panning [4] = {0, BK_MAX_VOLUME * 0.5, 0, -BK_MAX_VOLUME * 0.5};

	BKInstrumentSetSequence (&instr, BK_SEQUENCE_DUTY_CYCLE, dutyCycle, 9, 0, 9);
	BKInstrumentSetSequence (&instr, BK_SEQUENCE_ARPEGGIO, arpeggio, 4, 0, 4);
	BKInstrumentSetSequence (&instr, BK_SEQUENCE_PANNING, panning, 4, 0, 4);

	// attach to context
	BKTrackAttach (& square, & ctx);

	BKSetPtr (& square, BK_INSTRUMENT, & instr, sizeof (& instr));

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


	BKDispose (& square);
	BKDispose (& ctx);

    return 0;
}
