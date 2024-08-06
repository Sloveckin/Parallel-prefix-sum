#include "open_cl_calculate.h"
#include "tile.h"
#include "user_input.h"

#include <stdio.h>
#include <string.h>

static void write_info_in_console(data_buffer* data)
{
	printf("Time: %g\t%g\n", data->kernel_time, data->all_time);
	printf("LOCAL_WORK_SIZE [%i, %i]\n", TILE_SIZE, 1);
}

int main(int argc, char** argv)
{
	int err = 0;
	float* src1 = NULL;
	FILE* input_file = NULL;
	FILE* output_file = NULL;

	user_input input;
	init_user_input(&input);
	err = read_input(argc, argv, &input);
	if (err)
	{
		return err;
	}

	if (err == 1)
	{
		return 0;
	}

	input_file = fopen(input.input_file, "r");
	if (input_file == NULL)
	{
		puts("Input file not found.");
		goto cleanup;
	}

	output_file = fopen(input.output_file, "w");
	if (output_file == NULL)
	{
		puts("Output file not found.");
		goto cleanup;
	}

	size_t n;
	err = read_n(input_file, &n);
	if (err)
		goto cleanup;

	err = read_array(input_file, &src1, n, TILE_SIZE);
	if (err)
		goto cleanup;

	size_t n1 = next_power_of_two(n);
	data_buffer data;
	init_data_buffer(&data);

	data.buffer1 = src1;

	err = calculate(&data, &input, n1);
	if (err)
		goto cleanup;

	write_info_in_console(&data);
	err = write_array(output_file, src1, n);
	if (err)
		goto cleanup;

cleanup:
	if (src1)
		free(src1);

	if (output_file)
		fclose(output_file);

	if (input_file)
		fclose(input_file);

	return err;
}
