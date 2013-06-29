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
	
	
	
	
	/*43, -115, 350, -488, 1136, -914, 5861, 21022, 6464, -1021, 1190, -499, 350, -110, 40, 1,
	44, -118, 348, -473, 1076, -799, 5274, 21001, 7082, -1119, 1238, -506, 347, -102, 35, 3,
	45, -121, 344, -454, 1011, -677, 4706, 20936, 7713, -1205, 1278, -507, 341, -94, 31, 4,
	46, -122, 336, -431, 942, -549, 4156, 20829, 8355, -1280, 1312, -504, 333, -85, 26, 6,
	47, -123, 327, -404, 868, -418, 3629, 20679, 9005, -1339, 1337, -496, 322, -75, 22, 7,
	47, -122, 316, -375, 792, -285, 3124, 20488, 9660, -1383, 1354, -483, 309, -63, 16, 9,
	47, -120, 303, -344, 714, -151, 2644, 20256, 10319, -1410, 1362, -464, 292, -49, 9, 11,
	46, -117, 289, -310, 634, -17, 2188, 19985, 10979, -1419, 1361, -439, 272, -35, 3, 13,
	46, -114, 273, -275, 553, 117, 1758, 19675, 11638, -1408, 1351, -410, 250, -19, -4, 15,
	44, -108, 255, -237, 471, 247, 1356, 19327, 12293, -1376, 1331, -375, 226, -3, -12, 18,
	43, -103, 237, -199, 390, 373, 981, 18944, 12942, -1322, 1301, -335, 199, 16, -20, 20,
	42, -98, 218, -160, 310, 495, 633, 18527, 13582, -1244, 1261, -290, 170, 34, -27, 22,
	40, -91, 198, -121, 231, 611, 314, 18078, 14210, -1142, 1211, -239, 139, 53, -36, 25,
	38, -84, 178, -81, 153, 722, 22, 17599, 14824, -1015, 1152, -184, 106, 73, -44, 27,
	36, -76, 157, -43, 80, 824, -241, 17092, 15422, -862, 1083, -123, 70, 94, -52, 29,
	34, -68, 135, -3, 8, 919, -476, 16558, 16001, -683, 1006, -60, 34, 115, -61, 32,
	32, -61, 115, 34, -60, 1006, -683, 16001, 16558, -476, 919, 8, -3, 135, -68, 34,
	29, -52, 94, 70, -123, 1083, -862, 15422, 17092, -241, 824, 80, -43, 157, -76, 36,
	27, -44, 73, 106, -184, 1152, -1015, 14824, 17599, 22, 722, 153, -81, 178, -84, 38,
	25, -36, 53, 139, -239, 1211, -1142, 14210, 18078, 314, 611, 231, -121, 198, -91, 40,
	22, -27, 34, 170, -290, 1261, -1244, 13582, 18527, 633, 495, 310, -160, 218, -98, 42,
	20, -20, 16, 199, -335, 1301, -1322, 12942, 18944, 981, 373, 390, -199, 237, -103, 43,
	18, -12, -3, 226, -375, 1331, -1376, 12293, 19327, 1356, 247, 471, -237, 255, -108, 44,
	15, -4, -19, 250, -410, 1351, -1408, 11638, 19675, 1758, 117, 553, -275, 273, -114, 46,
	13, 3, -35, 272, -439, 1361, -1419, 10979, 19985, 2188, -17, 634, -310, 289, -117, 46,
	11, 9, -49, 292, -464, 1362, -1410, 10319, 20256, 2644, -151, 714, -344, 303, -120, 47,
	9, 16, -63, 309, -483, 1354, -1383, 9660, 20488, 3124, -285, 792, -375, 316, -122, 47,
	7, 22, -75, 322, -496, 1337, -1339, 9005, 20679, 3629, -418, 868, -404, 327, -123, 47,
	6, 26, -85, 333, -504, 1312, -1280, 8355, 20829, 4156, -549, 942, -431, 336, -122, 46,
	4, 31, -94, 341, -507, 1278, -1205, 7713, 20936, 4706, -677, 1011, -454, 344, -121, 45,
	3, 35, -102, 347, -506, 1238, -1119, 7082, 21001, 5274, -799, 1076, -473, 348, -118, 44,
	1, 40, -110, 350, -499, 1190, -1021, 6464, 21022, 5861, -914, 1136, -488, 350, -115, 43,*/
};

/*
static BKFrame table [][8] = {
	{   43, -115,  350, -488, 1136, -914, 5861,21022},
	{   44, -118,  348, -473, 1076, -799, 5274,21001},
	{   45, -121,  344, -454, 1011, -677, 4706,20936},
	{   46, -122,  336, -431,  942, -549, 4156,20829},
	{   47, -123,  327, -404,  868, -418, 3629,20679},
	{   47, -122,  316, -375,  792, -285, 3124,20488},
	{   47, -120,  303, -344,  714, -151, 2644,20256},
	{   46, -117,  289, -310,  634,  -17, 2188,19985},
	{   46, -114,  273, -275,  553,  117, 1758,19675},
	{   44, -108,  255, -237,  471,  247, 1356,19327},
	{   43, -103,  237, -199,  390,  373,  981,18944},
	{   42,  -98,  218, -160,  310,  495,  633,18527},
	{   40,  -91,  198, -121,  231,  611,  314,18078},
	{   38,  -84,  178,  -81,  153,  722,   22,17599},
	{   36,  -76,  157,  -43,   80,  824, -241,17092},
	{   34,  -68,  135,   -3,    8,  919, -476,16558},
	{   32,  -61,  115,   34,  -60, 1006, -683,16001},
	{   29,  -52,   94,   70, -123, 1083, -862,15422},
	{   27,  -44,   73,  106, -184, 1152,-1015,14824},
	{   25,  -36,   53,  139, -239, 1211,-1142,14210},
	{   22,  -27,   34,  170, -290, 1261,-1244,13582},
	{   20,  -20,   16,  199, -335, 1301,-1322,12942},
	{   18,  -12,   -3,  226, -375, 1331,-1376,12293},
	{   15,   -4,  -19,  250, -410, 1351,-1408,11638},
	{   13,    3,  -35,  272, -439, 1361,-1419,10979},
	{   11,    9,  -49,  292, -464, 1362,-1410,10319},
	{    9,   16,  -63,  309, -483, 1354,-1383, 9660},
	{    7,   22,  -75,  322, -496, 1337,-1339, 9005},
	{    6,   26,  -85,  333, -504, 1312,-1280, 8355},
	{    4,   31,  -94,  341, -507, 1278,-1205, 7713},
	{    3,   35, -102,  347, -506, 1238,-1119, 7082},
	{    1,   40, -110,  350, -499, 1190,-1021, 6464},
	{    0,   43, -115,  350, -488, 1136, -914, 5861}
};

static void bla ()
{
	for (BKInt i = 0; i < 32; i++) {
		for (BKInt j = 0; j < 8; j ++)
			printf ("%d, ", table [i][j]);
		for (BKInt j = 0; j < 8; j ++)
			printf ("%d, ", table [32 - i - 1][8 - j - 1]);
		printf ("\n");
	}
	printf ("\n");
}*/

/****
#include <math.h>
static void BKBufferCalcStepPhases (void)
{
	int const size = BK_STEP_WIDTH;
	double phasef [size];

	// step phase
	for (int phase = 0; phase < BK_STEP_UNIT; phase ++) {
		double sumf  = 0.0;
		BKInt  value = 0;
		BKInt  sum   = 0;
		
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
		
		printf ("{");

		for (BKInt i = 0; i < size; i ++)
			printf ("%6d, ", stepPhases [phase][i]);

		printf ("},\n");
	}
	printf ("\n");
}
****/

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
