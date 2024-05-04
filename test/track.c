#include "test.h"

int main(int argc, char const* argv[]) {
	BKInt res;
	BKTrack* track = INVALID_PTR;

	// check for allocation

	res = BKTrackAlloc(&track, BK_SQUARE);

	assert(res == 0);
	assert(track != INVALID_PTR && track != NULL);

	BKDispose(track);

	// check for init values

	res = BKTrackAlloc(&track, -345876);

	assert(track->waveform == 0);

	// check for allocation

	BKContext* ctx = INVALID_PTR;

	res = BKContextAlloc(&ctx, 2, 44100);

	assert(res == 0);

	res = BKTrackAttach(track, ctx);

	assert(res == 0);

	BKTrackDetach(track);

	assert(track->unit.ctx == NULL);

	BKDispose(track);
	BKDispose(ctx);

	return 0;
}
