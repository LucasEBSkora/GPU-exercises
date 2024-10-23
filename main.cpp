// ----------------------------------------------------------

// ----------------------------------------------------------

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <ctime>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>

using namespace std;

// ----------------------------------------------------------

// We provide a small library so we can easily setup OpenCL
#include "clutils.h"

// ----------------------------------------------------------

void print_array(const int *array, const int size)
{
	if (!array || !size)
		return;
	cout << '(' << array[0] << ',' << array[1] << ')';
	for (int i = 1; i < size; ++i)
		cout << " (" << array[2 * i + 0] << ',' << array[2 * i + 1] << ')';
	cout << endl;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "usage: td2 <N>\n\twhere 2^N is the size of the random array to generate\n";
	}
	const int N = atoi(argv[1]);

	const char *clu_File = SRC_PATH "min_distance.cl"; // path to file containing OpenCL kernel(s) code

	// Initialize OpenCL
	cluInit();

	// After this call you have access to
	// clu_Context;      <= OpenCL context (pointer)
	// clu_Devices;      <= OpenCL device list (vector)
	// clu_Queue;        <= OpenCL queue (pointer)

	// Load Program
	cl::Program *program = cluLoadProgram(clu_File);

	cl::Kernel *kernel_distances = cluLoadKernel(program, "calculate_distances");

	const int size = (1 << N);

	srand(time(nullptr));

	int *input_data = new int[2 * size];
	for (int i = 0; i < 2 * size; i++)
	{
		int value = rand() % 100;
		input_data[i] = value;
	}
	cout << "input data:\n";
	print_array(input_data, size);

	int result_cpu = INT32_MAX;

	cout << "squares of distances calculated by CPU: ";
	for (int i = 0; i < size; i++)
	{
		for (int j = i + 1; j < size; j++)
		{
			const int x1 = input_data[2 * i];
			const int x2 = input_data[2 * j];
			const int y1 = input_data[2 * i + 1];
			const int y2 = input_data[2 * j + 1];
			const int dx = x2 - x1;
			const int dy = y2 - y1;
			const int distance = dx * dx + dy * dy;
			cout << distance << ' ';

			if (distance < result_cpu)
				result_cpu = distance;
		}
	}
	cout << endl;

	cout << "\nCPU result: smallest distance is " << sqrt(result_cpu) << '\n';

	cl::Buffer point_buffer(*clu_Context, CL_MEM_READ_ONLY, size * 2 * sizeof(int));
	clu_Queue->enqueueWriteBuffer(point_buffer, true, 0, size * 2 * sizeof(int), input_data);

	delete[] input_data;

	const int distance_buffer_size = size * (size - 1) / 2;

	cl::Buffer buffer1(*clu_Context, CL_MEM_READ_WRITE, distance_buffer_size * sizeof(int));
	cl::Buffer buffer2(*clu_Context, CL_MEM_READ_WRITE, distance_buffer_size * sizeof(int));
	
	kernel_distances->setArg(0, point_buffer);
	kernel_distances->setArg(1, buffer1);
	kernel_distances->setArg(2, size);
	clu_Queue->enqueueNDRangeKernel(*kernel_distances, cl::NullRange, cl::NDRange(distance_buffer_size));
	clu_Queue->finish();

	int* distances = new int[distance_buffer_size];
	clu_Queue->enqueueReadBuffer(buffer1, true, 0, distance_buffer_size * sizeof(int), distances);

	cout << "\nsquares of distances calculated by GPU: ";
	for (int i = 0; i < distance_buffer_size; ++i)
		cout << distances[i] << ' ';
	cout << endl;
	
	cl::Kernel *kernel_reduction = cluLoadKernel(program, "hierarchical_reduction_min");

	int current_size = distance_buffer_size;

	for (int i = 0; current_size > 1; ++i)
	{
		if (i % 2 == 0)
		{
			kernel_reduction->setArg(0, buffer1);
			kernel_reduction->setArg(1, buffer2);
		}
		else
		{
			kernel_reduction->setArg(0, buffer2);
			kernel_reduction->setArg(1, buffer1);
		}
		kernel_reduction->setArg(2, current_size);
		int next_size = ceil(current_size / 2.0);
		clu_Queue->enqueueNDRangeKernel(*kernel_reduction, cl::NullRange, cl::NDRange(next_size));
		clu_Queue->finish();
		
		clu_Queue->enqueueReadBuffer((i % 2 == 0) ? buffer2 : buffer1, true, 0, distance_buffer_size * sizeof(int), distances);
		
		current_size = next_size;
		
		cout << "iteration " << i << ": ";
		for (int j = 0; j < current_size; ++j)
			cout << distances[j] << " ";
		cout << endl;
	}

	cout << "\nGPU result: smallest distance is " << sqrt(distances[0]) << '\n';
	delete[] distances;
}
