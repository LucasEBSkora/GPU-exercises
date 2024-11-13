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
#include <algorithm>
#include <vector>

using namespace std;

// ----------------------------------------------------------

// We provide a small library so we can easily setup OpenCL
#include "clutils.h"

// ----------------------------------------------------------

void print_array(const int *array, const int size)
{
	if (!array || !size)
		return;
	cout << array[0];
	for (int i = 1; i < size; ++i)
		cout << ' ' << array[i];
	cout << endl;
}

typedef struct compare_pair_s
{
	int first;
	int second;
} ComparePair;

typedef struct iteration_s
{
	int n_pairs;
	ComparePair pairs[10];
} Iteration;

typedef struct network_s
{
	int n_iterations;
	Iteration iterations[10];
} Network;

Network networks[6] = {
	{1, {{1, {{0,1}}}}},
	{3, {{1, {{0,2}}}, {1, {{0,1}}}, {1, {{1,2}}}}}
};

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "usage: td2 <N>\n\twhere 2 < N < 8 is the size of the random array to generate\n";
		exit(-1);
	}
	const int N = atoi(argv[1]);

	const int size = N;

	srand(time(nullptr));
	int *input_data = new int[size];
	for (int i = 0; i < size; i++)
	{
		int value = rand() % 10000;
		input_data[i] = value;
	}
	cout << "input data:\n";
	print_array(input_data, size);

	vector<int> result_cpu;
	for (int i = 0; i < size; ++i)
		result_cpu.push_back(input_data[i]);

	std::sort(result_cpu.begin(), result_cpu.end());

	cout << "\nCPU result:\n";
	print_array(result_cpu.data(), size);

	const char *clu_File = SRC_PATH "optimal_network.cl"; // path to file containing OpenCL kernel(s) code

	// // Initialize OpenCL
	cluInit();

	// // Load Program
	cl::Program *program = cluLoadProgram(clu_File);

	cl::Kernel *kernel = cluLoadKernel(program, "optimal_network");

	cl::Buffer buffer(*clu_Context, CL_MEM_READ_WRITE, size * sizeof(int));
	cl::Buffer buffer_pairs(*clu_Context, CL_MEM_READ_WRITE, 20 * sizeof(int));

	clu_Queue->enqueueWriteBuffer(buffer, true, 0, size * sizeof(int), input_data);

	delete[] input_data;

	int *result_gpu = new int[size];

	const int network_index = size - 2;

	kernel->setArg(0, buffer);

	Network& network = networks[network_index]; 

	for (int i = 0; i < network.n_iterations; ++i)
	{
		Iteration& it = network.iterations[i];

		clu_Queue->enqueueWriteBuffer(buffer_pairs, true, 0, it.n_pairs * 2 * sizeof(int), (int*) it.pairs);


		kernel->setArg(1, buffer_pairs);
		

		clu_Queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(it.n_pairs));
		clu_Queue->finish();

		int *c = new int[size];
		clu_Queue->enqueueReadBuffer(buffer, true, 0, size * sizeof(int), result_gpu);
		cout << "iteration " << i << ":\n";
		print_array(result_gpu, size);
	}

	for (int i = 0; i < size; ++i)
	{
		if (result_cpu[i] != result_gpu[i])
			cout << "value at index " << i << " is " << result_gpu[i] << " but should be " << result_cpu[i] << '\n';
	}
	delete[] result_gpu;
}