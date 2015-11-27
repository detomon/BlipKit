#include "test.h"

int main (int argc, char const * argv [])
{
	BKInt res;
	BKContext * ctx = INVALID_PTR;

	// check for allocation

	res = BKContextAlloc (& ctx, 2, 44100);

	assert (res == 0);
	assert (ctx != INVALID_PTR && ctx != NULL);

	BKDispose (ctx);

	// check for init values

	res = BKContextAlloc (& ctx, 999, 9999999);

	assert (ctx -> numChannels == BK_MAX_CHANNELS);
	assert (ctx -> sampleRate == BK_MAX_SAMPLE_RATE);

	BKDispose (ctx);

	return 0;
}
