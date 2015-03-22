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

#include <math.h>
#include <stdio.h>
#include "BlipKit.h"
#include "BKTone.h"

static void BKCalcTonePeriods (void)
{
	BKUInt const baseFreq = 440;
	double freq;
	BKFUInt20 period;

	printf (
		"/**\n"
		" * Frational periods\n"
		" * Multiplied by the sample rate gives the wave length of a tone\n"
		" * Generated with `%s`\n"
		" */\n"
		"static BKUInt const tonePeriods [(BK_MAX_PIANO_TONE - BK_MIN_PIANO_TONE) + 1] =\n"
		"{\n\t",
		__FILE__
	);

	for (BKInt i = BK_MIN_PIANO_TONE, c = 0; i <= BK_MAX_PIANO_TONE; i ++, c ++) {
		freq = baseFreq * pow (2.0, ((double) i / 12.0));

		// frequency in seconds
		// so it can be multiplied by the samplerate
		period = 1.0 / freq * BK_FINT20_UNIT;

		if (c && c % 12 == 0)
			printf ("\n\t");

		printf ("%5d, ", period);
	}

	printf ("\n};\n\n");
}

static void BKCalcLog2Periods (void)
{
	BKUInt period;

	printf (
		"/**\n"
		" * Logarithmic `BKFUInt20` values with base 2 (n / 12)\n"
		" * where index 48 (`BK_C_4`) represents factor 1.0\n"
		" * index 36 represent factor 0.5, index 60 represents factor 2.0, and so on\n"
		" * Generated with `%s`\n"
		" */\n"
		"static BKUInt const log2Periods [(BK_MAX_SAMPLE_TONE - BK_MIN_SAMPLE_TONE) + 1] =\n"
		"{\n\t",
		__FILE__
	);

	for (BKInt i = BK_MIN_SAMPLE_TONE, c = 0; i <= BK_MAX_SAMPLE_TONE; i ++, c ++) {
		period = pow (2.0, ((double) i / 12.0)) * BK_FINT20_UNIT;

		if (c && c % 12 == 0)
			printf ("\n\t");

		printf ("%8d, ", period);
	}

	printf ("\n};\n\n");
}

int main (int argc, char const * argv [])
{
	BKCalcTonePeriods ();
	BKCalcLog2Periods ();

	return 0;
}
