#include <math.h>
#include <stdio.h>
#include "BlipKit.h"
#include "BKTone.h"

static void BKCalcTonePeriods (void)
{
	BKUInt const baseFreq = 440;
	double freq;
	BKFUInt20 period;

	printf ("static BKUInt const tonePeriods [(BK_MAX_PIANO_TONE - BK_MIN_PIANO_TONE) + 1] =\n");
	printf ("{\n\t");

	for (BKInt i = BK_MIN_PIANO_TONE, c = 0; i <= BK_MAX_PIANO_TONE; i ++, c ++) {
		freq = baseFreq * pow (2.0, ((double) i / 12.0));

		// frequency in seconds
		// so it can be multiplied by the samplerate
		period = 1.0 / freq * BK_FINT20_UNIT;
		//tonePeriods [i] = period;

		//BKUInt eff = (double) 48000 / (((double) period * 48000) / BK_FINT20_UNIT);
		//printf ("%f, %2d %f\n", freq, i, (1.0 - (double) freq / eff) * 100);

		if (c && c % 12 == 0)
			printf ("\n\t");

		printf ("%5d, ", period);
	}

	printf ("\n};\n\n");
}

static void BKCalcLog2Periods (void)
{
	BKUInt period;

	printf ("static BKUInt const log2Periods [(BK_MAX_PIANO_TONE - BK_MIN_PIANO_TONE) + 1] =\n");
	printf ("{\n\t");

	for (BKInt i = BK_MIN_PIANO_TONE, c = 0; i <= BK_MAX_PIANO_TONE; i ++, c ++) {
		// frequency in seconds
		// so it can be multiplied by the samplerate
		period = pow (2.0, ((double) i / 12.0)) * BK_FINT20_UNIT;
		//tonePeriods [i] = period;

		//printf ("%d %d\n", i, period);

		//BKUInt eff = (double) 48000 / (((double) period * 48000) / BK_FINT20_UNIT);
		//printf ("%f, %2d %f\n", freq, i, (1.0 - (double) freq / eff) * 100);

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
