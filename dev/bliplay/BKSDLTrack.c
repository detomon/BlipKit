#include <fcntl.h>
#include <unistd.h>
#include "BKSDLTrack.h"

static BKInt parseSequence (BKSDLContext * ctx, BKParserItem * item, BKInt * sequence, BKInt * outRepeatBegin, BKInt * outRepeatLength, BKInt multiplier)
{
	BKInt length = (BKInt) item -> argCount - 2;
	BKInt repeatBegin = 0, repeatLength = 0;

	repeatBegin  = atoi (item -> args [0]);
	repeatLength = atoi (item -> args [1]);

	if (repeatBegin > length)
		repeatBegin = length;

	if (repeatBegin + repeatLength > length)
		repeatLength = length - repeatBegin;

	* outRepeatBegin  = repeatBegin;
	* outRepeatLength = repeatLength;

	for (BKInt i = 0; i < length; i ++)
		sequence [i] = atoi (item -> args [i + 2]) * multiplier;

	return length;
}

static BKInstrument * parseInstrument (BKSDLContext * ctx, BKParser * parser)
{
	BKInstrument * instrument;
	BKParserItem   item;
	BKInt sequence [256];
	BKInt sequenceLength, repeatBegin, repeatLength;
	
	instrument = malloc (sizeof (BKInstrument));
	
	if (instrument == NULL)
		return NULL;
	
	BKInstrumentInit (instrument);
	
	while (BKParserNextItem (parser, & item)) {
		if (strcmp (item.name, "i") == 0 && strcmp (item.args [0], "end") == 0) {
			break;
		}
		else if (strcmp (item.name, "v") == 0) {
			sequenceLength = parseSequence (ctx, & item, sequence, & repeatBegin, & repeatLength, (BK_MAX_VOLUME / 255));
			BKInstrumentSetSequence (instrument, BK_SEQUENCE_VOLUME, sequence, sequenceLength, repeatBegin, repeatLength);
		}
		else if (strcmp (item.name, "a") == 0) {
			sequenceLength = parseSequence (ctx, & item, sequence, & repeatBegin, & repeatLength, (BK_FINT20_UNIT / 12));
			BKInstrumentSetSequence (instrument, BK_SEQUENCE_ARPEGGIO, sequence, sequenceLength, repeatBegin, repeatLength);
		}
		else if (strcmp (item.name, "p") == 0) {
			sequenceLength = parseSequence (ctx, & item, sequence, & repeatBegin, & repeatLength, (BK_MAX_VOLUME / 255));
			BKInstrumentSetSequence (instrument, BK_SEQUENCE_PANNING, sequence, sequenceLength, repeatBegin, repeatLength);
		}
		else if (strcmp (item.name, "dt") == 0) {
			sequenceLength = parseSequence (ctx, & item, sequence, & repeatBegin, & repeatLength, 1);
			BKInstrumentSetSequence (instrument, BK_SEQUENCE_DUTY_CYCLE, sequence, sequenceLength, repeatBegin, repeatLength);
		}
	}
	
	return instrument;
}

static BKInt parseWaveform (BKSDLContext * ctx, BKParser * parser, BKData * waveform)
{
	BKParserItem item;
	BKFrame      sequence [256];
	BKInt        length;
	
	while (BKParserNextItem (parser, & item)) {
		if (strcmp (item.name, "w") == 0 && strcmp (item.args [0], "end") == 0) {
			break;
		}
		else if (strcmp (item.name, "s") == 0) {
			length = (BKInt) item.argCount;
			
			for (BKInt i = 0; i < length; i ++)
				sequence [i] = atoi (item.args [i]) * (BK_MAX_VOLUME / 255);
		}
	}
	
	return BKDataInitWithFrames (waveform, sequence, (BKInt) length, 1, 1);
}

static BKEnum beatCallback (BKCallbackInfo * info, BKSDLUserData * userInfo)
{
	BKInt           numSteps;
	BKInterpreter * interpreter;
	
	interpreter = userInfo -> interpreter;
	numSteps    = BKInterpreterTrackApplyNextStep (interpreter, userInfo -> track);

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

void BKSDLContextDispose (BKSDLContext * ctx)
{
	BKSDLContextUnloadData (ctx);
	BKContextDispose (& ctx -> ctx);

	memset (ctx, 0, sizeof (BKSDLContext));
}

BKInt BKSDLContextLoadData (BKSDLContext * ctx, void const * data, size_t size)
{
	BKParser       parser;
	BKCompiler     compiler;
	BKSDLTrack   * track = NULL;
	BKParserItem   item;
	BKInt          globalVolume = 0;
	BKInstrument * instrument;
	BKData       * waveform;
	
	BKParserInit (& parser, data, size);
	
	while (BKParserNextItem (& parser, & item)) {
		if (strcmp (item.name, "t") == 0) {
			if (strcmp (item.args [0], "begin") == 0) {
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
				
				if (strcmp (item.args [1], "square") == 0) {
					waveform = BK_SQUARE;
					masterVolume = BK_MAX_VOLUME * 0.15;
				}
				else if (strcmp (item.args [1], "triangle") == 0) {
					waveform = BK_TRIANGLE;
					masterVolume = BK_MAX_VOLUME * 0.30;
				}
				else if (strcmp (item.args [1], "noise") == 0) {
					waveform = BK_NOISE;
					masterVolume = BK_MAX_VOLUME * 0.15;
				}
				else if (strcmp (item.args [1], "sawtooth") == 0) {
					waveform = BK_SAWTOOTH;
					masterVolume = BK_MAX_VOLUME * 0.15;
				}
				
				masterVolume = (masterVolume * globalVolume) >> BK_VOLUME_SHIFT;
				
				BKTrackSetAttr (& track -> track, BK_WAVEFORM, waveform);
				BKTrackSetAttr (& track -> track, BK_MASTER_VOLUME, masterVolume);
				BKTrackSetAttr (& track -> track, BK_VOLUME, BK_MAX_VOLUME);
				
				while (BKParserNextItem (& parser, & item)) {
					if (strcmp (item.name, "t") == 0 && strcmp (item.args [0], "end") == 0) {
						BKCompilerTerminate (& compiler, & track -> interpreter);
						
						BKTrackAttach (& track -> track, & ctx -> ctx);
						BKContextAttachDivider (& ctx -> ctx, & track -> divider, BK_CLOCK_TYPE_BEAT);
						track -> interpreter.instruments   = ctx -> instruments;
						track -> interpreter.waveforms     = ctx -> waveforms;
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
			if (strcmp (item.args [0], "begin") == 0) {
				instrument = parseInstrument (ctx, & parser);
				
				if (instrument == NULL)
					return -1;
				
				ctx -> instruments [ctx -> numInstruments ++] = instrument;
			}
		}
		// waveform
		else if (strcmp (item.name, "w") == 0) {
			if (strcmp (item.args [0], "begin") == 0) {
				waveform = malloc (sizeof (BKData));

				if (waveform == NULL)
					return -1;
				
				parseWaveform (ctx, & parser, waveform);
				
				ctx -> waveforms [ctx -> numWaveforms ++] = waveform;
			}
		}
		else if (strcmp (item.name, "gv") == 0) {
			globalVolume = atoi (item.args [0]) * (BK_MAX_VOLUME / 255);
		}
		else if (strcmp (item.name, "gs") == 0) {
			ctx -> speed = atoi (item.args [0]);
		}
	}
	
	BKParserDispose (& parser);
	
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
