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
#include <unistd.h>
#include "BKSDLTrack.h"

static BKInt parseSequence (BKSDLContext * ctx, BKBlipCommand * item, BKInt * sequence, BKInt * outRepeatBegin, BKInt * outRepeatLength, BKInt multiplier)
{
	BKInt length = (BKInt) item -> argCount - 2;
	BKInt repeatBegin = 0, repeatLength = 0;

	repeatBegin  = atoi (item -> args [0].arg);
	repeatLength = atoi (item -> args [1].arg);

	if (repeatBegin > length)
		repeatBegin = length;

	if (repeatBegin + repeatLength > length)
		repeatLength = length - repeatBegin;

	* outRepeatBegin  = repeatBegin;
	* outRepeatLength = repeatLength;

	for (BKInt i = 0; i < length; i ++)
		sequence [i] = atoi (item -> args [i + 2].arg) * multiplier;

	return length;
}

static BKInstrument * parseInstrument (BKSDLContext * ctx, BKBlipReader * parser)
{
	BKInstrument * instrument;
	BKBlipCommand   item;
	BKInt sequence [256];
	BKInt sequenceLength, repeatBegin, repeatLength;

	instrument = malloc (sizeof (BKInstrument));

	if (instrument == NULL)
		return NULL;

	BKInstrumentInit (instrument);

	while (BKBlipReaderNextCommand (parser, & item)) {
		if (strcmp (item.name, "i") == 0 && strcmp (item.args [0].arg, "end") == 0) {
			break;
		}
		else if (strcmp (item.name, "v") == 0) {
			sequenceLength = parseSequence (ctx, & item, sequence, & repeatBegin, & repeatLength, (BK_MAX_VOLUME / 255));
			BKInstrumentSetSequence (instrument, BK_SEQUENCE_VOLUME, sequence, sequenceLength, repeatBegin, repeatLength);
		}
		else if (strcmp (item.name, "a") == 0) {
			sequenceLength = parseSequence (ctx, & item, sequence, & repeatBegin, & repeatLength, (BK_FINT20_UNIT / 100));
			BKInstrumentSetSequence (instrument, BK_SEQUENCE_ARPEGGIO, sequence, sequenceLength, repeatBegin, repeatLength);
		}
		else if (strcmp (item.name, "p") == 0) {
			sequenceLength = parseSequence (ctx, & item, sequence, & repeatBegin, & repeatLength, (BK_MAX_VOLUME / 255));
			BKInstrumentSetSequence (instrument, BK_SEQUENCE_PANNING, sequence, sequenceLength, repeatBegin, repeatLength);
		}
		else if (strcmp (item.name, "dc") == 0) {
			sequenceLength = parseSequence (ctx, & item, sequence, & repeatBegin, & repeatLength, 1);
			BKInstrumentSetSequence (instrument, BK_SEQUENCE_DUTY_CYCLE, sequence, sequenceLength, repeatBegin, repeatLength);
		}
	}

	return instrument;
}

static BKInt parseWaveform (BKSDLContext * ctx, BKBlipReader * parser, BKData * waveform)
{
	BKBlipCommand item;
	BKFrame      sequence [256];
	BKInt        length = 0;

	while (BKBlipReaderNextCommand (parser, & item)) {
		if (strcmp (item.name, "w") == 0 && strcmp (item.args [0].arg, "end") == 0) {
			break;
		}
		else if (strcmp (item.name, "s") == 0) {
			length = (BKInt) item.argCount;

			for (BKInt i = 0; i < length; i ++)
				sequence [i] = atoi (item.args [i].arg) * (BK_MAX_VOLUME / 255);
		}
	}

	return BKDataInitWithFrames (waveform, sequence, (BKInt) length, 1, 1);
}

static BKEnum numBitsParamFromNumBitsName (char const * numBitsString)
{
	char   sign     = 'u';
	BKInt  isSigned = 0;
	BKInt  numBits  = 0;
	BKEnum param    = 0;

	sscanf (numBitsString, "%u%c", & numBits, & sign);

	isSigned = (sign == 's');

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
		default:
		case 16: {
			param    = BK_16_BIT_SIGNED;
			isSigned = 1;
			break;
		}
	}

	return param;
}

static BKInt parseSample (BKSDLContext * ctx, BKBlipReader * parser, BKData * sample)
{
	BKBlipCommand item;
	BKInt         length = 0;
	BKInt         pitch = 0;
	BKInt         numChannels;
	BKEnum        format;

	while (BKBlipReaderNextCommand (parser, & item)) {
		if (strcmp (item.name, "s") == 0 && strcmp (item.args [0].arg, "end") == 0) {
			break;
		}
		else if (strcmp (item.name, "s") == 0) {
			length = (BKInt) item.argCount;

			if (length >= 3) {
				numChannels = atoi (item.args [0].arg);
				format      = numBitsParamFromNumBitsName (item.args [1].arg);

				BKDataInitWithData (sample, item.args [2].arg, item.args [2].size, numChannels, format);
			}
			else {
				return -1;
			}
		}
		else if (strcmp (item.name, "p") == 0) {
			pitch = atoi (item.args [0].arg);
		}
	}

	BKDataSetAttr (sample, BK_SAMPLE_PITCH, pitch);
	BKDataNormalize (sample);

	return 0;
}

static BKEnum beatCallback (BKCallbackInfo * info, BKSDLUserData * userInfo)
{
	BKInt           numSteps;
	BKInterpreter * interpreter;

	interpreter = userInfo -> interpreter;
	numSteps    = BKInterpreterTrackAdvance (interpreter, userInfo -> track);

	info -> divider = numSteps;

	return 0;
}

BKInt BKSDLContextInit (BKSDLContext * ctx, BKUInt numChannels, BKUInt sampleRate)
{
	memset (ctx, 0, sizeof (BKSDLContext));

	ctx -> speed = 23;

	return BKContextInit (& ctx -> ctx, numChannels, sampleRate);
}

void BKSDLContextUnloadData (BKSDLContext * ctx)
{
	BKSDLTrack * track;

	for (BKInt i = 0; i < ctx -> numWaveforms; i ++) {
		BKDataDispose (ctx -> waveforms [i]);
	}

	ctx -> numWaveforms = 0;

	for (BKInt i = 0; i < ctx -> numInstruments; i ++) {
		BKInstrumentDispose (ctx -> instruments [i]);
		free (ctx -> instruments [i]);
	}

	ctx -> numInstruments = 0;

	for (BKInt i = 0; i < ctx -> numTracks; i ++) {
		track = ctx -> tracks [i];
		BKTrackDispose (& track -> track);
		BKDividerDispose (& track -> divider);
		BKInterpreterDispose (& track -> interpreter);
		free (track);
	}

	ctx -> numTracks = 0;

	BKContextReset (& ctx -> ctx);
}

void BKSDLContextReset (BKSDLContext * ctx, BKInt resetTracks)
{
	BKSDLTrack * track;

	for (BKInt i = 0; i < ctx -> numTracks; i ++) {
		track = ctx -> tracks [i];
		BKInterpreterReset (& track -> interpreter);
		BKDividerReset (& track -> divider);
	}

	if (resetTracks)
		BKContextReset (& ctx -> ctx);
}

void BKSDLContextDispose (BKSDLContext * ctx)
{
	BKSDLContextUnloadData (ctx);
	BKContextDispose (& ctx -> ctx);

	memset (ctx, 0, sizeof (BKSDLContext));
}

BKInt BKSDLContextLoadData (BKSDLContext * ctx, void const * data, size_t size)
{
	BKBlipReader   parser;
	BKCompiler     compiler;
	BKSDLTrack   * track = NULL;
	BKBlipCommand  item;
	BKInt          globalVolume = 0;
	BKInstrument * instrument;
	BKData       * dataObject;

	BKBlipReaderInit (& parser, data, size, NULL, NULL);

	while (BKBlipReaderNextCommand (& parser, & item)) {
		if (strcmp (item.name, "t") == 0) {
			if (strcmp (item.args [0].arg, "begin") == 0) {
				BKCompilerInit (& compiler);

				track = malloc (sizeof (BKSDLTrack));

				if (track) {
					memset (track, 0, sizeof (BKSDLTrack));
					BKTrackInit (& track -> track, 0);

					BKSDLUserData * userData = & track -> userData;
					userData -> track        = & track -> track;
					userData -> interpreter  = & track -> interpreter;

					BKCallback callback = {
						.func     = (BKCallbackFunc) beatCallback,
						.userInfo = userData,
					};

					BKDividerInit (& track -> divider, 12, & callback);
				}
				else {
					return -1;
				}

				BKInt waveform = BK_SQUARE;
				BKInt masterVolume = 0;
				char const * type = item.args [1].arg;

				if (strcmp (type, "square") == 0) {
					waveform = BK_SQUARE;
					masterVolume = BK_MAX_VOLUME * 0.15;
				}
				else if (strcmp (type, "triangle") == 0) {
					waveform = BK_TRIANGLE;
					masterVolume = BK_MAX_VOLUME * 0.30;
				}
				else if (strcmp (type, "noise") == 0) {
					waveform = BK_NOISE;
					masterVolume = BK_MAX_VOLUME * 0.15;
				}
				else if (strcmp (type, "sawtooth") == 0) {
					waveform = BK_SAWTOOTH;
					masterVolume = BK_MAX_VOLUME * 0.15;
				}

				masterVolume = (masterVolume * globalVolume) >> BK_VOLUME_SHIFT;

				BKTrackSetAttr (& track -> track, BK_WAVEFORM, waveform);
				BKTrackSetAttr (& track -> track, BK_MASTER_VOLUME, masterVolume);
				BKTrackSetAttr (& track -> track, BK_VOLUME, BK_MAX_VOLUME);

				while (BKBlipReaderNextCommand (& parser, & item)) {
					if (strcmp (item.name, "t") == 0 && strcmp (item.args [0].arg, "end") == 0) {
						BKCompilerTerminate (& compiler, & track -> interpreter);

						BKTrackAttach (& track -> track, & ctx -> ctx);
						BKContextAttachDivider (& ctx -> ctx, & track -> divider, BK_CLOCK_TYPE_BEAT);
						track -> interpreter.instruments   = ctx -> instruments;
						track -> interpreter.waveforms     = ctx -> waveforms;
						track -> interpreter.samples       = ctx -> samples;
						track -> interpreter.stepTickCount = ctx -> speed;

						ctx -> tracks [ctx -> numTracks ++] = track;

						BKCompilerDispose (& compiler);

						break;
					}
					else {
						BKCompilerPushCommand (& compiler, & item);
					}
				}
			}
		}
		// instrument
		else if (strcmp (item.name, "i") == 0) {
			if (strcmp (item.args [0].arg, "begin") == 0) {
				instrument = parseInstrument (ctx, & parser);

				if (instrument == NULL)
					return -1;

				ctx -> instruments [ctx -> numInstruments ++] = instrument;
			}
		}
		// waveform
		else if (strcmp (item.name, "w") == 0) {
			if (strcmp (item.args [0].arg, "begin") == 0) {
				dataObject = malloc (sizeof (BKData));

				if (dataObject == NULL)
					return -1;

				parseWaveform (ctx, & parser, dataObject);

				ctx -> waveforms [ctx -> numWaveforms ++] = dataObject;
			}
		}
		// sample
		else if (strcmp (item.name, "s") == 0) {
			if (strcmp (item.args [0].arg, "begin") == 0) {
				dataObject = malloc (sizeof (BKData));

				if (dataObject == NULL)
					return -1;

				parseSample (ctx, & parser, dataObject);

				ctx -> samples [ctx -> numSamples ++] = dataObject;
			}
		}
		else if (strcmp (item.name, "gv") == 0) {
			globalVolume = atoi (item.args [0].arg) * (BK_MAX_VOLUME / 255);
		}
		else if (strcmp (item.name, "gs") == 0) {
			ctx -> speed = atoi (item.args [0].arg);
		}
	}

	BKBlipReaderDispose (& parser);

	return 0;
}

static char * dataFromFile (char const * filename, size_t * outSize)
{
	int f = open (filename, O_RDONLY);
	char * data;
	off_t size;

	if (f > -1) {
		size = lseek (f, 0, SEEK_END);
		lseek (f, 0, SEEK_SET);

		data = malloc (size);

		if (data) {
			read (f, data, size);
			* outSize = size;

			return data;
		}
	}

	return NULL;
}

BKInt BKSDLContextLoadFile (BKSDLContext * ctx, char const * filename)
{
	size_t size;
	char * data;
	BKInt  ret = 0;

	data = dataFromFile (filename, & size);

	if (data) {
		ret = BKSDLContextLoadData (ctx, data, size);
		free (data);
	}
	else {
		return -1;
	}

	return ret;
}
