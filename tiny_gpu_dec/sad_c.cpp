#include "sad_c.h"
#include <cmath>

template<int CU_SIZE>
uint16_t measure_sad(uint8_t* block, const uint8_t* ref_block, int ref_pitch)
{
	uint16_t sad = 0;
	for (int i = 0; i < CU_SIZE; i++) {
		for (int j = 0; j < CU_SIZE; j++) {
			uint16_t ref = ref_block[i + j * ref_pitch];
			uint16_t org = block[i + j * CU_SIZE];
			sad += abs(ref - org);
		}
	}
	return sad;
}

uint16_t measure_sad_16x16(uint8_t* block, const uint8_t* ref_block, int ref_pitch)
{
	return measure_sad<16>(block, ref_block, ref_pitch);
}
