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

#include "BKBuffer.h"

/**
 * Bandlimited step phases
 * Generated with `step_phases.c`
 */
BKInt const BKBufferStepPhases [BK_STEP_UNIT][BK_STEP_WIDTH] =
{
	{     0,      2,     -9,     25,    -57,    116,   -226,    519,  32721,   -443,    171,    -76,     32,    -11,      3,      0, },
	{     0,     -2,      9,    -25,     57,   -115,    223,   -504,  32787,    458,   -174,     77,    -32,     11,     -3,      0, },
	{     0,     -7,     28,    -76,    170,   -341,    660,  -1466,  32752,   1417,   -532,    234,    -98,     35,     -9,      0, },
	{     1,    -12,     47,   -126,    281,   -561,   1079,  -2363,  32614,   2432,   -898,    393,   -164,     58,    -15,      1, },
	{     1,    -17,     65,   -175,    388,   -771,   1477,  -3191,  32375,   3499,  -1269,    552,   -229,     82,    -21,      1, },
	{     1,    -22,     83,   -221,    490,   -972,   1852,  -3949,  32037,   4614,  -1642,    710,   -294,    105,    -27,      2, },
	{     2,    -26,     99,   -265,    586,  -1160,   2201,  -4634,  31602,   5772,  -2013,    865,   -358,    127,    -33,      2, },
	{     2,    -31,    115,   -306,    676,  -1335,   2521,  -5245,  31069,   6970,  -2380,   1016,   -419,    149,    -38,      3, },
	{     3,    -34,    129,   -344,    759,  -1495,   2810,  -5780,  30442,   8202,  -2738,   1162,   -478,    169,    -43,      3, },
	{     3,    -38,    142,   -378,    834,  -1638,   3067,  -6238,  29723,   9462,  -3085,   1301,   -533,    189,    -48,      4, },
	{     3,    -41,    154,   -409,    900,  -1765,   3289,  -6620,  28922,  10747,  -3416,   1431,   -585,    206,    -53,      4, },
	{     3,    -44,    164,   -435,    957,  -1873,   3477,  -6925,  28034,  12049,  -3729,   1551,   -632,    223,    -57,      4, },
	{     4,    -46,    173,   -458,   1005,  -1962,   3628,  -7155,  27066,  13363,  -4018,   1660,   -674,    237,    -61,      5, },
	{     4,    -48,    180,   -476,   1043,  -2032,   3742,  -7310,  26027,  14683,  -4281,   1755,   -711,    250,    -64,      5, },
	{     4,    -50,    185,   -489,   1071,  -2082,   3820,  -7392,  24916,  16003,  -4514,   1837,   -741,    260,    -66,      5, },
	{     4,    -51,    189,   -498,   1088,  -2111,   3860,  -7404,  23745,  17316,  -4714,   1903,   -765,    268,    -68,      5, },
	{     4,    -51,    190,   -502,   1096,  -2121,   3863,  -7347,  22519,  18615,  -4877,   1953,   -783,    273,    -70,      5, },
	{     4,    -51,    190,   -501,   1092,  -2111,   3830,  -7224,  21236,  19896,  -4999,   1986,   -793,    276,    -70,      6, },
	{     4,    -51,    188,   -495,   1079,  -2080,   3762,  -7038,  19909,  21150,  -5078,   2000,   -796,    277,    -70,      6, },
	{     4,    -50,    184,   -485,   1055,  -2031,   3660,  -6794,  18551,  22371,  -5111,   1995,   -791,    274,    -70,      5, },
	{     4,    -48,    179,   -470,   1022,  -1963,   3526,  -6495,  17155,  23554,  -5095,   1970,   -778,    269,    -68,      5, },
	{     4,    -46,    172,   -451,    979,  -1877,   3360,  -6145,  15737,  24691,  -5026,   1925,   -757,    262,    -66,      5, },
	{     3,    -44,    163,   -427,    927,  -1773,   3165,  -5748,  14304,  25777,  -4904,   1860,   -729,    251,    -63,      5, },
	{     3,    -41,    152,   -400,    866,  -1654,   2944,  -5308,  12859,  26806,  -4725,   1774,   -692,    238,    -60,      5, },
	{     3,    -38,    141,   -369,    797,  -1521,   2697,  -4832,  11415,  27773,  -4488,   1667,   -648,    222,    -56,      4, },
	{     3,    -34,    127,   -334,    722,  -1374,   2429,  -4324,   9972,  28671,  -4192,   1540,   -596,    204,    -51,      4, },
	{     2,    -30,    113,   -296,    639,  -1215,   2142,  -3788,   8543,  29495,  -3834,   1392,   -536,    183,    -46,      3, },
	{     2,    -26,     98,   -255,    551,  -1045,   1837,  -3230,   7131,  30241,  -3415,   1225,   -470,    160,    -40,      3, },
	{     1,    -22,     81,   -212,    457,   -867,   1519,  -2655,   5748,  30905,  -2934,   1039,   -396,    135,    -34,      2, },
	{     1,    -17,     64,   -167,    360,   -681,   1191,  -2069,   4394,  31481,  -2390,    835,   -317,    107,    -27,      2, },
	{     1,    -12,     46,   -120,    259,   -490,    854,  -1476,   3078,  31967,  -1783,    615,   -232,     78,    -19,      1, },
	{     0,     -7,     28,    -73,    156,   -295,    513,   -882,   1808,  32360,  -1115,    379,   -142,     48,    -12,      1, },
};

BKInt BKBufferInit (BKBuffer * buf)
{
	BKBufferClear (buf);

	return 0;
}

void BKBufferDispose (BKBuffer * buf)
{
	BKBufferClear (buf);
}

BKInt BKBufferRead (BKBuffer * buf, BKFrame outFrames [], BKUInt size, BKUInt interlace)
{
	BKInt * frames = & buf -> frames [0];
	BKInt   accum  = buf -> accum;
	BKUInt  bufferSize;
	BKInt   amp;

	interlace = BKMax (interlace, 1);          // step must be at least 1
	size      = BKMin (size, buf -> capacity); // can only read available frames

	while (frames < & buf -> frames [size]) {
		accum -= (accum >> (BK_INT_SHIFT - BK_HIGH_PASS_SHIFT));  // apply high pass filter
		accum += (* frames ++);                                   // accumulate

		amp = accum >> (BK_INT_SHIFT - BK_FRAME_SHIFT - 2);  // remove fraction

		// clamp
		if ((BKFrame) amp != amp)
			amp = (amp >> BK_FRAME_SHIFT) ^ BK_FRAME_MAX;

		// write frame
		(* outFrames) = amp;
		outFrames += interlace;
	}

	bufferSize = buf -> capacity + BK_STEP_WIDTH + 1;

	// move frames left
	memmove (& buf -> frames [0], & buf -> frames [size], sizeof (BKInt) * (bufferSize - size));
	// zero right gap
	memset (& buf -> frames [bufferSize - size], 0, sizeof (BKInt) * size);
	// reduce remaining capacity
	buf -> capacity -= size;

	buf -> accum = accum;
	buf -> time -= size << BK_FINT20_SHIFT;

	return size;
}

void BKBufferClear (BKBuffer * buf)
{
	memset (buf, 0, sizeof (BKBuffer));
}
