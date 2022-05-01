#pragma once
#include <cstdint>

uint16_t measure_sad_16x16(uint8_t* block, const uint8_t* ref_block, int ref_pitch);