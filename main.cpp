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
#include <set>

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

// Generates a set of values without any of the elements from the already_used set
set<int> generate_value_set(const int size, const int max, const set<int> &already_used = {})
{
	set<int> values;
	while (values.size() < size)
	{
		int val = rand() % max;
		if (!values.count(val) && !already_used.count(val))
		{
			values.insert(val);
		}
	}
	return values;
}

// converts a std set to a c-style array
int *set_to_array(const set<int> &set)
{
	int *array = new int[set.size()];
	int i = 0;
	for (int value : set)
	{
		array[i++] = value;
	}

	return array;
}

void cpu_merge(const int *A, const int *B, int *C, const int size)
{
	int i = 0;
	int j = 0;
	for (int k = 0; k < 2 * size; ++k)
	{
		if (i >= size)
		{
			C[k] = B[j++];
		}
		else if (j >= size)
		{
			C[k] = A[i++];
		}
		else if (A[i] < B[j])
		{
			C[k] = A[i++];
		}
		else
		{
			C[k] = B[j++];
		}
	}
}

void gpu_merge(const int *A, const int *B, int *C, const int size)
{
	const char *clu_File = SRC_PATH "parallel_merge.cl"; // path to file containing OpenCL kernel(s) code

	// // // Initialize OpenCL
	cluInit();

	// // // Load Program
	cl::Program *program = cluLoadProgram(clu_File);

	cl::Kernel *kernel = cluLoadKernel(program, "parallel_merge");

	cl::Buffer bufA(*clu_Context, CL_MEM_READ_ONLY, size * sizeof(int));
	cl::Buffer bufB(*clu_Context, CL_MEM_READ_ONLY, size * sizeof(int));
	cl::Buffer bufResult(*clu_Context, CL_MEM_WRITE_ONLY, 2 * size * sizeof(int));

	clu_Queue->enqueueWriteBuffer(bufA, true, 0, size * sizeof(int), A);
	clu_Queue->enqueueWriteBuffer(bufB, true, 0, size * sizeof(int), B);

	kernel->setArg(0, bufA);
	kernel->setArg(1, bufB);
	kernel->setArg(2, bufResult);
	kernel->setArg(3, size);

	clu_Queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(2 * size));
	clu_Queue->finish();

	clu_Queue->enqueueReadBuffer(bufResult, true, 0, 2 * size * sizeof(int), C);
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		cout << "usage: td2 <N> <MAX> \n\twhere N is the size of the random arrays to generate\n\tand MAX is the maximum allowed value\n";
		exit(-1);
	}
	const int N = atoi(argv[1]);
	const int MAX = atoi(argv[2]);

	if (MAX <= 2 * N)
	{
		cout << "value of MAX too small to create arrays without repetition!\n";
		exit(0);
	}
	const int size = N;

	srand(time(nullptr));
	set<int> A_set = generate_value_set(size, MAX);
	set<int> B_set = generate_value_set(size, MAX, A_set);

	int *A = set_to_array(A_set);
	int *B = set_to_array(B_set);

	cout << "A:";
	print_array(A, size);
	cout << "\nB:";
	print_array(B, size);

	int *result_cpu = new int[2 * size];

	cpu_merge(A, B, result_cpu, size);

	cout << "\nCPU result:\n";
	print_array(result_cpu, 2 * size);

	int *result_gpu = new int[2 * size];
	gpu_merge(A, B, result_gpu, size);

	cout << "\nGPU result:\n";
	print_array(result_gpu, 2 * size);

	for (int i = 0; i < 2 * size; ++i)
	{
		if (result_cpu[i] != result_gpu[i])
			cout << "value at index " << i << " is " << result_gpu[i] << " but should be " << result_cpu[i] << '\n';
	}

	delete[] A;
	delete[] B;

	delete[] result_cpu;
	delete[] result_gpu;
}