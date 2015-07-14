#include "test.h"

int main (int argc, char const * argv [])
{
	BKInt res;
	BKContext * ctx = INVALID_PTR;

	// check for allocation

	res = BKContextAlloc (& ctx, 2, 44100);

	if (res != 0) {
		fprintf (stderr, "Allocation failed (%d)\n", res);
		return RESULT_ERROR;
	}

	if (ctx == INVALID_PTR || ctx == NULL) {
		fprintf (stderr, "Invalid pointer (%p)\n", ctx);
		return RESULT_ERROR;
	}

	BKDispose (ctx);

	// check for init values

	res = BKContextAlloc (& ctx, 999, 9999999);

	if (ctx -> numChannels != BK_MAX_CHANNELS) {
		fprintf (stderr, "Invalid clamped value for channels (%d)\n", ctx -> numChannels);
		return RESULT_ERROR;
	}

	if (ctx -> sampleRate != BK_MAX_SAMPLE_RATE) {
		fprintf (stderr, "Invalid clamped value for sample rate (%d)\n", ctx -> sampleRate);
		return RESULT_ERROR;
	}

	BKDispose (ctx);

	return 0;
}
