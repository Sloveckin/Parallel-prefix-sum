#pragma once

#include "file_worker.h"
#include "tile.h"
#include "user_input.h"

#include <math.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

#define CL_TARGET_OPENCL_VERSION 120
#ifdef __APPLE__
#	include <OpenCL/cl.h>
#else
#	include <CL/cl.h>
#endif

typedef struct
{
	float* buffer1;
	cl_double kernel_time;
	cl_double all_time;
} data_buffer;

void init_data_buffer(data_buffer* result);
int calculate(data_buffer* data, user_input* input, size_t n);