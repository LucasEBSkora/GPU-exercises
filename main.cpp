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

int sort(int *T, int L, int R, bool increasing, int stage = 0);

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "usage: td2 <N>\n\twhere 2^N is the size of the random array to generate\n";
		exit(-1);
	}
	const int N = atoi(argv[1]);

	// const char *clu_File = SRC_PATH "parallel_sort.cl"; // path to file containing OpenCL kernel(s) code

	// // Initialize OpenCL
	// cluInit();

	// // Load Program
	// cl::Program *program = cluLoadProgram(clu_File);

	// cl::Kernel *kernel = cluLoadKernel(program, "odd_even_sort");

	const int size = (1 << N);

	srand(time(nullptr));

	// cl::Buffer buffer(*clu_Context, CL_MEM_READ_WRITE, size * sizeof(int));

	int *input_data = new int[size];
	for (int i = 0; i < size; i++)
	{
		int value = rand() % 10000;
		input_data[i] = value;
	}
	cout << "input data:\n";
	print_array(input_data, size);

	// clu_Queue->enqueueWriteBuffer(buffer, true, 0, size * sizeof(int), input_data);

	int *result_cpu = new int[size];
	memcpy(result_cpu, input_data, size * sizeof(int));

	sort(result_cpu, 0, size, true);

	cout << "\nCPU result:\n";
	print_array(result_cpu, size);

	delete[] input_data;

	int *result_gpu = new int[size];

	// kernel->setArg(0, buffer);
	// kernel->setArg(2, size);

	// for (int i = 0; i < N; ++i)
	// {
	// 	kernel->setArg(1, i);

	// 	clu_Queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange((i % 2 == 0) ? size / 2 : (size - 1) / 2));
	// 	clu_Queue->finish();

	// 	int *c = new int[size];
	// 	clu_Queue->enqueueReadBuffer(buffer, true, 0, size * sizeof(int), result_gpu);
	// 	// cout << "iteration " << i << ":\n";
	// 	// print_array(result_gpu, size);
	// }

	// for (int i = 0; i < size; ++i)
	// {
	// 	if (result_cpu[i] != result_gpu[i])
	// 		cout << "value at index " << i << " is " << result_gpu[i] << " but should be " << result_cpu[i] << '\n';
	// }
	// delete[] result_gpu;
	delete[] result_cpu;
}

void swap(int *T, int first, int second)
{
	// cout << "swap: " << first << " with " << second << endl;
	const int aux = T[first];
	T[first] = T[second];
	T[second] = aux;
}

int compare(int *T, int L, int R, bool increasing, int stage, int column)
{
	int k = (R - L) / 2;
	for (int i = 0; i < k; ++i)
	{
		cout << "stage " << stage << " column " <<  column
			 << ", T[" << L + i << "] " 
			 << (increasing ? '>' : '<')
			 << " T[" << L + i + k << "]\n";
		if ((increasing && T[L + i] > T[L + i + k]) ||
			(!increasing && T[L + i] < T[L + i + k]))
			swap(T, L + i, L + i + k);
	}
	return column+1;
}

int merge(int *T, int L, int R, bool increasing, int stage, int column = 0)
{
	if (R - L > 1)
	{
		column = compare(T, L, R, increasing, stage, column);
		merge(T, L, (L + R) / 2, increasing, stage);
		merge(T, (L + R) / 2, R, increasing, stage);
	}
	return column;
}

int sort(int *T, int L, int R, bool increasing, int stage)
{
	if (R - L > 1)
	{
		sort(T, L, (R + L) / 2, true, stage);
		stage = sort(T, (R + L) / 2, R, false, stage);
		merge(T, L, R, increasing, stage);
		stage++;
	}
	return stage;
}