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

/**
 * @file
 *
 * A data object containing audio frames.
 */

#ifndef _BK_DATA_H_
#define _BK_DATA_H_

#include "BKObject.h"
#include <stdio.h>

typedef struct BKData BKData;
typedef struct BKDataState BKDataState;

typedef struct BKDataInfo BKDataInfo;
typedef struct BKDataConvertInfo BKDataConvertInfo;
typedef struct BKDataExportInfo BKDataExportInfo;

typedef BKInt (*BKDataStateCallback)(BKEnum event, void* userInfo);

/**
 * Endian types.
 */
enum {
	BK_BIG_ENDIAN = 1 << 16,	///< Big endian flag.
	BK_LITTLE_ENDIAN = 2 << 16, ///< Little endian flag.
	BK_ENDIAN_MASK = 3 << 16,	///< Mask matching endians.
};

/**
 * Sizes and sign.
 */
enum {
	BK_1_BIT_UNSIGNED = 1,	///< 1 bit unsigned.
	BK_2_BIT_UNSIGNED = 2,	///< 2 bit unsigned.
	BK_4_BIT_UNSIGNED = 3,	///< 4 bit unsigned.
	BK_8_BIT_SIGNED = 4,	///< 8 bit signed.
	BK_8_BIT_UNSIGNED = 5,	///< 8 bit unsigned.
	BK_16_BIT_SIGNED = 6,	///< 16 bit signed.
	BK_DATA_BITS_MASK = 15, ///< Mask matching the bit type.
};

/**
 * Data object events.
 */
enum {
	BK_DATA_STATE_EVENT_RESET,	 ///< Event after changing object attributes.
	BK_DATA_STATE_EVENT_DISPOSE, ///< Event fired before disposing data object.
};

/**
 * The data struct.
 */
struct BKData {
	BKObject object;		///< The parent object.
	BKEnum numBits;			///< Number of bits.
	BKInt sampleRate;		///< Sample rate.
	BKUInt numFrames;		///< Number of frames per channel.
	BKUInt numChannels;		///< Number of channels.
	BKFInt20 samplePitch;	///< Sample pitch.
	BKUInt sustainOffset;	///< Sustain range offset,
	BKUInt sustainEnd;		///< Sustain range end.
	BKFrame* frames;		///< The frames.
	BKDataState* stateList; ///< The states.
};

/**
 * The data state.
 */
struct BKDataState {
	BKData* data;				  ///< The data object.
	BKDataStateCallback callback; ///< Called for every data event.
	void* callbackUserInfo;		  ///< The callback context.
	BKDataState* nextState;		  ///< The next state.
};

/**
 * Struct containing hints for converting data.
 */
struct BKDataConvertInfo {
	BKInt sourceSampleRate;	  ///< Sample rate of source.
	BKInt targetSampleRate;	  ///< Sample rate of target.
	BKEnum targetNumBits;	  ///< Target number of bits.
	BKInt ditherSmoothLength; ///< Used for downsampling. Number of samples to smooth out dithering.
	float ditherSlope;		  ///< Used for downsampling. Curve slope used for dithering.
	float ditherCurve;		  ///< Used for downsampling. Curve power used for dithering.
	float threshold;		  ///< Used for downsampling. Dithering threshold.
};

/**
 * Initialize data object.
 *
 * @param data The data object to initialize.
 * @return 0 on success.
 */
extern BKInt BKDataInit(BKData* data);

/**
 * Allocate data object and initialize.
 *
 * @param outData Reference to object pointer.
 * @return 0 on success.
 */
extern BKInt BKDataAlloc(BKData** outData);

/**
 * Detaches the object from all state.
 *
 * @param data The data object to detach.
 */
extern void BKDataDetach(BKData* data);

/**
 * Initialize data object and copy from other object.
 *
 * @param copy The data object to initialize.
 * @param original The data object to copy.
 * @return 0 on success.
 */
extern BKInt BKDataInitCopy(BKData* copy, BKData const* original);

/**
 * Set object attributes.
 *
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
 *
 * @param data The data object to the attribute to.
 * @param attr The attribute to set.
 * @param value The attribute value.
 * @return 0 on success.
 */
extern BKInt BKDataSetAttr(BKData* data, BKEnum attr, BKInt value) BK_DEPRECATED_FUNC("Use 'BKSetAttr' instead");

/**
 * Get object attribute.
 *
 * BK_NUM_SAMPLE
 * BK_NUM_CHANNELS
 * BK_SAMPLE_PITCH
 *
 * Errors:
 * BK_INVALID_ATTRIBUTE if attribute is unknown
 *
 * @param data The data object to get the attribute from.
 * @param attr The attribute to get.
 * @param outValue A reference to be set to the attribute value.
 * @return 0 on success.
 */
extern BKInt BKDataGetAttr(BKData const* data, BKEnum attr, BKInt* outValue) BK_DEPRECATED_FUNC("Use 'BKGetAttr' instead");

/**
 * Replace frames.
 *
 * @param data The data object to replace the frames.
 * @param frames The frames used for replacing.
 * @param numFrames Number of frames per channel.
 * @param numChannels The number of channels.
 * @param copy Set to 1 if the frames should be copied.
 * @return 0 on success.
 */
extern BKInt BKDataSetFrames(BKData* data, BKFrame const* frames, BKUInt numFrames, BKUInt numChannels, BKInt copy);

/**
 * Replace frames data and convert to internal format.
 *
 * @param data The data object to replace the frames.
 * @param frameData The frame data used for replacing.
 * @param dataSize The size of frameData in bytes.
 * @param numChannels The number of channels.
 * @params params A combination of endian and bit flags, e.g:
 *   BK_LITTLE_ENDIAN | BK_16_BIT_SIGNED
 *   Endianness only affects data with more than 8 bits per frame.
 * @return 0 on success.
 */
extern BKInt BKDataSetData(BKData* data, void const* frameData, BKUInt dataSize, BKUInt numChannels, BKEnum params);

/**
 * Load frames from raw audio file. Calls BKDataSetData with the data of the file.
 *
 * @param data The data object to load the audio data into.
 * @param file The file to read from. Will not be closed.
 * @param numChannels The number of channels.
 * @params params A combination of endian and bit flags, e.g:
 *   BK_LITTLE_ENDIAN | BK_16_BIT_SIGNED
 *   Endianness only affects data with more than 8 bits per frame.
 * @return 0 on success.
 */
extern BKInt BKDataLoadRaw(BKData* data, FILE* file, BKUInt numChannels, BKEnum params);

/**
 * Load frames from WAVE audio file. Only 16 and 8 bit PCM format is supported.
 *
 * @param data The data object to load the WAVE into.
 * @param file The file to read from. Will not be closed.
 * @return 0 on success.
 */
extern BKInt BKDataLoadWAVE(BKData* data, FILE* file);

/**
 * Normalize frames to their maximum possible value. If BKData was initialized
 * without copying frames, a copy is made.
 *
 * @param data The data object to normalize.
 * @return 0 on success.
 */
extern BKInt BKDataNormalize(BKData* data);

/**
 * TODO
 */
extern BKInt BKDataConvert(BKData* data, BKDataConvertInfo* info);

#endif /* ! _BK_DATA_H_ */
