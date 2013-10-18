/**
 * Copyright (c) 2012-2013 Simon Schoenenberger
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
	BK_DATA_FLAG_STATE_LIST_LOCK = 1 << 0,
	BK_DATA_FLAG_COPY            = 1 << 1,
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
		size   = data -> numSamples * data -> numChannels * sizeof (BKFrame);
		frames = malloc (size);

		if (frames == NULL)
			return BK_ALLOCATION_ERROR;

		memcpy (frames, data -> samples, size);

		data -> samples = frames;
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

	BKDataResetStates (data, BK_DATA_STATE_EVENT_DISPOSE);

	if (data -> samples)
		free (data -> samples);

	memset (data, 0, sizeof (BKData));
}

BKInt BKDataInitCopy (BKData * copy, BKData * original)
{
	BKInt res = 0;

	memset (copy, 0, sizeof (BKData));

	copy -> flags = (original -> flags & BK_DATA_FLAG_COPY_MASK);

	if (original -> samples)
		res = BKDataSetFrames (copy, original -> samples, original -> numSamples, original -> numChannels, 1);

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

BKInt BKDataGetAttr (BKData * data, BKEnum attr, BKInt * outValue)
{
	BKInt value = 0;

	switch (attr) {
		case BK_NUM_SAMPLES: {
			value = data -> numSamples;
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

BKInt BKDataGetPtr (BKData * data, BKEnum attr, void * outPtr)
{
	void ** ptrRef = outPtr;

	switch (attr) {
		case BK_SAMPLE: {
			* ptrRef = data -> samples;
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
		return BK_INVALID_NUM_SAMPLES;

	numChannels = BKClamp (numChannels, 1, BK_MAX_CHANNELS);
	size        = numFrames * numChannels * sizeof (BKFrame);

	if (copy) {
		if (data -> flags & BK_DATA_FLAG_COPY) {
			newFrames = realloc (data -> samples, size);
		}
		else {
			newFrames = malloc (size);
		}

		if (newFrames == NULL)
			return -1;

		memcpy (newFrames, frames, size);
	}
	else {
		if (data -> flags & BK_DATA_FLAG_COPY) {
			if (data -> samples)
				free (data -> samples);
		}

		newFrames = (BKFrame *) frames;
	}

	if (newFrames) {
		data -> samples     = newFrames;
		data -> numSamples  = numFrames;
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

BKInt BKDataInitWithFrames (BKData * data, BKFrame const * phases, BKUInt numFrames, BKUInt numChannels, BKInt copy)
{
	if (BKDataInit (data) == 0) {
		return BKDataSetFrames (data, phases, numFrames, numChannels, copy);
	}

	return -1;
}

/**
 * Reverse 16 bit integers
 */
static void BKEndianReverse16Bit (BKFrame frames [], off_t size)
{
	BKFrame value;

	for (off_t i = 0; i < size; i ++) {
		value = frames [i];
		value = (value >> 8) | (value << 8);
		frames [i] = value;
	}
}

BKInt BKDataInitAndLoadRawAudio (BKData * data, char const * path, BKUInt numBits, BKUInt numChannels, BKEnum endian)
{
	int    file;
	off_t  size;
	BKUInt packetSize;
	BKUInt numSamples;

	if (BKDataInit (data) == 0) {
		switch (numBits) {
			case 16: {
				switch (endian) {
					case BK_BIG_ENDIAN: break;
					case BK_LITTLE_ENDIAN: break;
					default: {
						return BK_INVALID_ATTRIBUTE;
						break;
					}
				}
				break;
			}
			default: {
				return BK_INVALID_ATTRIBUTE;
				break;
			}
		}

		numChannels = BKMax (1, numChannels);
		packetSize  = numChannels * numBits;
		packetSize  = (packetSize + 7) & ~7;  // round packet size up to one byte
		packetSize  = packetSize / 8;         // set number of bytes

		file = open (path, O_RDONLY);

		if (file != -1) {
			size = lseek (file, 0, SEEK_END);   // number of bytes in file
			size = size - (size % packetSize);  // round down to full packet

			if (size != -1) {
				numSamples = (BKUInt) size * 8 / (numChannels * numBits);
				data -> samples = malloc (sizeof (BKFrame) * numSamples * numChannels);

				if (data -> samples) {
					data -> numSamples  = numSamples;
					data -> numChannels = numChannels;

					lseek (file, 0, SEEK_SET);
					read (file, data -> samples, size);

					if (BKSystemIsBigEndian () != (endian == BK_BIG_ENDIAN))
						BKEndianReverse16Bit (data -> samples, size);
				}
			}
			else {
				return BK_OTHER_ERROR;
			}

			close (file);
		}
		else {
			return BK_OTHER_ERROR;
		}

		return 0;
	}

	return -1;
}

BKInt BKDataNormalize (BKData * data)
{
	BKInt res = 0;
	BKInt value, maxValue = 0;
	BKInt factor;

	res = BKDataPromoteToCopy (data);

	if (res != 0)
		return res;

	for (BKInt i = 0; i < data -> numSamples * data -> numChannels; i ++) {
		value = BKAbs (data -> samples [i]);

		if (value > maxValue)
			maxValue = value;
	}

	if (maxValue) {
		factor = (BK_MAX_VOLUME << 16) / maxValue;

		for (BKInt i = 0; i < data -> numSamples * data -> numChannels; i ++)
			data -> samples [i] = (data -> samples [i] * factor) >> 16;
	}

	return 0;
}

BKInt BKDataStateSetData (BKDataState * state, BKData * data)
{
	BKDataStateRemoveFromData (state);
	BKDataStateAddToData (state, data);

	return 0;
}
