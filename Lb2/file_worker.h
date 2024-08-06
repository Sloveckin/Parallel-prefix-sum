#pragma once

#include <stdio.h>
#include <stdlib.h>

int read_n(FILE* file, size_t* n);
int read_array(FILE* file, float** src, size_t n, int a);
int write_array(FILE* file, float* src, size_t n);
size_t next_power_of_two(size_t n);
