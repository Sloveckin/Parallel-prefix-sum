#include "file_worker.h"

size_t next_power_of_two(size_t n)
{
	if ((n & (n - 1)) == 0)
	{
		return n;
	}

	size_t p = 1;
	while (p < n)
	{
		p <<= 1;
	}
	return p;
}

int read_n(FILE* file, size_t* src)
{
	int res = fscanf_s(file, "%zu", src);
	if (res != 1)
	{
		puts("Error while reading file.");
		return -1;
	}
	return 0;
}

int read_array(FILE* file, float** src, size_t n, int a)
{
	size_t start_n = n;
	n = next_power_of_two(n);

	*src = (float*)calloc(sizeof(float), n);
	if (*src == NULL)
	{
		puts("Not enough memory for array.");
		return -1;
	}

	for (size_t i = 0; i < start_n; i++)
	{
		float tmp;
		int res = fscanf(file, "%f", &tmp);
		if (res == 0)
		{
			puts("Error while reading array.");
			free(*src);
			return -1;
		}
		(*src)[i] = tmp;
	}
	return 0;
}

int write_array(FILE* file, float* src, size_t n)
{
	for (size_t i = 0; i < n; i++)
	{
		int res = fprintf(file, "%f ", src[i]);
		if (res < 0)
		{
			puts("Error while writing in file.");
			return -1;
		}
	}
	return 0;
}
