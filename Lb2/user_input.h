#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
	DGPU,
	IGPU,
	CPU,
	ALL,
} device_type_t;

typedef struct
{
	char* input_file;
	char* output_file;
	device_type_t device_type;
	size_t device_index;
} user_input;

int read_input(int, char*[], user_input*);
void init_user_input(user_input* input);
void print_help();
