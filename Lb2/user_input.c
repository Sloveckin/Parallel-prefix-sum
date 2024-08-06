#include "user_input.h"

void init_user_input(user_input* input)
{
	input->input_file = NULL;
	input->output_file = NULL;
	input->device_index = 0;
}

static void print_help()
{
	puts("< --input file_name > \\");
	puts("< --output file_name > \\");
	puts("< --realization realization > \\");
	puts("[ --device-type { dgpu | igpu | cpu | all } ]");
	puts("[ --device-index index ]");
}

static int is_valid_device_type(user_input* input, char* device_type)
{
	if (strcmp(device_type, "gpu") == 0)
	{
		input->device_type = DGPU;
	}
	else if (strcmp(device_type, "cpu") == 0)
	{
		input->device_type = CPU;
	}
	else if (strcmp(device_type, "all") == 0)
	{
		input->device_type = ALL;
	}
	else if (strcmp(device_type, "igpu") == 0)
	{
		input->device_type = IGPU;
	}
	else if (strcmp(device_type, "dgpu") == 0)
	{
		input->device_type = DGPU;
	}
	else
	{
		puts("This type of devices not supported.");
		return -1;
	}
	return 0;
}

int read_input(int argc, char* argv[], user_input* input)
{
	if (argc == 1)
	{
		printf("Invalid count of arguemnts");
		print_help();
		return -1;
	}

	if (argc == 2 && strcmp(argv[1], "--help") == 0)
	{
		print_help();
		return 1;
	}

	for (size_t i = 1; i < argc; i += 2)
	{
		if (i + 1 == argc)
		{
			printf("No value for key %s.\n", argv[i]);
			return -1;
		}

		if (strcmp(argv[i], "--input") == 0)
		{
			input->input_file = argv[i + 1];
		}
		else if (strcmp(argv[i], "--output") == 0)
		{
			input->output_file = argv[i + 1];
		}
		else if (strcmp(argv[i], "--device-index") == 0)
		{
			input->device_index = atoi(argv[i + 1]);
		}
		else if (strcmp(argv[i], "--device-type") == 0)
		{
			if (is_valid_device_type(input, argv[i + 1]))
			{
				return -1;
			}
		}
		else
		{
			printf("Unkown flag: %s\n", argv[i]);
			return -1;
		}
	}

	if (input->input_file == NULL)
	{
		puts("No input file.");
		return -1;
	}

	if (input->output_file == NULL)
	{
		puts("No output file.");
		return -1;
	}

	return 0;
}