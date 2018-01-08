#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <arm_neon.h>

/*
 * Make a finite impule response filter pass over data. Uses simd (neon intrinsics) to accelerate math.
 *
 * weights are assumed to sum up to 256
 *
 * Modifies result and rlen
 *
 */
void fir_neon(const unsigned char *data, size_t dlen, const unsigned char *weights, size_t wlen, unsigned char **result, size_t *rlen) {

	assert(wlen % 8 == 0);

	*rlen = (dlen - wlen);
	*result = malloc(*rlen);

	uint8x8_t  d, w; // Registers in SIMD module to load data and weights
	uint16x8_t tmp;  // Register in SIMD module to store intermediate 16-bit results

	size_t i, j;
	for (i = 0; i < *rlen; i ++) {
		tmp = vdupq_n_u16(0);
		for (j = 0; j < wlen; j += 8) {
			d   = vld1_u8(&data[i + j]);
			w   = vld1_u8(&weights[j]);
			tmp = vmlal_u8(tmp, d, w);
		}
		(*result)[i] = (unsigned char)((vgetq_lane_u16(tmp, 0) +
			        vgetq_lane_u16(tmp, 1) +
			        vgetq_lane_u16(tmp, 2) +
			        vgetq_lane_u16(tmp, 3) +
			        vgetq_lane_u16(tmp, 4) +
			        vgetq_lane_u16(tmp, 5) +
			        vgetq_lane_u16(tmp, 6) +
			        vgetq_lane_u16(tmp, 7)) >> 8);
	}
}

