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

#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include "BKData_internal.h"
#include "BKWaveFileReader.h"
#include "BKTone.h"

extern BKClass const BKDataClass;

enum
{
	BK_DATA_FLAG_COPY      = 1 << 16,
	BK_DATA_FLAG_COPY_MASK = BK_DATA_FLAG_COPY,
};

/**
 * Add state to data state list
 */
static void BKDataStateAddToData (BKDataState * state, BKData * data)
{
	BKDataState * lastState;

	if (state -> data == NULL && data && (data -> object.flags & BKObjectFlagLocked) == 0) {
		// search for last state in  list
		for (lastState = data -> stateList; lastState && lastState -> nextState;)
			lastState = lastState -> nextState;

		// has last state
		if (lastState) {
			lastState -> nextState = state;
		}
		// list was empty
		else {
			data -> stateList = state;
		}

		state -> data       = data;
		state -> nextState  = NULL;
	}
}

/**
 * Remove state from data state list
 */
static void BKDataStateRemoveFromData (BKDataState * state)
{
	BKData      * data = state -> data;
	BKDataState * searchState, * prevState = NULL;

	if (data != NULL && (data -> object.flags & BKObjectFlagLocked) == 0) {
		// search for state and previous state
		for (searchState = data -> stateList; searchState; searchState = searchState -> nextState) {
			if (searchState == state)
				break;

			prevState = searchState;
		}

		// found in list
		if (prevState) {
			prevState -> nextState = state -> nextState;
		}
		// is first item
		else if (data -> stateList == state) {
			data -> stateList = data -> stateList -> nextState;
		}

		state -> data       = NULL;
		state -> nextState  = NULL;
	}
}

/**
 * Reset states which has set this data
 */
static void BKDataResetStates (BKData * data, BKEnum event)
{
	BKInt res;
	BKInt dispose;
	BKDataState * state;
	BKDataState * nextState, * prevState = NULL;

	data -> object.flags |= BKObjectFlagLocked;

	for (state = data -> stateList; state; state = nextState) {
		dispose = 0;
		nextState = state -> nextState;

		if (state -> callback) {
			res = state -> callback (event, state -> callbackUserInfo);

			// remove from list if failed
			if (res < 0)
				dispose = 1;
		}

		if (event == BK_DATA_STATE_EVENT_DISPOSE)
			dispose = 1;

		if (dispose) {
			if (prevState)
				prevState -> nextState = nextState;

			if (data -> stateList == state)
				data -> stateList = state -> nextState;

			state -> nextState = NULL;
			state -> data      = NULL;
			state = NULL;
		}

		prevState = state;
	}

	data -> object.flags &= ~BKObjectFlagLocked;
}

static BKInt BKDataPromoteToCopy (BKData * data)
{
	BKSize    size;
	BKFrame * frames;

	if ((data -> object.flags & BK_DATA_FLAG_COPY) == 0) {
		size   = data -> numFrames * data -> numChannels * sizeof (BKFrame);
		frames = malloc (size);

		if (frames == NULL)
			return BK_ALLOCATION_ERROR;

		memcpy (frames, data -> frames, size);

		data -> frames = frames;
		data -> object.flags |= BK_DATA_FLAG_COPY;
	}

	return 0;
}

BKInt BKDataInit (BKData * data)
{
	return BKObjectInit (data, & BKDataClass, sizeof (BKData));
}

BKInt BKDataAlloc (BKData ** outData)
{
	return BKObjectAlloc ((void *) outData, & BKDataClass, 0);
}

static void BKDataDisposeObject (BKData * data)
{
	BKDataDetach (data);

	if (data -> frames && (data -> object.flags & BK_DATA_FLAG_COPY)) {
		free (data -> frames);
	}
}

void BKDataDetach (BKData * data)
{
	if (data == NULL)
		return;

	BKDataResetStates (data, BK_DATA_STATE_EVENT_DISPOSE);
}

BKInt BKDataInitCopy (BKData * copy, BKData const * original)
{
	BKInt res = 0;

	memcpy (copy, original, sizeof (BKData));

	copy -> object.flags &= BK_DATA_FLAG_COPY_MASK;
	copy -> stateList = NULL;
	copy -> frames    = NULL;

	if (original -> frames)
		res = BKDataSetFrames (copy, original -> frames, original -> numFrames, original -> numChannels, 1);

	if (res < 0)
		return res;

	return 0;
}

static BKInt BKDataSetAttrInt (BKData * data, BKEnum attr, BKInt value)
{
	switch (attr) {
		case BK_SAMPLE_PITCH: {
			data -> samplePitch = BKClamp (value, BK_MIN_SAMPLE_TONE << BK_FINT20_SHIFT, BK_MAX_SAMPLE_TONE << BK_FINT20_SHIFT);
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

BKInt BKDataSetAttr (BKData * data, BKEnum attr, BKInt value)
{
	return BKDataSetAttrInt (data, attr, value);
}

static BKInt BKDataGetAttrInt (BKData const * data, BKEnum attr, BKInt * outValue)
{
	BKInt value = 0;

	switch (attr) {
		case BK_NUM_FRAMES: {
			value = data -> numFrames;
			break;
		}
		case BK_NUM_CHANNELS: {
			value = data -> numChannels;
			break;
		}
		case BK_SAMPLE_PITCH: {
			value = data -> samplePitch;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	* outValue = value;

	return 0;
}

BKInt BKDataGetAttr (BKData const * data, BKEnum attr, BKInt * outValue)
{
	return BKDataGetAttrInt (data, attr, outValue);
}

static BKInt BKDataSetPtr (BKData * data, BKEnum attr, void * ptr)
{
	BKInt * values;

	switch (attr) {
		case BK_SAMPLE_SUSTAIN_RANGE: {
			BKInt tmp;
			BKInt range [2];
			BKInt sampleLength = data -> numFrames;
			BKInt offset, end;

			values = ptr;

			if (values == NULL || values [0] == values [1]) {
				range [0] = 0;
				range [1] = 0;
				values = range;
			}

			offset = values [0];
			end    = values [1];

			if (offset < 0) {
				offset += sampleLength + 1;
			}

			if (end < 0) {
				end += sampleLength + 1;
			}

			offset = BKClamp (offset, 0, sampleLength);
			end    = BKClamp (end, 0, sampleLength);

			if (end < offset) {
				tmp    = offset;
				offset = end;
				end    = tmp;
			}

			data -> sustainOffset = offset;
			data -> sustainEnd    = end;

			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

static BKInt BKDataGetPtr (BKData const * data, BKEnum attr, void * outPtr)
{
	BKInt * values = outPtr;

	switch (attr) {
		case BK_SAMPLE_SUSTAIN_RANGE: {
			values [0] = data -> sustainOffset;
			values [1] = data -> sustainEnd;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

BKInt BKDataSetFrames (BKData * data, BKFrame const * frames, BKUInt numFrames, BKUInt numChannels, BKInt copy)
{
	BKUInt    size;
	BKFrame * newFrames;

	// need at least 2 phases
	if (numFrames < 2)
		return BK_INVALID_NUM_FRAMES;

	if (numChannels < 1 || numChannels > BK_MAX_CHANNELS)
		return BK_INVALID_NUM_CHANNELS;

	size = numFrames * numChannels * sizeof (BKFrame);

	if (copy) {
		if (data -> object.flags & BK_DATA_FLAG_COPY) {
			newFrames = realloc (data -> frames, size);
		}
		else {
			newFrames = malloc (size);
		}

		if (newFrames == NULL)
			return -1;

		data -> object.flags |= BK_DATA_FLAG_COPY;

		memcpy (newFrames, frames, size);
	}
	else {
		if (data -> object.flags & BK_DATA_FLAG_COPY) {
			if (data -> frames)
				free (data -> frames);
		}

		newFrames = (BKFrame *) frames;
	}

	if (newFrames == NULL) {
		return -1;
	}

	data -> frames      = newFrames;
	data -> numFrames   = numFrames;
	data -> numChannels = numChannels;
	data -> numBits     = 16;

	BKDataResetStates (data, BK_DATA_STATE_EVENT_RESET);

	return 0;
}

/**
 * Returns 1 if system is big endian otherwise 0
 */
static BKInt BKSystemIsBigEndian (void)
{
	union { BKUInt i; char c [4]; } sentinel;

	sentinel.i = 0x01020304;

	return sentinel.c[0] == 0x01;
}

static BKInt BKDataNumBitsFromParam (BKEnum param, BKUInt * outNumBits, BKInt * outIsSigned)
{
	BKInt numBits  = 0;
	BKInt isSigned = 0;

	switch (param) {
		case BK_1_BIT_UNSIGNED: {
			numBits = 1;
			break;
		}
		case BK_2_BIT_UNSIGNED: {
			numBits = 2;
			break;
		}
		case BK_4_BIT_UNSIGNED: {
			numBits = 4;
			break;
		}
		case BK_8_BIT_SIGNED: {
			numBits = 8;
			isSigned = 1;
			break;
		}
		case BK_8_BIT_UNSIGNED: {
			numBits  = 8;
			break;
		}
		case BK_16_BIT_SIGNED: {
			numBits  = 16;
			isSigned = 1;
			break;
		}
		default: {
			return BK_INVALID_NUM_BITS;
			break;
		}
	}

	* outNumBits  = numBits;
	* outIsSigned = isSigned;

	return 0;
}

static BKInt BKDataCalculateNumFramesFromNumBits (BKUInt dataSize, BKUInt numBits, BKUInt numChannels)
{
	BKInt numFrames = 0;
	BKInt packetSize;
	BKInt dataSizeBits = dataSize * 8;

	if (numBits <= 8) {
		packetSize = (numBits * numChannels);
	}
	else {
		// round up to multiple of 4
		packetSize = (numBits + 3) / 4 * 4 * numChannels;
	}

	numFrames = dataSizeBits / packetSize * numChannels;

	return numFrames;
}

static BKInt BKDataConvertFromBits (BKFrame * outFrames, void const * data, BKUInt dataSize, BKUInt numBits, BKInt isSigned, BKInt reverseEndian, BKUInt numChannels)
{
	BKSize mask;
	unsigned char const * charData = data;
	unsigned char c;

	switch (numBits) {
		case  1: mask = 1; break;
		case  2: mask = 1; break;
		case  4: mask = 1; break;
		case  8: mask = 1; break;
		default:
		case 16: mask = 2; break;
	}

	dataSize -= (dataSize % (mask * numChannels));

	for (charData = data; (void *) charData < data + dataSize;) {
		switch (numBits) {
			case 1: {
				c = charData [0];
				outFrames [0] = ((c & (1 << 7)) >> 7) * BK_FRAME_MAX;
				outFrames [1] = ((c & (1 << 6)) >> 6) * BK_FRAME_MAX;
				outFrames [2] = ((c & (1 << 5)) >> 5) * BK_FRAME_MAX;
				outFrames [3] = ((c & (1 << 4)) >> 4) * BK_FRAME_MAX;
				outFrames [4] = ((c & (1 << 3)) >> 3) * BK_FRAME_MAX;
				outFrames [5] = ((c & (1 << 2)) >> 2) * BK_FRAME_MAX;
				outFrames [6] = ((c & (1 << 1)) >> 1) * BK_FRAME_MAX;
				outFrames [7] = ((c & (1 << 0)) >> 0) * BK_FRAME_MAX;
				charData += 1;
				outFrames += 8;
				break;
			}
			case 2: {
				c = charData [0];
				outFrames [0] = ((c & (3 << 6)) >> 6) * BK_FRAME_MAX / 3;
				outFrames [1] = ((c & (3 << 4)) >> 4) * BK_FRAME_MAX / 3;
				outFrames [2] = ((c & (3 << 2)) >> 2) * BK_FRAME_MAX / 3;
				outFrames [3] = ((c & (3 << 0)) >> 0) * BK_FRAME_MAX / 3;
				charData += 1;
				outFrames += 4;
				break;
			}
			case 4: {
				c = charData [0];
				outFrames [0] = ((c & (15 << 4)) >> 4) * BK_FRAME_MAX / 15;
				outFrames [1] = ((c & (15 << 0)) >> 0) * BK_FRAME_MAX / 15;
				charData += 1;
				outFrames += 2;
				break;
			}
			case 8: {
				if (isSigned) {
					outFrames [0] = (BKFrame) (* (signed char *) charData) * BK_FRAME_MAX / 127;
				} else {
					outFrames [0] = (BKFrame) (* (unsigned char *) charData) * BK_FRAME_MAX / 255;
				}

				charData += 1;
				outFrames += 1;
				break;
			}
			case 16: {
				if (reverseEndian) {
					outFrames [0] = (charData [0] << 8) | (charData [1] >> 8);
				} else {
					outFrames [0] =  (* (BKFrame *) charData);
				}

				charData += 2;
				outFrames += 1;
				break;
			}
			default: {
				return BK_INVALID_NUM_BITS;
			}
		}
	}

	return 0;
}

BKInt BKDataSetData (BKData * data, void const * frameData, BKUInt dataSize, BKUInt numChannels, BKEnum params)
{
	BKUInt    endian = (params & BK_ENDIAN_MASK);
	BKUInt    bits   = (params & BK_DATA_BITS_MASK);
	BKUInt    numBits;
	BKInt     isSigned, reverseEndian = 0;
	BKInt     numFrames;
	BKFrame * frames;

	if (numChannels < 1 || numChannels > BK_MAX_CHANNELS)
		return BK_INVALID_NUM_CHANNELS;

	if (endian)
		reverseEndian = BKSystemIsBigEndian () != (endian == BK_BIG_ENDIAN);

	if (BKDataNumBitsFromParam (bits, & numBits, & isSigned) < 0)
		return BK_INVALID_NUM_BITS;

	numFrames = BKDataCalculateNumFramesFromNumBits (dataSize, numBits, numChannels);

	if (numFrames < 0)
		return BK_INVALID_NUM_BITS;

	if (data -> object.flags & BK_DATA_FLAG_COPY) {
		frames = realloc (data -> frames, numFrames * sizeof (BKFrame));
	}
	else {
		frames = malloc (numFrames * sizeof (BKFrame));
	}

	if (frames == NULL)
		return BK_ALLOCATION_ERROR;

	if (BKDataConvertFromBits (frames, frameData, dataSize, numBits, isSigned, reverseEndian, numChannels) < 0)
		return -1;

	data -> frames      = frames;
	data -> numFrames   = numFrames / numChannels;
	data -> numChannels = numChannels;
	data -> numBits     = numBits;

	return 0;
}

BKInt BKDataLoadRaw (BKData * data, FILE * file, BKUInt numChannels, BKEnum params)
{
	BKSize offset, size;
	void * frames;
	BKInt  ret = 0;

	offset = ftell (file);

	if (offset < 0) {
		return BK_FILE_ERROR;
	}

	fseek (file, 0, SEEK_END);
	size = ftell (file);

	if (size < 0) {
		return BK_FILE_ERROR;
	}

	size -= offset;
	frames = malloc (size);

	if (frames == NULL) {
		return BK_ALLOCATION_ERROR;
	}

	fseek (file, offset, SEEK_SET);
	fread (frames, sizeof (char), size, file);

	ret = BKDataSetData (data, frames, (BKUInt) size, numChannels, params);

	free (frames);

	return ret;
}

BKInt BKDataLoadWAVE (BKData * data, FILE * file)
{
	BKWaveFileReader reader;
	BKInt numChannels, sampleRate, numFrames;
	BKFrame * frames;
	BKSize size;

	if (BKWaveFileReaderInit (& reader, file) < 0) {
		return BK_INVALID_RETURN_VALUE;
	}

	if (BKWaveFileReaderReadHeader (& reader, & numChannels, & sampleRate, & numFrames) < 0) {
		return BK_INVALID_RETURN_VALUE;
	}

	size   = numFrames * numChannels * sizeof (BKFrame);
	frames = malloc (size);

	if (frames == NULL) {
		BKDispose (& reader);
		return BK_ALLOCATION_ERROR;
	}

	if (BKWaveFileReaderReadFrames (& reader, frames) < 0) {
		free (frames);
		return BK_INVALID_RETURN_VALUE;
	}

	data -> object.flags |= BK_DATA_FLAG_COPY;
	data -> numBits       = 16;
	data -> sampleRate    = sampleRate;
	data -> numFrames     = numFrames;
	data -> numChannels   = numChannels;
	data -> frames        = frames;

	BKDispose (& reader);

	return 0;
}

BKInt BKDataNormalize (BKData * data)
{
	BKInt res = 0;
	BKInt value, maxValue = 0;
	BKInt factor;

	res = BKDataPromoteToCopy (data);

	if (res != 0)
		return res;

	for (BKInt i = 0; i < data -> numFrames * data -> numChannels; i ++) {
		value = BKAbs (data -> frames [i]);

		if (value > maxValue)
			maxValue = value;
	}

	if (maxValue) {
		factor = (BK_MAX_VOLUME << 16) / maxValue;

		for (BKInt i = 0; i < data -> numFrames * data -> numChannels; i ++)
			data -> frames [i] = (data -> frames [i] * factor) >> 16;
	}

	return 0;
}

BKInt BKDataStateSetData (BKDataState * state, BKData * data)
{
	BKDataStateRemoveFromData (state);
	BKDataStateAddToData (state, data);

	return 0;
}

static void BKDataReduceBits (BKFrame * outFrames, BKFrame * frames, BKSize length, BKDataConvertInfo * info)
{
	BKInt maxValue;

	BKInt const maximize = 1;
	BKInt const shiftUp  = 1;

	BKInt bits               = info -> targetNumBits;
	BKInt ditherSmoothLength = info -> ditherSmoothLength;
	float ditherSlope        = info -> ditherSlope;
	float ditherCurve        = info -> ditherCurve;

	maxValue = (1 << 15) - 1;

	BKInt frame;
	BKInt frame32;
	BKInt dither, deltaDither = 0;
	BKInt threshold = info -> threshold * maxValue;
	float sum = 0.0;
	float lastFrame = 0.0;
	int downsample = 15 - bits + 1;
	float ditherFactor;

	if (bits <= 8)
		deltaDither = (1 << (downsample)) - 1;

	for (int i = 0; i < length; i ++) {
		frame = frames [i];

		sum -= lastFrame;
		sum += frame;
		lastFrame = frame;

		frame32 = (BKInt) frame;

		if (BKAbs (frame) >= threshold) {
			if (deltaDither) {
				dither = rand ();
				dither = (dither & 1) ? -deltaDither : deltaDither;

				// smooth dither
				ditherFactor = pow (BKAbs ((sum / ditherSmoothLength)) / maxValue, ditherCurve);
				ditherFactor = (1.0 - ditherSlope) + (ditherFactor * ditherSlope);
				dither *= ditherFactor;

				frame32 += dither;
			}
		}
		else {
			frame32 = 0;
		}

		int tmpDownsample = downsample;

		frame32 = BKClamp (frame32, -maxValue, maxValue);

		if (shiftUp && bits <= 8) {
			frame32 = (frame32 >> 1) - (1 << 15) / 2;
			tmpDownsample -= 1;
			frame32 = BKClamp (frame32, -maxValue, maxValue);
			frame32 >>= tmpDownsample;
			frame32 = -frame32;
		}
		else {
			frame32 >>= tmpDownsample;
			frame32 = -frame32;
		}

		if (maximize)
			frame32 = frame32 * maxValue / (1 << (15 - tmpDownsample));

		outFrames [i] = frame32;
	}
}

BKInt BKDataConvert (BKData * data, BKDataConvertInfo * info)
{
	BKFrame * convertedFrames;
	BKSize length;
	BKDataConvertInfo validatedInfo;

	length = data -> numFrames * data -> numChannels;

	validatedInfo = (* info);

	if (validatedInfo.ditherSmoothLength == 0)
		validatedInfo.ditherSmoothLength = 64;

	if (validatedInfo.ditherSlope == 0)
		validatedInfo.ditherSlope = 1.0;

	if (validatedInfo.ditherCurve == 0)
		validatedInfo.ditherCurve = 1.0;

	if (validatedInfo.threshold == 0)
		validatedInfo.threshold = 0.02;

	if (validatedInfo.targetNumBits > 15)
		validatedInfo.targetNumBits = 15;

	if ((data -> object.flags & BK_DATA_FLAG_COPY) == 0) {
		convertedFrames = malloc (length * sizeof (BKFrame));

		if (convertedFrames == NULL)
			return -1;
	}
	else {
		convertedFrames = data -> frames;
	}

	BKDataReduceBits (convertedFrames, data -> frames, length, & validatedInfo);

	data -> frames = convertedFrames;

	BKDataResetStates (data, BK_DATA_STATE_EVENT_RESET);

	return 0;
}

BKClass const BKDataClass =
{
	.instanceSize = sizeof (BKData),
	.setAttr      = (void *) BKDataSetAttrInt,
	.getAttr      = (void *) BKDataGetAttrInt,
	.setPtr       = (void *) BKDataSetPtr,
	.getPtr       = (void *) BKDataGetPtr,
	.dispose      = (void *) BKDataDisposeObject,
};
