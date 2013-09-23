#include <math.h>
#include <stdio.h>
#include "BKBuffer.h"

static BKFrame stepPhases [BK_STEP_UNIT][BK_STEP_WIDTH];

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

int main (int argc, char const * argv [])
{
	BKBufferCalcStepPhases ();

	return 0;
}
