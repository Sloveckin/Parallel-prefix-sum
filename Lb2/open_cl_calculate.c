#include "open_cl_calculate.h"

#define BUFFER_SIZE 128

#define RETURN_ERR(message) \
	do                      \
	{                       \
		if (err)            \
		{                   \
			puts(message);  \
			return err;     \
		}                   \
	} while (0)

#define ADD_KERNEL_TIME                                  \
	do                                                   \
	{                                                    \
		err = get_work_time(ev, &start_time, &end_time); \
		if (err)                                         \
		{                                                \
			clReleaseEvent(ev);                          \
			goto cleanup;                                \
		}                                                \
		time = get_time(start_time, end_time);           \
		clReleaseEvent(ev);                              \
		data->all_time += time;                          \
		data->kernel_time += time;                       \
	} while (0)

#define WRITE_BUFFER(buffer, src, len, message)                                                            \
	do                                                                                                     \
	{                                                                                                      \
		err = clEnqueueWriteBuffer(queue, buffer, CL_FALSE, 0, sizeof(cl_float) * len, src, 0, NULL, &ev); \
		if (err)                                                                                           \
		{                                                                                                  \
			puts(message);                                                                                 \
			clReleaseEvent(ev);                                                                            \
			goto cleanup;                                                                                  \
		}                                                                                                  \
		err = get_work_time(ev, &start_time, &end_time);                                                   \
		if (err)                                                                                           \
		{                                                                                                  \
			clReleaseEvent(ev);                                                                            \
			goto cleanup;                                                                                  \
		}                                                                                                  \
		time = get_time(start_time, end_time);                                                             \
		clReleaseEvent(ev);                                                                                \
		data->all_time += time;                                                                            \
	} while (0)

#define READ_BUFFER(buffer, src, len)                                                                    \
	do                                                                                                   \
	{                                                                                                    \
		err = clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, sizeof(cl_float) * len, src, 0, NULL, &ev); \
		if (err)                                                                                         \
		{                                                                                                \
			puts("clEnqueueReadBuffer failed.");                                                         \
			clReleaseEvent(ev);                                                                          \
			goto cleanup;                                                                                \
		}                                                                                                \
		err = get_work_time(ev, &start_time, &end_time);                                                 \
		if (err)                                                                                         \
		{                                                                                                \
			clReleaseEvent(ev);                                                                          \
			goto cleanup;                                                                                \
		}                                                                                                \
		time = get_time(start_time, end_time);                                                           \
		clReleaseEvent(ev);                                                                              \
		data->all_time += time;                                                                          \
	} while (0)

#define SET_KERNEL_ARG(kernel, index, buffer, type, message)        \
	do                                                              \
	{                                                               \
		err = clSetKernelArg(kernel, index, sizeof(type), &buffer); \
		if (err)                                                    \
		{                                                           \
			puts(message);                                          \
			goto cleanup;                                           \
		}                                                           \
	} while (0)

#define RUN_KERNEL1(kernel, N, n)                                                                                \
	do                                                                                                           \
	{                                                                                                            \
		global_work_size = N;                                                                                    \
		local_work_size = n;                                                                                     \
		err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, &ev); \
		if (err)                                                                                                 \
		{                                                                                                        \
			puts("clEnqueueNDRangeKernel failed.");                                                              \
			clReleaseEvent(ev);                                                                                  \
			goto cleanup;                                                                                        \
		}                                                                                                        \
		ADD_KERNEL_TIME;                                                                                         \
	} while (0)

#define RUN_KERNEL2(kernel, N)                                                                       \
	do                                                                                               \
	{                                                                                                \
		global_work_size = N;                                                                        \
		err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, &ev); \
		if (err)                                                                                     \
		{                                                                                            \
			puts("clEnqueueNDRangeKernel failed.");                                                  \
			clReleaseEvent(ev);                                                                      \
			goto cleanup;                                                                            \
		}                                                                                            \
		ADD_KERNEL_TIME;                                                                             \
	} while (0)

#define HANDLE_ERRROR(message) \
	do                         \
	{                          \
		if (err)               \
		{                      \
			puts(message);     \
			goto cleanup;      \
		}                      \
	} while (0)

void init_data_buffer(data_buffer* result)
{
	result->buffer1 = NULL;
	result->all_time = 0.0f;
	result->kernel_time = 0.0f;
}

static int get_work_time(cl_event cur_event, cl_ulong* start_time, cl_ulong* end_time)
{
	int err = 0;
	err = clWaitForEvents(1, &cur_event);
	if (err)
		return err;
	err = clGetEventProfilingInfo(cur_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), start_time, NULL);
	if (err)
		return err;
	err = clGetEventProfilingInfo(cur_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), end_time, NULL);
	if (err)
		return err;
	return 0;
}

static int print_device_info(cl_device_id device)
{
	char platform_name[BUFFER_SIZE];
	char device_name[BUFFER_SIZE];
	cl_platform_id platform;
	int err;

	err = clGetDeviceInfo(device, CL_DEVICE_NAME, BUFFER_SIZE, device_name, NULL);
	RETURN_ERR("Error while getting information from device.");

	err = clGetDeviceInfo(device, CL_DEVICE_PLATFORM, sizeof(cl_platform_id), &platform, NULL);
	RETURN_ERR("Error while getting information from device.");

	err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, BUFFER_SIZE, platform_name, NULL);
	RETURN_ERR("Error while getting information from platform.");

	printf("Device: %s\tPlatform: %s\n", device_name, platform_name);
	return 0;
}

static int isIGPU(cl_device_id device)
{
	cl_bool res;
	clGetDeviceInfo(device, CL_DEVICE_HOST_UNIFIED_MEMORY, sizeof(cl_bool), &res, NULL);
	if (res == CL_TRUE)
	{
		return 1;
	}
	return 0;
}

static int isDGPU(cl_device_id device)
{
	int res = isIGPU(device);
	if (res == 0)
	{
		return 1;
	}
	return 0;
}

static int get_cl_device_type(device_type_t type)
{
	if (type == ALL)
	{
		return CL_DEVICE_TYPE_ALL;
	}
	else if (type == IGPU || type == DGPU)
	{
		return CL_DEVICE_TYPE_GPU;
	}
	return CL_DEVICE_TYPE_CPU;
}

static device_type_t get_device_type(cl_device_id device)
{
	cl_device_type type1;
	clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &type1, NULL);
	if (type1 == CL_DEVICE_TYPE_GPU)
	{
		if (isDGPU(device))
		{
			return DGPU;
		}
		return IGPU;
	}
	return CPU;
}

int compare(const void* a, const void* b)
{
	cl_device_id a1 = *(const cl_device_id*)a;
	cl_device_id b1 = *(const cl_device_id*)b;

	device_type_t type1 = get_device_type(a1);
	device_type_t type2 = get_device_type(b1);

	if (type1 < type2)
		return -1;
	else if (type1 > type2)
		return 1;
	return 0;
}

static int chose_device(cl_device_id* device, cl_platform_id* platforms, size_t platform_count, user_input* input)
{
	cl_device_id all_devices[100];
	cl_device_id buffer[10];
	size_t pos = 0;

	for (size_t i = 0; i < platform_count; i++)
	{
		cl_int res;
		cl_uint num;

		int type = get_cl_device_type(input->device_type);

		res = clGetDeviceIDs(platforms[i], type, 10, buffer, &num);
		if (res == CL_DEVICE_NOT_FOUND)
		{
			continue;
		}
		if (res)
		{
			puts("clGetDeviceIDs failed.");
			return res;
		}
		if (input->device_type == DGPU || input->device_type == IGPU)
		{
			for (size_t j = 0; j < num; j++)
			{
				device_type_t type = get_device_type(buffer[j]);
				if ((type == DGPU && input->device_type == DGPU) || (type == IGPU && input->device_type == IGPU))
				{
					all_devices[pos] = buffer[j];
					pos++;
				}
			}
		}
		else
		{
			memcpy(&all_devices[pos], buffer, num * sizeof(cl_device_id));
			pos += num;
		}
	}

	if (pos == 0)
	{
		puts("Device not found.");
		return -1;
	}

	if (input->device_index >= pos)
	{
		*device = all_devices[0];
	}
	else
	{
		if (input->device_type != ALL)
		{
			*device = all_devices[input->device_index];
		}
		else
		{
			qsort(all_devices, pos, sizeof(cl_device_id), compare);
			*device = all_devices[input->device_index];
		}
	}
	return 0;
}

static int find_device(cl_device_id* device, user_input* input)
{
	cl_uint num_platforms;
	cl_int res;

	res = clGetPlatformIDs(0, NULL, &num_platforms);
	if (res)
	{
		puts("clGetPlatform failed.");
		return res;
	}

	cl_platform_id* platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_platforms);
	if (platforms == NULL)
	{
		puts("Not enough memory.");
		return -1;
	}

	res = clGetPlatformIDs(num_platforms, platforms, NULL);
	if (res)
	{
		puts("clGetPlatform failed.");
		free(platforms);
		return res;
	}

	res = chose_device(device, platforms, num_platforms, input);
	free(platforms);
	return res;
}

static cl_double get_time(cl_ulong start, cl_ulong end)
{
	return (end - start) / 1000000.0;
}

static int
	rec_run(data_buffer* data, cl_context context, cl_program program, cl_mem* buffers, cl_command_queue queue, cl_kernel* kernels, size_t n, float* buffer, int index)
{
	cl_event ev = NULL;
	cl_ulong start_time = 0;
	cl_ulong end_time = 0;
	double time = 0;
	int err = 0;
	float* partition_sum = NULL;

	size_t global_work_size = 0;
	size_t local_work_size = 0;

	if (n <= TILE_SIZE)
	{
		WRITE_BUFFER(buffers[2], buffer, n, "clEnqueueWriteBuffer failed (buffer3).");

		SET_KERNEL_ARG(kernels[2], 0, buffers[2], cl_mem, "clSetKernelArg failed.");
		SET_KERNEL_ARG(kernels[2], 1, n, cl_uint, "clSetKernelArg failed.");
		RUN_KERNEL1(kernels[2], n, n);
		READ_BUFFER(buffers[2], buffer, n);
		return 0;
	}
	size_t part_n = n / TILE_SIZE + 1;

	partition_sum = (float*)calloc(part_n, sizeof(float));
	if (partition_sum == NULL)
	{
		puts("Not enough memory.");
		return -1;
	}

	SET_KERNEL_ARG(kernels[0], 0, buffers[0], cl_mem, "clSetKernelArg failed. First kernel. First argument,");
	SET_KERNEL_ARG(kernels[0], 1, buffers[1], cl_mem, "clSetKernelArg failed. First kernel. Second arguement.");

	WRITE_BUFFER(buffers[0], buffer, n, "clEnqueueWriteBuffer failed. First kernel. First arguemnt.");
	WRITE_BUFFER(buffers[1], partition_sum, n / TILE_SIZE, "clEnqueueWriteBuffer failed. First kernel. Second argument.");
	RUN_KERNEL1(kernels[0], n, TILE_SIZE);

	READ_BUFFER(buffers[0], buffer, n);
	READ_BUFFER(buffers[1], partition_sum + 1, n / TILE_SIZE);

	err = rec_run(data, context, program, buffers, queue, kernels, n / TILE_SIZE, partition_sum, index + 1);
	if (err)
		goto cleanup;

	SET_KERNEL_ARG(kernels[1], 0, buffers[0], cl_mem, "clSetKernelArg failed. Second kernel. First argument.");
	SET_KERNEL_ARG(kernels[1], 1, buffers[1], cl_mem, "clSetKernelArg failed. Second kernel. Second arguemnt.");

	WRITE_BUFFER(buffers[0], buffer, n, "clEnqueueWriteBuffer failed. Second kernel. First arguemnt.");

	WRITE_BUFFER(buffers[1], partition_sum, n / TILE_SIZE, "clEnqueueWriteBuffer failed. Second kernel. Second arguemnt.");

	RUN_KERNEL2(kernels[1], n / TILE_SIZE);
	READ_BUFFER(buffers[0], buffer, n);

cleanup:

	if (partition_sum)
		free(partition_sum);

	return err;
}

int calculate(data_buffer* data, user_input* input, size_t n)
{
	cl_device_id device = NULL;
	cl_context context = NULL;
	cl_program program = NULL;

	cl_mem buffer1 = NULL;
	cl_mem buffer2 = NULL;
	cl_mem buffer3 = NULL;

	cl_kernel kernel1 = NULL;
	cl_kernel kernel2 = NULL;
	cl_kernel kernel3 = NULL;
	cl_command_queue queue = NULL;

	FILE* cl_file = NULL;
	char* source = NULL;

	cl_int err;
	int res;
	res = find_device(&device, input);
	if (res)
	{
		return res;
	}

	print_device_info(device);

	context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
	HANDLE_ERRROR("clCreateContext failed.");

	cl_file = fopen("realization.cl", "rb");
	HANDLE_ERRROR("Not expected error: file .cl not found.");

	fseek(cl_file, 0, SEEK_END);
	size_t file_size = ftell(cl_file);
	fseek(cl_file, 0, SEEK_SET);

	source = (char*)malloc(file_size + 1);
	if (source == NULL)
	{
		puts("Not enough memory for reading .cl file.");
		fclose(cl_file);
		clReleaseContext(context);
		return -1;
	}
	fread(source, sizeof(char), file_size, cl_file);
	source[file_size] = 0;
	fclose(cl_file);

	// NULL т.к. в конце стоит 0.
	program = clCreateProgramWithSource(context, 1, (const char**)&source, NULL, &err);
	HANDLE_ERRROR("clCreateProgramWithSource failed.");

	free(source);

	res = clBuildProgram(program, 1, &device, BUILD_OPTIONS, NULL, NULL);
	if (res)
	{
		puts("clBuildProgram failed.");
		size_t log_len;
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_len);
		char* log = (char*)malloc(sizeof(char) * log_len);
		if (log == NULL)
		{
			puts("Not enough memory for log.");
		}
		else
		{
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_len, log, NULL);
			puts("Log:\n");
			puts(log);
			free(log);
		}

		err = -1;
		goto cleanup;
	}

	buffer1 = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * n, NULL, &err);
	HANDLE_ERRROR("clCreateBuffer failed.");

	buffer2 = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * n, NULL, &err);
	HANDLE_ERRROR("clCreateBuffer failed.");

	buffer3 = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(cl_float) * TILE_SIZE, NULL, &err);
	HANDLE_ERRROR("clCreateBuffer failed.");

	kernel1 = clCreateKernel(program, "part1", &err);
	HANDLE_ERRROR("clCreateKernel failed.");

	kernel2 = clCreateKernel(program, "part2", &err);
	HANDLE_ERRROR("clCreateKernel failed.");

	kernel3 = clCreateKernel(program, "list_prefix_sum", &err);
	HANDLE_ERRROR("clCreateKernel failed.");

	queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
	HANDLE_ERRROR("clCreateCommandQueue failed.");

	cl_kernel kernels[3] = { kernel1, kernel2, kernel3 };
	cl_mem buffers[3] = { buffer1, buffer2, buffer3 };
	err = rec_run(data, context, program, buffers, queue, kernels, n, data->buffer1, 0);
	if (err)
	{
		goto cleanup;
	}

cleanup:
	if (device)
		clReleaseDevice(device);

	if (context)
		clReleaseContext(context);

	if (program)
		clReleaseProgram(program);

	if (buffer1)
		clReleaseMemObject(buffer1);

	if (buffer2)
		clReleaseMemObject(buffer2);

	if (buffer3)
		clReleaseMemObject(buffer3);

	if (kernel1)
		clReleaseKernel(kernel1);

	if (kernel2)
		clReleaseKernel(kernel2);

	if (kernel3)
		clReleaseKernel(kernel3);

	if (queue)
		clReleaseCommandQueue(queue);

	if (cl_file)
		fclose(cl_file);

	return err;
}
