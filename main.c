#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include <arm_neon.h>

#define DATA_SIZE    1e6

typedef void (*fncn_t)(const unsigned char *, size_t, const unsigned char *, size_t, unsigned char **, size_t *);


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


/*
 * Write the low-pass data and the input data to file
 */
void write_file(const unsigned char *data, size_t dlen, const unsigned char *res, size_t rlen, const char *fpath) {

	FILE *fp = fopen(fpath, "w");

	size_t i;
	long   j;
	for (i = 0, j = rlen - dlen; i < dlen; i++, j++) {
		if (j >= 0) {
			fprintf(fp, "%u - %u\n", data[i], res[j]);
		} else {
			fprintf(fp, "%u -\n", data[i]);
		}
	}

	fclose(fp);
}


/*
 * Return timediff as nanoseconds per sample
 */
long time_per_sample(struct timespec start, struct timespec stop, size_t samples) {
	return (((long long)stop.tv_sec - start.tv_sec) * 1e9 +
		(stop.tv_nsec - start.tv_nsec))
		/ samples;
}


int main(int argc, char *argv[]) {

	if (argc != 2) {
		printf("Usage: %s <output_file_path>\n", argv[0]);
		exit(1);
	}
	int  flen = strlen(argv[1]);
	char *name[] = {"plain_c", "neon"};
	char *fname[2];
	fname[0] = malloc(flen + strlen(name[0]) + 2);
	fname[1] = malloc(flen + strlen(name[1]) + 2);
	sprintf(fname[0], "%s.%s", argv[1], name[0]);
	sprintf(fname[1], "%s.%s", argv[1], name[1]);

	static fncn_t fncn[] = {fir_c, fir_neon};

	// Saw tooth data
	unsigned char *data = malloc(DATA_SIZE);
	size_t        i;
	for (i = 0; i < DATA_SIZE; i++) {
		data[i] = i % 256;
	}

	// Weights for FIR filter
	const unsigned char weights[] = {32, 32, 32, 32, 32, 32, 32, 32}; // Sum is 256

	// Do FIR!
	for (i = 0; i < 2; i++) {
		printf("Using method: %s\n", name[i]);

		struct timespec t_prc_start,
				t_mon_start,
				t_prc_stop,
				t_mon_stop;

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_prc_start);
		clock_gettime(CLOCK_MONOTONIC, &t_mon_start);
		
		size_t rlen;
		unsigned char   *result;

		(fncn[i])(data, DATA_SIZE, weights, sizeof(weights) / sizeof(weights[0]), &result, &rlen);

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t_prc_stop);
		clock_gettime(CLOCK_MONOTONIC, &t_mon_stop);

		printf("Monotonic time per sample: %ld\n", time_per_sample(t_mon_start, t_mon_stop, DATA_SIZE));
		printf("Process time per sample:   %ld\n", time_per_sample(t_prc_start, t_prc_stop, DATA_SIZE));

		// Print results to file
		write_file(data, DATA_SIZE, result, rlen, fname[i]);

		free(result);
	}

	free(data);
}
