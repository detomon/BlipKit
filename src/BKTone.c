/**
 * Copyright (c) 2012-2013 Simon Schoenenberger
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

#include "BKTone.h"

#define BK_MIN_PIANO_TONE (3 - 4 * 12 + BK_C_0)
#define BK_MAX_PIANO_TONE (BK_MIN_PIANO_TONE + BK_C_8)

#define BK_TONE_SHIFT 16
#define BK_TONE_UNIT (1 << BK_TONE_SHIFT)
#define BK_TONE_FRAC (BK_TONE_UNIT - 1)

#define BK_SAMPLE_RATE_SHIFT 18

/**
 * Frational periods
 * Multiplied by the sample rate gives the wave length of a tone
 */
static BKUInt const tonePeriods [(BK_MAX_PIANO_TONE - BK_MIN_PIANO_TONE) + 1] =
{
	32063, 	30263, 	28565, 	26962, 	25448, 	24020, 	22672, 	21399, 	20198, 	19065, 	17994, 	16985,
	16031, 	15131, 	14282, 	13481, 	12724, 	12010, 	11336, 	10699, 	10099, 	 9532, 	 8997, 	 8492,
	8015, 	 7565, 	 7141, 	 6740, 	 6362, 	 6005, 	 5668, 	 5349, 	 5049, 	 4766, 	 4498, 	 4246,
	4007, 	 3782, 	 3570, 	 3370, 	 3181, 	 3002, 	 2834, 	 2674, 	 2524, 	 2383, 	 2249, 	 2123,
	2003, 	 1891, 	 1785, 	 1685, 	 1590, 	 1501, 	 1417, 	 1337, 	 1262, 	 1191, 	 1124, 	 1061,
	1001, 	  945, 	  892, 	  842, 	  795, 	  750, 	  708, 	  668, 	  631, 	  595, 	  562, 	  530,
	500, 	  472, 	  446, 	  421, 	  397, 	  375, 	  354, 	  334, 	  315, 	  297, 	  281, 	  265,
	250, 	  236, 	  223, 	  210, 	  198, 	  187, 	  177, 	  167, 	  157, 	  148, 	  140, 	  132,
	125,
};

/****
#include <math.h>

void BKCalcTonePeriods (void)
{
	BKUInt const baseFreq = 440;
	double freq;
	BKFUInt20 period;

	for (BKInt i = BK_MIN_PIANO_TONE, c = 0; i <= BK_MAX_PIANO_TONE; i ++, c ++) {
		freq = baseFreq * pow (2.0, ((double) i / 12.0));

		// frequency in seconds
		// so it can be multiplied by the samplerate
		period = 1.0 / freq * BK_FINT20_UNIT;
		//tonePeriods [i] = period;

		//BKUInt eff = (double) 48000 / (((double) period * 48000) / BK_FINT20_UNIT);
		//printf ("%f, %2d %f\n", freq, i, (1.0 - (double) freq / eff) * 100);

		if (c && c % 12 == 0)
			printf ("\n");

		printf ("\t%5d, ", period);
	}
	
	printf ("\n\n");
}
****/

BKFUInt20 BKTonePeriodLookup (BKFInt20 tone, BKUInt sampleRate)
{
	//BKCalcTonePeriods ();
	
	BKUInt    frac;
	BKUInt    period1, period2;
	BKFUInt20 period;

	tone = BKClamp (tone, BK_MIN_NOTE << BK_FINT20_SHIFT, BK_MAX_NOTE << BK_FINT20_SHIFT);
	frac = ((tone & BK_FINT20_FRAC) >> (BK_FINT20_SHIFT - BK_TONE_SHIFT));
	tone = tone >> BK_FINT20_SHIFT;

	sampleRate = BKClamp (sampleRate, BK_MIN_SAMPLE_RATE, BK_MAX_SAMPLE_RATE);

	period1 = tonePeriods [tone];
	period2 = tonePeriods [BKMin (tone + 1, BK_MAX_NOTE)];

	period = period1 * (BK_TONE_UNIT - frac) + period2 * frac;
	period = ((period >> BK_SAMPLE_RATE_SHIFT) * sampleRate) << (BK_SAMPLE_RATE_SHIFT - BK_TONE_SHIFT);

	return period;
}
