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

#ifndef _TRACK_H_
#define _TRACK_H_

#include <SDL/SDL.h>
#include "BlipKit.h"
#include "BKCompiler.h"

typedef struct {
	BKTrack       * track;
	BKInterpreter * interpreter;
} BKSDLUserData;

typedef struct {
	BKTrack       track;
	BKDivider     divider;
	BKInterpreter interpreter;
	BKSDLUserData userData;
} BKSDLTrack;

typedef struct {
	BKContext      ctx;
	BKUInt         speed;
	BKInstrument * instruments [64];
	BKUInt         numInstruments;
	BKData       * waveforms [64];
	BKUInt         numWaveforms;
	BKData       * samples [64];
	BKUInt         numSamples;
	BKSDLTrack   * tracks [64];
	BKUInt         numTracks;
} BKSDLContext;

/**
 *
 */
extern BKInt BKSDLContextInit (BKSDLContext * ctx, BKUInt numChannels, BKUInt sampleRate);

/**
 *
 */
extern void BKSDLContextDispose (BKSDLContext * ctx);

/**
 *
 */
extern BKInt BKSDLContextLoadData (BKSDLContext * ctx, void const * data, size_t size);

/**
 *
 */
extern BKInt BKSDLContextLoadFile (BKSDLContext * ctx, char const * filename);

/**
 *
 */
extern void BKSDLContextUnloadData (BKSDLContext * ctx);

/**
 *
 */
extern void BKSDLContextReset (BKSDLContext * ctx, BKInt resetTracks);

#endif /* ! _TRACK_H_ */
