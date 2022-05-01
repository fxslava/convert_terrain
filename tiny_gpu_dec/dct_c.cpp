#include "dct_c.h"
#define _USE_MATH_DEFINES
#include  <cmath>
#include  <vector>

inline int16_t clamp_16bit(int val) {
	return std::min(std::min(int(val), int(INT16_MAX)), int(INT16_MIN));
}

static void forwardTransform(float vec[], float temp[], size_t len) {
	if (len == 1)
		return;
	size_t halfLen = len / 2;
	for (size_t i = 0; i < halfLen; i++) {
		float x = vec[i];
		float y = vec[len - 1 - i];
		temp[i] = x + y;
		temp[i + halfLen] = (x - y) / (std::cos((i + 0.5) * M_PI / len) * 2);
	}
	forwardTransform(temp, vec, halfLen);
	forwardTransform(&temp[halfLen], vec, halfLen);
	for (size_t i = 0; i < halfLen - 1; i++) {
		vec[i * 2 + 0] = temp[i];
		vec[i * 2 + 1] = temp[i + halfLen] + temp[i + halfLen + 1];
	}
	vec[len - 2] = temp[halfLen - 1];
	vec[len - 1] = temp[len - 1];
}

template<int len>
void dct_N(float (&vec)[len]) {
	float temp[len];
	forwardTransform(vec, temp, len);
}

template<int len>
void dct_Ni(uint16_t *dst, uint8_t *src) {
	float vecf[len];
	for (int i = 0; i < len; i++) {
		vecf[i] = src[i];
	}
	dct_N<len>(vecf);
	for (int i = 0; i < len; i++) {
		dst[i] = clamp_16bit(vecf[i]);
	}
}

void dct256i(uint16_t* dst, uint8_t* src) {
	dct_Ni<256>(dst, src);
}