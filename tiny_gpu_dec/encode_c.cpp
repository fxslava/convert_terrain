#include "encode_c.h"

#include <cassert>
#include <memory>

#include "intra_c.h"

template<int CU_SIZE>
void load_neighbors(uint8_t* neighbors, const uint8_t* block, const int pitch, const bool left, const bool top, const bool right, const bool bottom)
{
	memset(neighbors, 0, CU_SIZE * 4 + 1);

	if (top && left)                   load_neighbors<CU_SIZE, BLK_LOC_TL   >(neighbors, block, pitch);
	else if (top && !left && !right)   load_neighbors<CU_SIZE, BLK_LOC_T    >(neighbors, block, pitch);
	else if (top && right)             load_neighbors<CU_SIZE, BLK_LOC_TR   >(neighbors, block, pitch);
	else if (right && !top && !bottom) load_neighbors<CU_SIZE, BLK_LOC_R    >(neighbors, block, pitch);
	else if (right && bottom)          load_neighbors<CU_SIZE, BLK_LOC_BR   >(neighbors, block, pitch);
	else if (bottom && !top && !left)  load_neighbors<CU_SIZE, BLK_LOC_B    >(neighbors, block, pitch);
	else if (bottom && left)           load_neighbors<CU_SIZE, BLK_LOC_BL   >(neighbors, block, pitch);
	else                               load_neighbors<CU_SIZE, BLK_LOC_CENTR>(neighbors, block, pitch);
}

void intra_pred_image(uint8_t* dst[CHANNEL_NUM], uint8_t* src[CHANNEL_NUM], const uint8_t* src_luma, int pitch, int width, int height)
{
	constexpr int CU_SIZE = 16;

	assert(width % CU_SIZE == 0);
	assert(height % CU_SIZE == 0);
	assert(width > CU_SIZE * 2);
	assert(height > CU_SIZE * 2);

	for (int y = 0; y < height; y += CU_SIZE)	{
		for (int x = 0; x < width; x += CU_SIZE) 
		{
			uint8_t neighbors[CU_SIZE * 4 + 1];

			const uint8_t* src_block = &src_luma[x + y * pitch];

			const bool left   = (x == 0);
			const bool top    = (y == 0);
			const bool right  = (x == width  - CU_SIZE);
			const bool bottom = (y == height - CU_SIZE);

			load_neighbors<CU_SIZE>(neighbors, src_block, pitch, left, top, right, bottom);
			const int best_intra_mode = bruteforce_intra_16x16_sad(src_block, neighbors, pitch);

			for (int i = 0; i < CHANNEL_NUM; i++) {
				uint8_t* dst_block = &dst[i][x + y * pitch];
				load_neighbors<CU_SIZE>(neighbors, &src[i][x + y * pitch], pitch, left, top, right, bottom);
				make_intra_mode_16x16(dst_block, neighbors, pitch, best_intra_mode);
			}
		}
	}
}