#include <stdint.h>

enum block_loaction_t
{
	BLK_LOC_TL = 0,
	BLK_LOC_T,
	BLK_LOC_TR,
	BLK_LOC_R,
	BLK_LOC_BR,
	BLK_LOC_B,
	BLK_LOC_BL,
	BLK_LOC_CENTR
};

template<int CU_SIZE, block_loaction_t BLOCK_LOCATION>
void load_neighbors(uint8_t* neighbors, const uint8_t* block, const int pitch)
{
	const uint8_t* vert_neighbors = &block[-1];
	const uint8_t* horz_neighbors = &block[-pitch];
	const uint8_t* middle = &block[-pitch - 1];
	int num_vert = CU_SIZE * 2;
	int num_horz = CU_SIZE * 2;

	switch (BLOCK_LOCATION)
	{
	case BLK_LOC_TL:
		vert_neighbors = const_cast<uint8_t*>(&block[0]);
		horz_neighbors = const_cast<uint8_t*>(&block[0]);
		middle = const_cast<uint8_t*>(&block[0]);
		break;
	case BLK_LOC_T:
		horz_neighbors = const_cast<uint8_t*>(&block[0]);
		middle = const_cast<uint8_t*>(&block[-1]);
		break;
	case BLK_LOC_TR:
		horz_neighbors = const_cast<uint8_t*>(&block[0]);
		middle = const_cast<uint8_t*>(&block[-1]);
	case BLK_LOC_R:
		num_horz = CU_SIZE;
		break;
	case BLK_LOC_BR:
		num_vert = CU_SIZE;
		num_horz = CU_SIZE;
		break;
	case BLK_LOC_B:
		num_vert = CU_SIZE;
		break;
	case BLK_LOC_BL:
		num_vert = CU_SIZE;
		vert_neighbors = const_cast<uint8_t*>(&block[0]);
		middle = const_cast<uint8_t*>(&block[-pitch]);
		break;
	}

	for (int i = 0; i < num_vert; i++) {
		int y = num_vert - i - 1;
		neighbors[i] = vert_neighbors[i * pitch];
	}
	for (int i = 0; i < num_horz; i++) {
		neighbors[num_vert + i + 1] = horz_neighbors[num_vert + i + 1];
	}
	neighbors[num_vert] = middle[0];
}

typedef uint16_t(*measure_block_func_t)(uint8_t* block, const uint8_t* ref_block, int ref_pitch);

void make_intra_mode_16x16(uint8_t* block, uint8_t* neighbors, int pitch, int intra_mode);
int bruteforce_intra_16x16_sad(const uint8_t* block, const uint8_t* neighbors, const int pitch);
