/*
 * \file: random.c
 * \brief: Created by hushouguo at Nov 06 2014 02:13:20
 */

#include "zebra.h"

static unsigned int seed = 0;
#ifdef LINUX
__attribute__((constructor)) static void random_init()
{
	seed = time(0);
}
#endif
int random_int()
{
	return rand_r(&seed);
}
float random_float()
{
	return (float)random_int()/(double)(RAND_MAX);
}
int random_between(int min, int max)
{
	seed = 214013 * seed + 2531011;

	if (min < max)
	{
		return min + (seed ^ seed >> 15) % (max - min + 1);
	}
	else
	{
		return max + (seed ^ seed >> 15) % (min - max + 1);
	}
}
float random_float_between(float min, float max)
{
	seed = 214013 * seed + 2531011;

	if (min < max)
	{
		return min + (seed >> 16) * (1.0f/65535.0f) * (max - min);
	}
	else
	{
		return max + (seed >> 16) * (1.0f/65535.0f) * (min - max);
	}
}

