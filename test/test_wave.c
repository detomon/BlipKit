#include <unistd.h>
#include <errno.h>
#include "test.h"
#include "BKWaveFileReader.h"
#include "BKWaveFileWriter.h"

int main (int argc, char const * argv [])
{
	BKInt res;
	char const * filename = "bk_test_wave.wav";
	BKInt numChannels = 2, readNumChannels;
	BKInt sampleRate = 44100, readSampleRate;
	BKWaveFileReader reader;
	BKWaveFileWriter writer;

	BKContext ctx;

	res = BKContextInit (& ctx, numChannels, sampleRate);

	assert (res == 0);

	BKTrack track;

	res = BKTrackInit (& track, BK_SAWTOOTH);

	assert (res == 0);

	res = BKTrackAttach (& track, & ctx);

	assert (res == 0);

	BKSetAttr (& track, BK_MASTER_VOLUME, 0.2 * BK_MAX_VOLUME);
	BKSetAttr (& track, BK_VOLUME, BK_MAX_VOLUME);
	BKSetAttr (& track, BK_NOTE, BK_F_2 * BK_FINT20_UNIT);

	BKInt numFrames = 481, readNumFrames;
	BKFrame * frames = malloc (numChannels * numFrames * sizeof (BKFrame));

	assert (frames != 0);

	FILE * file = fopen (filename, "w+");

	assert (file != NULL);

	res = BKWaveFileWriterInit (& writer, file, numChannels, sampleRate);

	assert (res == 0);

	for (int i = 0; i < 100; i ++) {
		res = BKContextGenerate (& ctx, frames, numFrames);

		assert (res == numFrames);

		res = BKWaveFileWriterAppendFrames (& writer, frames, numChannels * numFrames);

		assert (res == 0);
	}

	res = BKWaveFileWriterTerminate (& writer);

	assert (res == 0);

	BKDispose (& writer);

	fclose (file);
	file = fopen (filename, "r");

	assert (file != NULL);

	res = BKWaveFileReaderInit (& reader, file);

	assert (res == 0);

	res = BKWaveFileReaderReadHeader (& reader, & readNumChannels, & readSampleRate, & readNumFrames);

	assert (res == 0);

	assert (numChannels == readNumChannels);
	assert (sampleRate == readSampleRate);
	assert (numFrames * 100 == readNumFrames);

	frames = realloc (frames, readNumChannels * readNumFrames * sizeof (BKFrame));

	assert (frames != NULL);

	res = BKWaveFileReaderReadFrames (& reader, frames);

	assert (res == 0);

	BKDispose (& reader);

	fclose (file);
	unlink (filename);

	free (frames);

	BKDispose (& ctx);
	BKDispose (& track);

	return 0;
}
