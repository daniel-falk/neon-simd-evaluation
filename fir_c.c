#include <stdlib.h>
#include <stddef.h>

/*
 * Make a finite impule response filter pass over data
 *
 * weights are assumed to sum up to 256
 *
 * Modifies result and rlen
 *
 */
void fir_c(const unsigned char *data, size_t dlen, const unsigned char *weights, size_t wlen, unsigned char **result, size_t *rlen) {

	*rlen   = (dlen - wlen);
	*result = malloc(*rlen);

	unsigned int tmp;
	size_t       i, j;

	for (i = 0; i < *rlen; i ++) {
		tmp = 0;
		for (j = 0; j < wlen; j++) {
			tmp += data[i + j] * weights[j];
		}
		(*result)[i] = (char)(tmp >> 8); // Divide by the sum of weights
	}
}


