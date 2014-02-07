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

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "BKData_internal.h"
#include "BKTone.h"

enum
{
	BK_DATA_FLAG_STATE_LIST_LOCK = 1 << 16,
	BK_DATA_FLAG_COPY            = 1 << 17,
	BK_DATA_FLAG_COPY_MASK       = BK_DATA_FLAG_COPY,
};

/**
 * Add state to data state list
 */
static void BKDataStateAddToData (BKDataState * state, BKData * data)
{
	BKDataState * lastState;

	if (state -> data == NULL && data && (data -> flags & BK_DATA_FLAG_STATE_LIST_LOCK) == 0) {
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

	if (data != NULL && (data -> flags & BK_DATA_FLAG_STATE_LIST_LOCK) == 0) {
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

	data -> flags |= BK_DATA_FLAG_STATE_LIST_LOCK;

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

	data -> flags &= ~BK_DATA_FLAG_STATE_LIST_LOCK;
}

static BKInt BKDataPromoteToCopy (BKData * data)
{
	size_t    size;
	BKFrame * frames;

	if ((data -> flags & BK_DATA_FLAG_COPY) == 0) {
		size   = data -> numFrames * data -> numChannels * sizeof (BKFrame);
		frames = malloc (size);

		if (frames == NULL)
			return BK_ALLOCATION_ERROR;

		memcpy (frames, data -> frames, size);

		data -> frames = frames;
		data -> flags |= BK_DATA_FLAG_COPY;
	}

	return 0;
}

BKInt BKDataInit (BKData * data)
{
	memset (data, 0, sizeof (BKData));

	return 0;
}

void BKDataDispose (BKData * data)
{
	if (data == NULL)
		return;

	BKDataDetach (data);

	if (data -> frames)
		free (data -> frames);

	memset (data, 0, sizeof (BKData));
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

	copy -> flags    &= BK_DATA_FLAG_COPY_MASK;
	copy -> stateList = NULL;
	copy -> frames    = NULL;

	if (original -> frames)
		res = BKDataSetFrames (copy, original -> frames, original -> numFrames, original -> numChannels, 1);

	if (res < 0)
		return res;

	return 0;
}

BKInt BKDataSetAttr (BKData * data, BKEnum attr, BKInt value)
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

BKInt BKDataGetAttr (BKData const * data, BKEnum attr, BKInt * outValue)
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

BKInt BKDataSetPtr (BKData * data, BKEnum attr, void * ptr)
{
	switch (attr) {
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	return 0;
}

BKInt BKDataGetPtr (BKData const * data, BKEnum attr, void * outPtr)
{
	void ** ptrRef = outPtr;

	switch (attr) {
		case BK_SAMPLE: {
			* ptrRef = data -> frames;
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
		if (data -> flags & BK_DATA_FLAG_COPY) {
			newFrames = realloc (data -> frames, size);
		}
		else {
			newFrames = malloc (size);
		}

		if (newFrames == NULL)
			return -1;

		data -> flags |= BK_DATA_FLAG_COPY;

		memcpy (newFrames, frames, size);
	}
	else {
		if (data -> flags & BK_DATA_FLAG_COPY) {
			if (data -> frames)
				free (data -> frames);
		}

		newFrames = (BKFrame *) frames;
	}

	if (newFrames) {
		data -> frames      = newFrames;
		data -> numFrames   = numFrames;
		data -> numChannels = numChannels;
	}
	else {
		return -1;
	}

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

static BKInt BKDataParamFromNumBits (BKEnum * outParam, BKUInt numBits, BKInt isSigned)
{
	BKEnum param = 0;

	switch (numBits) {
		case 1: {
			param = BK_1_BIT_UNSIGNED;
			break;
		}
		case 2: {
			param = BK_2_BIT_UNSIGNED;
			break;
		}
		case 4: {
			param = BK_4_BIT_UNSIGNED;
			break;
		}
		case 8: {
			param = isSigned ? BK_8_BIT_SIGNED : BK_8_BIT_UNSIGNED;
			break;
		}
		case 16: {
			param = BK_16_BIT_SIGNED;
			break;
		}
		default: {
			return BK_INVALID_ATTRIBUTE;
			break;
		}
	}

	* outParam = param;

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
	unsigned char const * charData = data;
	unsigned char c;

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
					outFrames [0] = (int16_t) (* (signed char *) charData) * BK_FRAME_MAX / 127;
				} else {
					outFrames [0] = (int16_t) (* (unsigned char *) charData) * BK_FRAME_MAX / 255;
				}

				charData += 1;
				outFrames += 1;
				break;
			}
			case 16: {
				if (reverseEndian) {
					outFrames [0] = (charData [0] << 8) | (charData [1] >> 8);
				} else {
					outFrames [0] =  (* (int16_t *) charData);
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

BKInt BKDataInitWithFrames (BKData * data, BKFrame const * phases, BKUInt numFrames, BKUInt numChannels, BKInt copy)
{
	if (BKDataInit (data) == 0) {
		return BKDataSetFrames (data, phases, numFrames, numChannels, copy);
	}

	return -1;
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

	if (data -> flags & BK_DATA_FLAG_COPY) {
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

	return 0;
}

BKInt BKDataInitWithData (BKData * data, void const * frameData, BKUInt dataSize, BKUInt numChannels, BKEnum params)
{
	if (BKDataInit (data) == 0) {
		return BKDataSetData (data, frameData, dataSize, numChannels, params);
	}

	return -1;
}

BKInt BKDataInitAndLoadRawAudio (BKData * data, char const * path, BKUInt numBits, BKUInt numChannels, BKEnum endian)
{
	int    file;
	off_t  size;
	void * frames;
	BKEnum params;
	BKInt  ret = 0;

	if (BKDataInit (data) < 0)
		return BK_INVALID_RETURN_VALUE;

	file = open (path, O_RDONLY);

	if (file < 0)
		return BK_FILE_ERROR;

	size = lseek (file, 0, SEEK_END);

	if (size < 0) {
		close (file);
		return BK_FILE_ERROR;
	}

	frames = malloc (size);

	if (frames == NULL) {
		close (file);
		return BK_ALLOCATION_ERROR;
	}

	lseek (file, 0, SEEK_SET);
	read (file, frames, size);
	close (file);

	if (BKDataParamFromNumBits (& params, numBits, 0) < 0) {
		free (frames);
		return BK_INVALID_ATTRIBUTE;
	}

	params |= endian;

	ret = BKDataSetData (data, frames, size, numChannels, params);

	free (frames);

	return ret;
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
