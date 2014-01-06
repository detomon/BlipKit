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

#ifndef _BK_CHIP_H_
#define _BK_CHIP_H_

#include <SDL/SDL.h>
#include "BlipKit.h"

#define BK_CHIP_NUM_TRACKS 5

typedef struct BKChip                       BKChip;
typedef const struct BKChipTrackDescription BKChipTrackDescription;
typedef struct BKChipCommand                BKChipCommand;
typedef struct BKChipResponse               BKChipResponse;

struct BKChip
{
	BKInt     sampleRate;
	BKInt     numChannels;
	BKInt     numTracks;
	BKContext ctx;
	BKTrack * tracks;
};

struct BKChipTrackDescription
{
	BKInt waveform;
	BKInt masterVolume;
};

struct BKChipCommand
{
	BKInt track;
	BKInt attr;
	BKInt values [2];
};

struct BKChipResponse
{
	BKInt hasResponse;
	BKInt values [2];
};

enum
{
	BK_CHIP_ATTR_VOLUME,
	BK_CHIP_ATTR_PANNING,
	BK_CHIP_ATTR_NOTE,
	BK_CHIP_ATTR_DUTY_CYCLE,
	BK_CHIP_ATTR_EFFECT_VOLUME_SLIDE,
	BK_CHIP_ATTR_EFFECT_PANNING_SLIDE,
	BK_CHIP_ATTR_EFFECT_PORTAMENTO,
	BK_CHIP_ATTR_EFFECT_TREMOLO,
	BK_CHIP_ATTR_EFFECT_VIBRATO,
};

#define BK_CHIP_ATTR_RESPONSE_FLAG (1 << 30)

/**
 *
 */
extern BKInt BKChipInit (BKChip * chip, BKInt numChannels, BKInt sampleRate, BKChipTrackDescription const * trackDescriptions, BKInt numTracks);

/**
 *
 */
extern void BKChipDispose (BKChip * chip);

/**
 *
 */
extern BKInt BKChipPushCommand (BKChip * chip, BKChipCommand const * command, BKChipResponse * response);

#endif /* ! _BK_CHIP_H_ */
