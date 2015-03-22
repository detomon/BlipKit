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

#include "BKTone.h"

/**
 * Frational periods
 * Multiplied by the sample rate gives the wave length of a tone
 * Generated with `tone_periods.c`
 */
static BKUInt const tonePeriods [(BK_MAX_PIANO_TONE - BK_MIN_PIANO_TONE) + 1] =
{
	32063, 30263, 28565, 26962, 25448, 24020, 22672, 21399, 20198, 19065, 17994, 16985,
	16031, 15131, 14282, 13481, 12724, 12010, 11336, 10699, 10099,  9532,  8997,  8492,
	 8015,  7565,  7141,  6740,  6362,  6005,  5668,  5349,  5049,  4766,  4498,  4246,
	 4007,  3782,  3570,  3370,  3181,  3002,  2834,  2674,  2524,  2383,  2249,  2123,
	 2003,  1891,  1785,  1685,  1590,  1501,  1417,  1337,  1262,  1191,  1124,  1061,
	 1001,   945,   892,   842,   795,   750,   708,   668,   631,   595,   562,   530,
	  500,   472,   446,   421,   397,   375,   354,   334,   315,   297,   281,   265,
	  250,   236,   223,   210,   198,   187,   177,   167,   157,   148,   140,   132,
	  125,
};

/**
 * Logarithmic `BKFUInt20` values with base 2 (n / 12)
 * where index 48 (`BK_C_4`) represents factor 1.0
 * index 36 represent factor 0.5, index 60 represents factor 2.0, and so on
 * Generated with `tone_periods.c`
 */
static BKUInt const log2Periods [(BK_MAX_SAMPLE_TONE - BK_MIN_SAMPLE_TONE) + 1] =
{
	   65536,    69432,    73561,    77935,    82570,    87480,    92681,    98193,   104031,   110217,   116771,   123715,
	  131072,   138865,   147123,   155871,   165140,   174960,   185363,   196386,   208063,   220435,   233543,   247430,
	  262144,   277731,   294246,   311743,   330280,   349920,   370727,   392772,   416127,   440871,   467087,   494861,
	  524288,   555463,   588493,   623487,   660561,   699840,   741455,   785544,   832255,   881743,   934175,   989723,
	 1048576,  1110927,  1176986,  1246974,  1321122,  1399681,  1482910,  1571088,  1664510,  1763487,  1868350,  1979447,
	 2097152,  2221855,  2353973,  2493948,  2642245,  2799362,  2965820,  3142177,  3329021,  3526975,  3736700,  3958895,
	 4194304,  4443710,  4707947,  4987896,  5284491,  5598724,  5931641,  6284355,  6658042,  7053950,  7473400,  7917791,
	 8388608,  8887420,  9415894,  9975792, 10568983, 11197448, 11863283, 12568710, 13316085, 14107900, 14946800, 15835583,
	16777216,
};

BKFUInt20 BKTonePeriodLookup (BKFInt20 tone, BKUInt sampleRate)
{
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
	period = ((period >> BK_TONE_SAMPLE_RATE_SHIFT) * sampleRate) << (BK_TONE_SAMPLE_RATE_SHIFT - BK_TONE_SHIFT);

	return period;
}

BKFUInt20 BKLog2PeriodLookup (BKFInt20 tone)
{
	BKUInt    frac;
	BKUInt    period1, period2;
	BKFUInt20 period;

	tone = BKClamp (tone, BK_MIN_NOTE << BK_FINT20_SHIFT, BK_MAX_NOTE << BK_FINT20_SHIFT);
	frac = (tone & BK_FINT20_FRAC) >> 12;
	tone = tone >> BK_FINT20_SHIFT;

	period1 = log2Periods [tone] >> 2;
	period2 = log2Periods [BKMin (tone + 1, BK_MAX_NOTE)] >> 2;

	period = period1 * ((BK_FINT20_UNIT >> 12) - frac) + period2 * frac;
	period >>= 6;

	return period;
}
