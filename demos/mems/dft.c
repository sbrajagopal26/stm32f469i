/*
 * Discrete Fourier Transform
 *
 * To understand FFTs you need to understand DFTs and
 * this code applies the DFT using correlation to the
 * samples generated by data gen.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/*
 * This is a bucket of samples, is is 'n' samples long.
 * And it represents collecting them at a rate 'r' per
 * second. So a total of n / r seconds worth of signal.
 */
typedef struct {
	float	*data;
	int		n;
	int		r;
} sample_buffer;

float cos_basis(float, float);
float sin_basis(float, float);
void add_freq(sample_buffer *, float, float);

/*
 * basis function
 *
 * This is a basis function (cos)
 *	f is the frequency
 *	t is the time.
 */
float
cos_basis(float f, float t)
{
	return cos(2 * M_PI * f * t);
}

/*
 * This is the sin basis function
 */
float
sin_basis(float f, float t)
{
	return sin(2 * M_PI * f * t);
}

/*
 * add_freq( ... )
 *
 * Add in a signal at this frequency and amplitude into
 * the sample buffer. 
 *
 * Scaled to a 12 bit value (0 == 2048, -1 = 0, +1 = 2048)
 */
void
add_freq(sample_buffer *s, float f, float a)
{
	int	i;
	float *buf = s->data;

	/*
	 * n is samples
	 * r is rate (samples per second)
	 * f is frequency (cycles per second)
	 * what span is (n / r) seconds / f = cyles /n is cycles per sample?
	 */
	for (i = 0; i < s->n; i++ ) {
		*buf += a * cos(2 * M_PI * f * i / s->r);
		buf++;
	}
}

void add_triangle(sample_buffer *, float, float);
void add_square(sample_buffer *, float, float);

/*
 * add_triangle( ... )
 *
 * Add a triangle wave to the sample buffer.
 *
 * seconds per sample * sample # = time
 * period is seconds per period.
 * seconds / period - int seconds = percentage of period.
 */
void
add_triangle(sample_buffer *s, float f, float a)
{
	int i;
	float period = (float) s->r / f;
	float t;
	float *buf = s->data;

	for (i = 0; i < s->n; i++) {
		t = (float) i / period;
		t = t - (int) t;
		*buf += a * t;
		buf++;
	}
}

void
add_square(sample_buffer *s, float f, float a)
{
	int i;
	int per = s->r / (2 * f);
	float *buf = s->data;

	printf("Square wave period is %d\n", per);
	for (i = 0; i < s->n; i++) {
		*buf += ((i / per) & 1) * a;
		buf++;
	}
}

#define SAMP_SIZE 1024
#define SAMP_RATE 8192

float r_x[SAMP_SIZE / 2];
float i_x[SAMP_SIZE / 2];
float res_dft[SAMP_SIZE / 2];
int ri_len = SAMP_SIZE / 2;
float s_data[SAMP_SIZE];

void gen_data(int opt);

void
gen_data(int opt)
{
	sample_buffer	sb;
	float f, data_min, data_max;
	int	i, k;

	sb.data = s_data;
	sb.n = SAMP_SIZE;
	sb.r = SAMP_RATE;	/* 8 khz sample rate */

	/* clear sample data */
	memset(s_data, 0, sizeof(s_data));

	switch (opt) {
		case 3:
			/* waves smeared over bin size */
			for (i = 0; i < 9; i++) {
				add_freq(&sb, 32 * i + 64 + i, 1.5);
			}
			break;
		case 1:
			/* triangle wave */
			add_triangle(&sb, 100.0, 1.5);
			break;
		case 2:
			/* square wave */
			add_square(&sb, 100.0, 1.5);
			break;
		default:
			/* a C chord */
			add_freq(&sb, 130.81, 1.0);
			add_freq(&sb, 164.81, 1.0);
			add_freq(&sb, 196, 1.0);
			break;
	}
	data_min = data_max = 0;
	for (i = 0; i < sb.n; i++) {
		data_min = (*(sb.data + i) < data_min) ? *(sb.data+i) : data_min;
		data_max = (*(sb.data + i) > data_max) ? *(sb.data+i) : data_max;
	}
#ifdef DEBUG
	scale = 60.0 / (data_max - data_min);
	printf("Min/Max - %f, %f\n", data_min, data_max);
	printf("Scale/Offset - %f, %f\n", scale, offset);
#endif
	
	printf("Generating data ... \n");

	for (i = 1; i < SAMP_SIZE/2; i++) {
		float t;

		f = (float) (4096 * i) / 4096.0;
		r_x[i] = 0;
		i_x[i] = 0;
		for (k = 0; k < SAMP_SIZE; k++) {
			t = (float) k / (float) SAMP_RATE;
			r_x[i] += s_data[k] * cos_basis(f, t);
			i_x[i] += s_data[k] * sin_basis(f, t);
		}
		r_x[i] = (2 * r_x[i]) / (SAMP_SIZE / 2);
		i_x[i] = (-2 * i_x[i]) / (SAMP_SIZE / 2);
		printf("\r%d of %d ... ", i, SAMP_SIZE/2);
		fflush(stdout);
	}
	printf("Done.\n");
}
