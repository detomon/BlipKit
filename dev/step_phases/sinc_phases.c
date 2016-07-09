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

#include <math.h>
#include <stdio.h>
#include "BKBuffer.h"

static BKInt stepPhases [BK_STEP_UNIT][BK_STEP_WIDTH];

int main (int argc, char const * argv [])
{
	int const size = BK_STEP_WIDTH;
	double phasef [size];

	printf (
		"/**\n"
		" * Bandlimited step phases\n"
		" * Generated with `%s`\n"
		" */\n"
		"BKInt const BKBufferStepPhases [BK_STEP_UNIT][BK_STEP_WIDTH] =\n{\n",
		__FILE__
	);

	// step phase
	for (int phase = 0; phase < BK_STEP_UNIT; phase ++) {
		double sumf  = 0.0;
		BKInt  value = 0;
		BKInt  sum   = 0;

		printf ("\t{");

		// phase offset
		for (BKInt i = 0; i < size; i ++) {
			double delta  = 0.0;
			double iphase = i - (size / 2) - ((double) phase / BK_STEP_UNIT) + (1.0 / BK_STEP_UNIT / 2);

			//iphase += (0.3 / 16.0);

			//printf("%lf ", iphase);

			// prevent division by zero
			if (iphase != 0.0) {
				// sinc
				delta = sin (iphase * M_PI) / (iphase * M_PI);
				// apply Blackman window
				double w = i + 0.5;
				delta *= 0.42 - 0.5 * cos (2 * M_PI * w / size) + 0.08 * cos (4 * M_PI * w / size);
			}
			else {
				delta = 1.0;
			}

			phasef [i] = delta;
			sumf += delta;
		}

		// normalize step
		for (BKInt i = 0; i < size; i ++) {
			value = phasef [i] / sumf * BK_FRAME_MAX;
			stepPhases [phase][i] = value;
			sum += value;
		}

		// correct round-off error
		stepPhases [phase][size / 2] += (BK_FRAME_MAX - sum);

		for (BKInt i = 0; i < size; i ++) {
			printf ("%6d, ", stepPhases [phase][i]);
		}

		printf ("},\n");
	}

	printf ("};\n\n");

	return 0;
}
