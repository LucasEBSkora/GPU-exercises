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

void print_array(const int * array, const int size) {
	if (!array || !size) return;
	cout << array[0];
	for (int i = 1; i < size; ++i)
		cout << ' ' << array[i];
	cout << endl;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		cout << "usage: td2 <N>\n\twhere 2^N is the size of the random array to generate\n";
	}
	const int N = atoi(argv[1]);

	const char *clu_File = SRC_PATH "parallel_scan.cl"; // path to file containing OpenCL kernel(s) code

	// Initialize OpenCL
	cluInit();

	// After this call you have access to
	// clu_Context;      <= OpenCL context (pointer)
	// clu_Devices;      <= OpenCL device list (vector)
	// clu_Queue;        <= OpenCL queue (pointer)

	// Load Program
	cl::Program *program = cluLoadProgram(clu_File);

	cl::Kernel *kernel = cluLoadKernel(program, "parallel_scan");

	const int size = (1 << N);

	srand(time(nullptr));

	cl::Buffer buffer1(*clu_Context, CL_MEM_READ_WRITE, size * sizeof(int));
	cl::Buffer buffer2(*clu_Context, CL_MEM_READ_WRITE, size * sizeof(int));

	int *input_data = new int[size];
	for (int i = 0; i < size; i++)
	{
		int value = rand() % 100;
		input_data[i] = value;
	}
	cout << "input data:\n";
	print_array(input_data, size);

	clu_Queue->enqueueWriteBuffer(buffer1, true, 0, size * sizeof(int), input_data);
	int *result_cpu = new int[size];
	result_cpu[0] = input_data[0];

	for (int i = 1; i < size; i++) {
		result_cpu[i] = result_cpu[i - 1] + input_data[i];
	}

	cout << "\nCPU result:\n";
	print_array(result_cpu, size);

	delete[] input_data;

	
	int *result_gpu = new int[size];

	for (int i = 0; i < N; ++i) {
		if (i % 2 == 0)
		{
			kernel->setArg(0, buffer1);
			kernel->setArg(1, buffer2);
		}
		else
		{
			kernel->setArg(0, buffer2);
			kernel->setArg(1, buffer1);
		}
		kernel->setArg(2, i);
		
		clu_Queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(size));
		clu_Queue->finish();

		int *c = new int[size];
		clu_Queue->enqueueReadBuffer((i % 2 == 0) ? buffer2 : buffer1, true, 0, size * sizeof(int), result_gpu);
		cout << "iteration " << i << ":\n";
		print_array(result_gpu, size);

	}

	for (int i = 0; i < size; ++i) {
		if (result_cpu[i] != result_gpu[i]) 
			cout << "value at index " << i << " is " << result_gpu[i] << " but should be " << result_cpu[i] << '\n';
	}
	delete[] result_cpu;
	delete[] result_gpu;
}
