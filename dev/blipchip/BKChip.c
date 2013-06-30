#include <stdio.h>
#include "BKChip.h"

#define BK_CHIP_MAX_ATTRS 9
#define BK_CHIP_NUM_CHANNELS 2
typedef const struct BKChipAttribute
{
	BKInt attribute;
	BKInt multiplier;
} BKChipAttribute;

const static struct BKChipAttribute attributes [BK_CHIP_MAX_ATTRS] =
{
	[BK_CHIP_ATTR_VOLUME]               = {.attribute = BK_VOLUME,               .multiplier = BK_MAX_VOLUME / 255},
	[BK_CHIP_ATTR_PANNING]              = {.attribute = BK_PANNING,              .multiplier = BK_MAX_VOLUME / 127},
	[BK_CHIP_ATTR_NOTE]                 = {.attribute = BK_NOTE,                 .multiplier = BK_FINT20_UNIT},
	[BK_CHIP_ATTR_DUTY_CYCLE]           = {.attribute = BK_DUTY_CYCLE,           .multiplier = 1},
	[BK_CHIP_ATTR_EFFECT_VOLUME_SLIDE]  = {.attribute = BK_EFFECT_VOLUME_SLIDE,  .multiplier = 1},
	[BK_CHIP_ATTR_EFFECT_PANNING_SLIDE] = {.attribute = BK_EFFECT_PANNING_SLIDE, .multiplier = 1},
	[BK_CHIP_ATTR_EFFECT_PORTAMENTO]    = {.attribute = BK_EFFECT_PORTAMENTO,    .multiplier = 1},
	[BK_CHIP_ATTR_EFFECT_TREMOLO]       = {.attribute = BK_EFFECT_TREMOLO,       .multiplier = BK_MAX_VOLUME / 255},
	[BK_CHIP_ATTR_EFFECT_VIBRATO]       = {.attribute = BK_EFFECT_VIBRATO,       .multiplier = BK_FINT20_UNIT / 100},
};

extern BKInt BKChipInit (BKChip * chip, BKInt numChannels, BKInt sampleRate, BKChipTrackDescription const * trackDescriptions, BKInt numTracks)
{
	memset (chip, 0, sizeof (* chip));

	if (BKContextInit (& chip -> ctx, numChannels, sampleRate) != 0)
		return -1;

	BKContextGetAttr (& chip -> ctx, BK_SAMPLE_RATE, & chip -> sampleRate);
	BKContextGetAttr (& chip -> ctx, BK_NUM_CHANNELS, & chip -> numChannels);

	chip -> numTracks = numTracks;
	chip -> tracks    = malloc (chip -> numTracks * sizeof (BKTrack));

	if (chip -> tracks == NULL)
		return -1;
	
	for (BKInt i = 0; i < chip -> numTracks; i ++) {
		BKTrack * track = & chip -> tracks [i];
		BKChipTrackDescription * description = & trackDescriptions [i];

		BKTrackInit (track, description -> waveform);
		BKTrackSetAttr (track, BK_MASTER_VOLUME, description -> masterVolume);
		BKTrackSetAttr (track, BK_VOLUME, BK_MAX_VOLUME);

		BKTrackAttach (track, & chip -> ctx);
	}
	
	return 0;
}

void BKChipDispose (BKChip * chip)
{
	if (chip -> tracks) {
		for (BKInt i = 0; i < chip -> numTracks; i ++)
			BKTrackDispose (& chip -> tracks [i]);
		
		free (chip -> tracks);
	}

	BKContextDispose (& chip -> ctx);

	memset (chip, 0, sizeof (* chip));
}

BKInt BKChipPushCommand (BKChip * chip, BKChipCommand const * command, BKChipResponse * response)
{
	BKInt             trackIndex   = command -> track;
	BKInt             attrIndex    = command -> attr;
	BKInt             transAttribute;
	BKInt             makeResponse = 0;
	BKInt             values [2];
	BKTrack         * track;
	BKChipAttribute * attribute;

	memset (response, 0, sizeof (* response));

	if (trackIndex < 0 && trackIndex >= chip -> numTracks)
		return -1;

	if (attrIndex & BK_CHIP_ATTR_RESPONSE_FLAG) {
		attrIndex &= ~BK_CHIP_ATTR_RESPONSE_FLAG;
		makeResponse = 1;
	}

	if (attrIndex < 0 && attrIndex >= BK_CHIP_MAX_ATTRS)
		return -1;

	track          = & chip -> tracks [trackIndex];
	attribute      = & attributes [attrIndex];
	transAttribute = attribute -> attribute;

	values [0] = command -> values [0];
	values [1] = command -> values [1];

	switch (attrIndex) {
		case BK_CHIP_ATTR_VOLUME:
		case BK_CHIP_ATTR_PANNING:
		case BK_CHIP_ATTR_NOTE:
		case BK_CHIP_ATTR_DUTY_CYCLE: {
			// dont't multiply negative note values
			if (attrIndex != BK_CHIP_ATTR_NOTE || values [0] > 0)
				values [0] *= attribute -> multiplier;

			if (makeResponse) {
				response -> hasResponse = 1;
				BKTrackGetAttr (track, transAttribute, & response -> values [0]);
				response -> values [0] /= attribute -> multiplier;
			}
			else {
				BKTrackSetAttr (track, transAttribute, values [0]);
			}
			
			break;
		}
		case BK_CHIP_ATTR_EFFECT_VOLUME_SLIDE:
		case BK_CHIP_ATTR_EFFECT_PANNING_SLIDE:
		case BK_CHIP_ATTR_EFFECT_PORTAMENTO:
		case BK_CHIP_ATTR_EFFECT_TREMOLO:
		case BK_CHIP_ATTR_EFFECT_VIBRATO: {
			if (makeResponse) {
				response -> hasResponse = 1;
				BKTrackGetPtr (track, transAttribute, response -> values);
				response -> values [1] /= attribute -> multiplier;
			}
			else {
				values [1] *= attribute -> multiplier;
				BKTrackSetPtr (track, transAttribute, values);
			}

			break;
		}
		default: {
			return -1;
			break;
		}
	}

	return 0;
}
