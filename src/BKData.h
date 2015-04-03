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

#ifndef _BK_DATA_H_
#define _BK_DATA_H_

#include <stdio.h>
#include "BKObject.h"

typedef struct BKData      BKData;
typedef struct BKDataState BKDataState;

typedef struct BKDataInfo        BKDataInfo;
typedef struct BKDataConvertInfo BKDataConvertInfo;
typedef struct BKDataExportInfo  BKDataExportInfo;

typedef BKInt (* BKDataStateCallback) (BKEnum event, void * userInfo);

/**
 * Endian
 */
enum
{
	BK_BIG_ENDIAN    = 1 << 16,
	BK_LITTLE_ENDIAN = 2 << 16,
	BK_ENDIAN_MASK   = 3 << 16,
};

/**
 * Bit
 */
enum
{
	BK_1_BIT_UNSIGNED  = 1,
	BK_2_BIT_UNSIGNED  = 2,
	BK_4_BIT_UNSIGNED  = 3,
	BK_8_BIT_SIGNED    = 4,
	BK_8_BIT_UNSIGNED  = 5,
	BK_16_BIT_SIGNED   = 6,
	BK_DATA_BITS_MASK  = 15,
};

/**
 * Callback events
 */
enum
{
	BK_DATA_STATE_EVENT_RESET,
	BK_DATA_STATE_EVENT_DISPOSE,
};

struct BKData
{
	BKObject      object;
	BKEnum        numBits;
	BKInt         sampleRate;
	BKUInt        numFrames;
	BKUInt        numChannels;
	BKFInt20      samplePitch;
	BKUInt        sustainOffset;
	BKUInt        sustainEnd;
	BKFrame     * frames;
	BKDataState * stateList;
};

struct BKDataState
{
	BKData            * data;
	BKDataStateCallback callback; // called when setting new frames
	void              * callbackUserInfo;
	BKDataState       * nextState;
};

struct BKDataConvertInfo
{
	BKInt  sourceSampleRate;
	BKInt  targetSampleRate;
	BKEnum targetNumBits;
	BKInt  ditherSmoothLength;
	float  ditherSlope;
	float  ditherCurve;
	float  threshold;
};

/**
 * Initialize data object
 */
extern BKInt BKDataInit (BKData * data);

/**
 * Allocate data object
 */
extern BKInt BKDataAlloc (BKData ** outData);

/**
 * Dispose data and detach from all tracks
 */
extern void BKDataDispose (BKData * data) BK_DEPRECATED_FUNC ("Use 'BKDispose' instead");

/**
 * Detach data from all tracks
 */
extern void BKDataDetach (BKData * data);

/**
 * Copy a data object
 * The copy will not be attached to any track
 */
extern BKInt BKDataInitCopy (BKData * copy, BKData const * original);

/**
 * BK_SAMPLE_PITCH
 *   Frames are assumed to be tuned in BK_C_4.
 *   The sample pitch can be corrected with this value.
 *   Value is multiple of BK_FINT20_UNIT.
 *   This attribute must be set before the data object is attached to a track.
 * BK_SAMPLE_SUSTAIN_RANGE
 *   Set range to repeat when sample is played
 *   Will be copied to track when setting sample
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 */
extern BKInt BKDataSetAttr (BKData * data, BKEnum attr, BKInt value) BK_DEPRECATED_FUNC ("Use 'BKSetAttr' instead");

/**
 * Get attribute
 *
 * BK_NUM_SAMPLE
 * BK_NUM_CHANNELS
 * BK_SAMPLE_PITCH
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 */
extern BKInt BKDataGetAttr (BKData const * data, BKEnum attr, BKInt * outValue) BK_DEPRECATED_FUNC ("Use 'BKGetAttr' instead");

/**
 * Set frames
 */
extern BKInt BKDataSetFrames (BKData * data, BKFrame const * frames, BKUInt numFrames, BKUInt numChannels, BKInt copy);

/**
 * Set frame data and convert to internal format
 * `params` can be a combination of endian and bit flags e.g:
 *   BK_LITTLE_ENDIAN | BK_16_BIT_SIGNED
 * Endianness only affects data with more than 8 bits per frame
 */
extern BKInt BKDataSetData (BKData * data, void const * frameData, BKUInt dataSize, BKUInt numChannels, BKEnum params);

/**
 * Load frames from raw audio file
 * Calls `BKDataSetData` with the data of the file
 */
extern BKInt BKDataLoadRaw (BKData * data, FILE * file, BKUInt numChannels, BKEnum params);

/**
 * Load frames from WAVE audio file
 * Only 16 and 8 bit PCM format is supported
 * File `file` is not closed
 */
extern BKInt BKDataLoadWAVE (BKData * data, FILE * file);

/**
 * Normalize frames to maximum possible values
 * If BKData was initialized without copying frames, a copy is made
 */
extern BKInt BKDataNormalize (BKData * data);

/**
 * 
 */
extern BKInt BKDataConvert (BKData * data, BKDataConvertInfo * info);

#endif /* ! _BK_DATA_H_ */
