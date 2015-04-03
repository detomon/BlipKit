/**
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


#include <stdio.h>
#include "BKSequence.h"

int main (int argc, char const * argv [])
{
	/*BKInt max = 100;
	BKInt attack = 10;
	BKInt decay = 10;
	BKInt sustain = max * 0.5;
	BKInt release = 10;
	
	BKSequencePhase const phases [4] = {
		{attack, max},
		{decay, sustain},
		{1, sustain},
		{release, 0},
	};*/
	
	BKSequencePhase const phases [4] = {
		{5, 0},
		{10, -100},
		{10, +100},
		{5, 0},
	};

	
	BKInt values [] = {
		24, 32, 64, 48, 32, 24, 16, 8,
	};

	BKSequence * sequence;
	BKSequenceState state;
	
	BKSequenceCreate (& sequence, & BKSequenceFuncsEnvelope, phases, 4, 1, 2);
	
	memset (& state, 0, sizeof (state));

	BKSequenceStateSetSequence (& state, sequence);

	BKSequenceStateSetPhase (& state, BK_SEQUENCE_PHASE_ATTACK);
	
	for (BKInt i = 0; ; i ++) {
		printf ("%d\n", state.value);
		
		if (BKSequenceStateStep (& state, 1) == BK_SEQUENCE_RETURN_FINISH)
			break;
		
		if (i == 30)
			BKSequenceStateSetPhase (& state, BK_SEQUENCE_PHASE_RELEASE);
	}
	
	BKSequenceDispose (sequence);
	
	return 0;
}
