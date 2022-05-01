#pragma once
#include <cstdint>

enum color_channel_t
{
	CHANNEL_RED = 0,
	CHANNEL_GREEN,
	CHANNEL_BLUE,
	CHANNEL_NUM
};

void intra_pred_image(uint8_t* dst[CHANNEL_NUM], uint8_t* src[CHANNEL_NUM], const uint8_t* src_luma, int pitch, int width, int height);