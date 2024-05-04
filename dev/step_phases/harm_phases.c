/**
 * Copyright (c) 2012-2016 Simon Schoenenberger
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

#include "BKBase.h"
#include "BKBuffer.h"
#include <math.h>
#include <stdio.h>

static BKInt stepPhases[BK_STEP_UNIT][BK_STEP_WIDTH];

int main(int argc, char const* argv[]) {
	int const size = BK_STEP_WIDTH;

	printf(
		"/**\n"
		" * Bandlimited step phases\n"
		" * Generated with `%s`\n"
		" */\n"
		"BKInt const BKBufferStepPhases [BK_STEP_UNIT][BK_STEP_WIDTH] =\n{\n",
		__FILE__);

	// step phase
	for (int phase = 0; phase < BK_STEP_UNIT; phase++) {
		double wave[BK_STEP_UNIT + 1];
		double shift = -((double)phase / BK_STEP_UNIT);

		memset(wave, 0, sizeof(wave));

		printf("\t{");

		// phase offset
		for (double n = 1; n <= 31; n += 2) {
			for (double x = -size / 2; x <= size / 2; x++) {
				double a = (x + shift) * M_PI / 31;

				wave[(int)x + size / 2] += sin(n * a) / n;
			}
		}

		double sum = 0;
		double sumf = 0;
		double last = wave[0];
		BKInt value = 0;

		for (int i = 0; i <= size; i++) {
			double diff = wave[i] - last;
			last = wave[i];
			wave[i] = diff;
			sumf += diff;
		}

		for (int i = 1; i <= size; i++) {
			wave[i] /= sumf;

			// apply Blackman window
			double w = i - 0.5;
			wave[i] *= 0.42 - 0.5 * cos(2 * M_PI * w / size) + 0.08 * cos(4 * M_PI * w / size);

			value = wave[i] * BK_FRAME_MAX;
			stepPhases[phase][i - 1] = value;
			sum += value;
		}

		// correct round-off error
		stepPhases[phase][size / 2] += (BK_FRAME_MAX - sum);

		for (BKInt i = 0; i < size; i++) {
			printf("%6d, ", stepPhases[phase][i]);
		}

		printf("},\n");
	}

	printf("};\n\n");

	return 0;
}
