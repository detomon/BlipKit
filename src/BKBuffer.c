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

#include "BKBuffer.h"

/**
 * Bandlimited step phases
 */
static BKFrame const stepPhases [BK_STEP_UNIT][BK_STEP_WIDTH] =
{
	{     0,     -3,     13,    -37,     87,   -192,    483,  32755,   -468,    189,    -86,     36,    -13,      3,      0, },
	{     0,      3,    -13,     36,    -86,    189,   -468,  32755,    483,   -192,     87,    -37,     13,     -3,      0, },
	{     0,     10,    -39,    109,   -256,    558,  -1360,  32654,   1493,   -584,    264,   -112,     40,    -10,      0, },
	{    -1,     16,    -64,    179,   -419,    910,  -2187,  32454,   2558,   -984,    442,   -187,     66,    -17,      1, },
	{    -1,     23,    -89,    247,   -576,   1244,  -2949,  32153,   3673,  -1388,    620,   -261,     93,    -24,      2, },
	{    -2,     29,   -112,    312,   -724,   1556,  -3642,  31755,   4834,  -1792,    796,   -334,    119,    -30,      2, },
	{    -3,     35,   -135,    372,   -863,   1846,  -4265,  31265,   6036,  -2193,    968,   -406,    144,    -37,      3, },
	{    -3,     40,   -155,    429,   -991,   2110,  -4818,  30679,   7275,  -2587,   1135,   -475,    168,    -43,      3, },
	{    -3,     45,   -174,    480,  -1108,   2348,  -5299,  30003,   8544,  -2971,   1296,   -540,    191,    -49,      4, },
	{    -4,     49,   -191,    527,  -1212,   2558,  -5709,  29242,   9839,  -3341,   1448,   -602,    213,    -54,      4, },
	{    -4,     53,   -206,    567,  -1303,   2738,  -6047,  28399,  11154,  -3693,   1589,   -659,    233,    -59,      5, },
	{    -4,     57,   -219,    603,  -1380,   2889,  -6315,  27474,  12483,  -4023,   1720,   -710,    250,    -63,      5, },
	{    -5,     60,   -230,    631,  -1444,   3009,  -6512,  26481,  13820,  -4328,   1837,   -756,    266,    -67,      5, },
	{    -5,     62,   -239,    654,  -1492,   3099,  -6642,  25418,  15157,  -4603,   1939,   -796,    280,    -71,      6, },
	{    -5,     64,   -245,    670,  -1526,   3157,  -6705,  24291,  16490,  -4845,   2026,   -829,    291,    -73,      6, },
	{    -5,     65,   -249,    680,  -1545,   3185,  -6703,  23107,  17812,  -5051,   2095,   -854,    299,    -75,      6, },
	{    -5,     65,   -251,    683,  -1550,   3182,  -6640,  21875,  19115,  -5216,   2147,   -872,    305,    -77,      6, },
	{    -5,     65,   -250,    680,  -1539,   3149,  -6518,  20596,  20395,  -5338,   2178,   -882,    307,    -77,      6, },
	{    -5,     64,   -247,    671,  -1515,   3088,  -6340,  19278,  21643,  -5413,   2190,   -883,    307,    -77,      6, },
	{    -5,     63,   -241,    655,  -1476,   2999,  -6109,  17927,  22854,  -5439,   2181,   -876,    304,    -76,      6, },
	{    -5,     61,   -233,    633,  -1424,   2884,  -5830,  16553,  24022,  -5412,   2150,   -861,    298,    -75,      6, },
	{    -5,     58,   -223,    605,  -1359,   2744,  -5506,  15159,  25140,  -5331,   2098,   -836,    289,    -72,      6, },
	{    -4,     55,   -211,    572,  -1283,   2581,  -5142,  13755,  26203,  -5192,   2023,   -803,    277,    -69,      5, },
	{    -4,     52,   -197,    534,  -1195,   2396,  -4742,  12348,  27204,  -4995,   1926,   -762,    262,    -65,      5, },
	{    -4,     47,   -182,    491,  -1096,   2192,  -4309,  10942,  28140,  -4737,   1807,   -712,    244,    -61,      5, },
	{    -3,     43,   -164,    443,   -989,   1971,  -3850,   9544,  29003,  -4417,   1667,   -653,    224,    -56,      4, },
	{    -3,     38,   -145,    392,   -873,   1735,  -3367,   8161,  29790,  -4034,   1505,   -587,    201,    -50,      4, },
	{    -2,     33,   -125,    337,   -750,   1486,  -2867,   6804,  30495,  -3587,   1322,   -514,    175,    -43,      3, },
	{    -2,     27,   -104,    280,   -621,   1227,  -2353,   5473,  31116,  -3077,   1120,   -433,    147,    -36,      3, },
	{    -1,     21,    -82,    220,   -487,    960,  -1830,   4177,  31648,  -2502,    899,   -346,    117,    -29,      2, },
	{    -1,     15,    -59,    158,   -350,    688,  -1304,   2925,  32087,  -1865,    661,   -253,     85,    -21,      1, },
	{     0,      9,    -35,     95,   -210,    412,   -778,   1714,  32432,  -1164,    407,   -155,     52,    -13,      1, },
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

BKInt BKBufferUpdateStep (BKBuffer * buf, BKFUInt20 time, BKFrame pulse)
{
	BKUInt          frac;
	BKUInt          offset;
	BKInt         * frames;
	BKFrame const * phase;

	time   = buf -> time + time;
	offset = time >> BK_FINT20_SHIFT;

	frac = time & BK_FINT20_FRAC;                // sample fraction
	frac >>= (BK_FINT20_SHIFT - BK_STEP_SHIFT);  // step fraction

	phase  = stepPhases [frac];
	frames = & buf -> frames [offset];

	// add step
	for (BKInt i = 0; i < BK_STEP_WIDTH; i ++)
		frames [i] += phase [i] * pulse;
	
	return 0;
}

BKInt BKBufferUpdateSample (BKBuffer * buf, BKFUInt20 time, BKFrame pulse)
{
	BKUInt offset;

	time   = buf -> time + time;
	offset = time >> BK_FINT20_SHIFT;

	buf -> frames [offset] += BK_MAX_VOLUME * pulse;

	return 0;
}

BKInt BKBufferEnd (BKBuffer * buf, BKFUInt20 time)
{
	BKUInt offset;

	time   = buf -> time + time;
	offset = time >> BK_FINT20_SHIFT;

	if (offset > buf -> capacity)
		buf -> capacity = offset;

	return 0;
}

BKInt BKBufferShift (BKBuffer * buf, BKFUInt20 time)
{
	buf -> time += time;

	return 0;
}

BKInt BKBufferSize (BKBuffer const * buf)
{
	return buf -> time >> BK_FINT20_SHIFT;
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
