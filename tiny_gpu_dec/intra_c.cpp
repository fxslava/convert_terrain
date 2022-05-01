#include "intra_c.h"
#define _USE_MATH_DEFINES
#include  <cmath>

#include "sad_c.h"

constexpr float EPSILON = 0.0001f;

template<int CU_SIZE>
uint8_t sample_neighbor(int x, int y, const uint8_t* neighbors, const float angle)
{
	int i0 = x + int(y * tanf(M_PI_2 - angle));
	int i1 = y - int(x * tanf(angle));

	return (i0 < 0 || i0 > CU_SIZE * 2 - 1) ? neighbors[CU_SIZE * 2 - i1] : neighbors[CU_SIZE * 2 + i0];
}

template<int CU_SIZE>
void make_intra_mode(uint8_t *block, const uint8_t *neighbors, const float angle) {
	for (int x = 0; x < CU_SIZE; x++) {
		for (int y = 0; y < CU_SIZE; y++) {
			block[x + y * CU_SIZE] = sample_neighbor<CU_SIZE>(x, y, neighbors, angle);
		}
	}
}

template<int CU_SIZE>
void make_intra_mode(uint8_t* block, const uint8_t* neighbors, int pitch, const float angle) {
	for (int x = 0; x < CU_SIZE; x++) {
		for (int y = 0; y < CU_SIZE; y++) {
			block[x + y * pitch] = sample_neighbor<CU_SIZE>(x, y, neighbors, angle);
		}
	}
}

template<int INTRA_MODES_NUM>
float intra_mode_to_angle(int intra_mode) {
	return intra_mode * static_cast<float>(M_PI / INTRA_MODES_NUM) - M_PI_4;
}

template<int CU_SIZE, int INTRA_MODES_NUM>
int bruteforce_intra_modes(const uint8_t* block, const uint8_t* neighbors, const int pitch, measure_block_func_t measure_block)
{
	uint8_t pred_block[CU_SIZE * CU_SIZE];

	int best_intra_mode = -1;
	uint16_t best_intra_cost = UINT16_MAX;

	for (int intra_mode = 0; intra_mode < INTRA_MODES_NUM; intra_mode++)
	{
		const float angle = intra_mode_to_angle<INTRA_MODES_NUM>(intra_mode);
		make_intra_mode<CU_SIZE>(pred_block, neighbors, angle);

		uint16_t intra_cost = measure_block(pred_block, block, pitch);

		if (intra_cost < best_intra_cost)
		{
			best_intra_cost = intra_cost;
			best_intra_mode = intra_mode;
		}
	}

	return best_intra_mode;
}

void make_intra_mode_16x16(uint8_t* block, uint8_t* neighbors, int pitch, int intra_mode) {
	const float angle = intra_mode_to_angle<32>(intra_mode);
	make_intra_mode<16>(block, neighbors, pitch, angle);
}

int bruteforce_intra_16x16_sad(const uint8_t* block, const uint8_t* neighbors, const int pitch) {
	return bruteforce_intra_modes<16, 32>(block, neighbors, pitch, measure_sad_16x16);
}
