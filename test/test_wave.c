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

	if (res != 0) {
		fprintf (stderr, "Initializing BKContext failed (%d)\n", res);
		return RESULT_ERROR;
	}

	BKTrack track;

	res = BKTrackInit (& track, BK_SAWTOOTH);

	if (res != 0) {
		fprintf (stderr, "Initializing BKTrack failed (%d)\n", res);
		return RESULT_ERROR;
	}

	res = BKTrackAttach (& track, & ctx);

	if (res != 0) {
		fprintf (stderr, "Failed to attach track (%d)\n", res);
		return RESULT_ERROR;
	}

	BKSetAttr (& track, BK_MASTER_VOLUME, 0.2 * BK_MAX_VOLUME);
	BKSetAttr (& track, BK_VOLUME, BK_MAX_VOLUME);
	BKSetAttr (& track, BK_NOTE, BK_F_2 * BK_FINT20_UNIT);

	BKInt numFrames = 481, readNumFrames;
	BKFrame * frames = malloc (numChannels * numFrames * sizeof (BKFrame));

	if (frames == NULL) {
		fprintf (stderr, "Failed to allocate frames\n");
		return RESULT_ERROR;
	}

	FILE * file = fopen (filename, "w+");

	if (file == NULL) {
		fprintf (stderr, "Failed to open file (%d)\n", errno);
		return RESULT_ERROR;
	}

	res = BKWaveFileWriterInit (& writer, file, numChannels, sampleRate);

	if (res != 0) {
		fprintf (stderr, "BKWaveFileWriter initializing failed (%d)\n", res);
		return RESULT_ERROR;
	}

	for (int i = 0; i < 100; i ++) {
		res = BKContextGenerate (& ctx, frames, numFrames);

		if (res != numFrames) {
			fprintf (stderr, "Invalid number of frames generated (%d)\n", res);
			return RESULT_ERROR;
		}

		res = BKWaveFileWriterAppendFrames (& writer, frames, numChannels * numFrames);

		if (res != 0) {
			fprintf (stderr, "Failed to write frames (%d)\n", res);
			return RESULT_ERROR;
		}
	}

	res = BKWaveFileWriterTerminate (& writer);

	if (res != 0) {
		fprintf (stderr, "Failed to terminate BKWaveFileWriter (%d)\n", res);
		return RESULT_ERROR;
	}

	BKDispose (& writer);

	fclose (file);
	file = fopen (filename, "r");

	if (file == NULL) {
		fprintf (stderr, "Failed to open file (%d)\n", errno);
		return RESULT_ERROR;
	}

	res = BKWaveFileReaderInit (& reader, file);

	if (res != 0) {
		fprintf (stderr, "BKWaveFileReader initializing failed (%d)\n", res);
		return RESULT_ERROR;
	}

	res = BKWaveFileReaderReadHeader (& reader, & readNumChannels, & readSampleRate, & readNumFrames);

	if (res != 0) {
		fprintf (stderr, "Failed to read WAVE header (%d)\n", res);
		return RESULT_ERROR;
	}

	printf("Read WAVE file: numChannels: %d, sampleRate, %u, numFrames: %u\n", readNumChannels, readSampleRate, readNumFrames);

	if (numChannels != readNumChannels) {
		fprintf (stderr, "Read number of channels differ: (%d, %d)\n", numChannels, readNumChannels);
		return RESULT_ERROR;
	}

	if (sampleRate != readSampleRate) {
		fprintf (stderr, "Read sample rate differs: (%d, %d)\n", sampleRate, readSampleRate);
		return RESULT_ERROR;
	}

	if (numFrames * 100 != readNumFrames) {
		fprintf (stderr, "Read number of frames differ: (%d, %d)\n", numFrames * 100, readNumFrames);
		return RESULT_ERROR;
	}

	frames = realloc (frames, readNumChannels * readNumFrames * sizeof (BKFrame));

	if (frames == NULL) {
		fprintf (stderr, "Reallocation failed\n");
		return RESULT_ERROR;
	}

	res = BKWaveFileReaderReadFrames (& reader, frames);

	if (res != 0) {
		fprintf (stderr, "Failed to read frames (%d)\n", res);
		return RESULT_ERROR;
	}

	BKDispose (& reader);

	fclose (file);
	unlink (filename);

	free (frames);

	BKDispose (& ctx);
	BKDispose (& track);

	return 0;
}
