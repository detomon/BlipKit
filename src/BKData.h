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

#ifndef _BK_DATA_H_
#define _BK_DATA_H_

#include "BKBase.h"

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

struct BKDataInfo
{
	BKEnum numBits;
};

struct BKData
{
	BKUInt        flags;
	BKDataInfo    info;
	BKUInt        numFrames;
	BKUInt        numChannels;
	BKFInt20      samplePitch;
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

struct BKDataExportInfo
{

};

/**
 * Initialize Data
 */
extern BKInt BKDataInit (BKData * data);

/**
 * Dispose data and detach from all tracks
 */
extern void BKDataDispose (BKData * data);

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
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 */
extern BKInt BKDataSetAttr (BKData * data, BKEnum attr, BKInt value);

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
extern BKInt BKDataGetAttr (BKData const * data, BKEnum attr, BKInt * outValue);

/**
 * No attributes defined
 *
 * Errors:
 * Always returns BK_INVALID_ATTRIBUTE
 */
extern BKInt BKDataSetPtr (BKData * data, BKEnum attr, void * ptr);

/**
 * Get attribute
 *
 * BK_NUM_SAMPLE
 * BK_NUM_CHANNELS
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 */
extern BKInt BKDataGetPtr (BKData const * data, BKEnum attr, void * outPtr);

/**
 * Set frames
 */
extern BKInt BKDataSetFrames (BKData * data, BKFrame const * frames, BKUInt numFrames, BKUInt numChannels, BKInt copy);

/**
 * Data from frames
 * `numFrames` must be at least 2
 * `numChannel` must be between 1 and BK_MAX_CHANNELS
 */
extern BKInt BKDataInitWithFrames (BKData * data, BKFrame const * frames, BKUInt numFrames, BKUInt numChannels, BKInt copy);

/**
 * Set frame data and convert to internal format
 * `params` can be a combination of endian and bit flags e.g:
 *   BK_LITTLE_ENDIAN | BK_16_BIT_SIGNED
 * Endianness only affects data with more than 8 bits per frame
 */
extern BKInt BKDataSetData (BKData * data, void const * frameData, BKUInt dataSize, BKUInt numChannels, BKEnum params);

/**
 * Initialize data object with frame data and convert to internal format
 * `params` can be a combination of endian and bit flags e.g:
 *   BK_LITTLE_ENDIAN | BK_16_BIT_SIGNED
 * Endianness only affects data with more than 8 bits per frame
 */
extern BKInt BKDataInitWithData (BKData * data, void const * frameData, BKUInt dataSize, BKUInt numChannels, BKEnum params);

/**
 * Load frames from raw audio file
 * Calls `BKDataSetData` with the data of the file
 */
extern BKInt BKDataInitAndLoadRawAudio (BKData * data, char const * path, BKEnum bits, BKUInt numChannels, BKEnum endian);

/**
 * Normalize frames to maximum possible values
 * If BKData was initialized without copying frames, a copy is made
 */
extern BKInt BKDataNormalize (BKData * data);

/**
 * 
 */
extern BKInt BKDataConvert (BKData * data, BKDataConvertInfo * info);

/**
 * 
 */
extern BKInt BKDataExport (BKData * data, BKDataExportInfo * info);

#endif /* ! _BK_DATA_H_ */
