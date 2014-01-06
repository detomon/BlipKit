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

#include <stdlib.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include "BlipKit.h"
#include "BKChip.h"

const int sampleRate  = 44100;
const int numChannels = 2;

const static struct BKChipTrackDescription trackDescription [BK_CHIP_NUM_TRACKS] =
{
	{.waveform = BK_SQUARE,   .masterVolume = BK_MAX_VOLUME * 0.15},
	{.waveform = BK_SQUARE,   .masterVolume = BK_MAX_VOLUME * 0.15},
	{.waveform = BK_TRIANGLE, .masterVolume = BK_MAX_VOLUME * 0.30},
	{.waveform = BK_NOISE,    .masterVolume = BK_MAX_VOLUME * 0.15},
	{.waveform = BK_SAWTOOTH, .masterVolume = BK_MAX_VOLUME * 0.25},
};

BKChip server;

static void SDLCallback (BKChip * server, Uint8 * stream, int len)
{
	BKUInt numFrames = len / sizeof (BKFrame) / server -> numChannels;

	BKContextGenerate (& server -> ctx, (BKFrame *) stream, numFrames);
}

static int initSDL (BKChip * server, int sampleRate, int numChannels)
{	
	SDL_AudioSpec wanted;

	wanted.freq     = sampleRate;
	wanted.format   = AUDIO_S16SYS;
	wanted.channels = numChannels;
	wanted.samples  = 1024;
	wanted.callback = (void *) SDLCallback;
	wanted.userdata = server;

	SDL_Init (SDL_INIT_AUDIO);
	
	if (SDL_OpenAudio (& wanted, NULL) < 0) {
		fprintf (stderr, "Couldn't open audio: %s\n", SDL_GetError ());
		return 1;
	}
	
	return 0;
}

static int interpretCommand (BKChip * chip, char const * line)
{
	BKInt          trackIndex = 0;
	BKInt          attrIndex  = 0;
	BKInt          value0     = 0;
	BKInt          value1     = 0;
	BKChipCommand  cmd;
	BKChipResponse response;

	if (sscanf (line, "%d %d %d %d", & trackIndex, & attrIndex, & value0, & value1) >= 2) {
		memset (& cmd, 0, sizeof (cmd));
		cmd.track = trackIndex;
		cmd.attr  = attrIndex;
		cmd.values [0] = value0;
		cmd.values [1] = value1;

		BKChipPushCommand (chip, & cmd, & response);
		
		if (response.hasResponse)
			printf ("%u %u\n", response.values [0], response.values [1]);
	}
	else {
		fprintf (stderr, "Invalid command\n");
	}
	
	return 0;
}

static int runLoop (BKChip * server)
{
	char line [1024];

    SDL_PauseAudio (0);

	do {
		fgets (line, sizeof (line), stdin);

		interpretCommand (server, line);
	}
	while (1);

    SDL_PauseAudio (1);
	SDL_CloseAudio ();
	
	return 0;
}

#ifdef main
#undef main
#endif

int main (int argc, const char * argv [])
{
	if (BKChipInit (& server, numChannels, sampleRate, trackDescription, BK_CHIP_NUM_TRACKS) != 0) {
		fprintf (stderr, "Failed to initialize BKContext");
		return EXIT_FAILURE;
	}

	if (initSDL (& server, sampleRate, numChannels) != 0)
		return EXIT_FAILURE;

	if (runLoop (& server) != 0) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
