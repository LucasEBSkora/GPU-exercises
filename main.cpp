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

#define MIN_N 1
#define MAX_N 21

int *generate_input_array(const int size);

void print_array(const int *array, const int size);
void print_array(const vector<int> &array);

vector<int> sort_array_cpu(const int *input_data, const int size);

using Iteration = vector<int>;
using Network = vector<Iteration>;

Network get_network(const int size);
int *sort_array_gpu(const int *input_data, const int size, const char *path);

void print_usage_and_exit()
{
	cerr << "usage: td2 <networks_file> <N>"
		 << "\n\tand networks_file is the path to the file with the networks"
		 << "\n\twhere " << MIN_N << " < N < "
		 << MAX_N << " is the size of the random array to generate\n";
	exit(-1);
}

int main(int argc, char **argv)
{
	if (argc < 3)
		print_usage_and_exit();

	const char *networks_file_path = argv[1];
	const int N = atoi(argv[2]);

	if (N < MIN_N || N > MAX_N)
		print_usage_and_exit();

	const int size = N;

	int *input_data = generate_input_array(size);

	cout << "input data:\n";
	print_array(input_data, size);

	vector<int> result_cpu = sort_array_cpu(input_data, size);

	cout << "\nCPU result:\n";
	print_array(result_cpu);
	cout << '\n';

	int *result_gpu = sort_array_gpu(input_data, size, networks_file_path);

	for (int i = 0; i < size; ++i)
	{
		if (result_cpu[i] != result_gpu[i])
			cout << "value at index " << i << " is " << result_gpu[i] << " but should be " << result_cpu[i] << '\n';
	}
	delete[] input_data;
	delete[] result_gpu;
}

int *generate_input_array(const int size)
{
	srand(time(nullptr));
	int *input_data = new int[size];
	for (int i = 0; i < size; i++)
	{
		int value = rand() % 10000;
		input_data[i] = value;
	}
	return input_data;
}

void print_array(const int *array, const int size)
{
	if (!array || !size)
		return;
	cout << array[0];
	for (int i = 1; i < size; ++i)
		cout << ' ' << array[i];
	cout << endl;
}

void print_array(const vector<int> &array)
{
	print_array(array.data(), array.size());
}

vector<int> sort_array_cpu(const int *input_data, const int size)
{
	vector<int> array(input_data, input_data + size);

	std::sort(array.begin(), array.end());
	return array;
}

// advances a file to the beginning of the
void advance_to_line(ifstream &file, const unsigned int line_number)
{
	// efficient way of skipping lines in a file from https://stackoverflow.com/a/25012566
	for (int i = 1; i < line_number; ++i)
	{
		if (file.ignore(numeric_limits<streamsize>::max(), file.widen('\n')).eof())
		{
			cerr << "no network of size " << line_number << " in file!" << endl;
			file.close();
			exit(-1);
		}
	}
}

Network parse_file(ifstream &stream)
{
	Network net;
	bool done = false;
	while (!stream.eof() && !done)
	{
		char c;
		stream.get(c);
		switch (c)
		{
		case '[':
			net.push_back({});
			break;
		case '(':
		{
			int first, second;
			stream >> first;
			char comma;
			stream >> comma;
			stream >> second;
			net.back().push_back(first);
			net.back().push_back(second);
			break;
		}
		case '\n':
			done = true;
			break;
		// just ignores all other characters (specially ']' and ')' ) because it doesn't really add anything
		default:
			break;
		}
	}

	return net;
}

Network get_network(const char *path, const int size)
{
	ifstream networks_file{path};
	if (!networks_file)
	{
		cerr << "file with networks not found at path " << path;
		exit(-1);
	}
	advance_to_line(networks_file, size);

	return parse_file(networks_file);
}

int *sort_array_gpu(const int *input_data, const int size, const char *path)
{

	const Network network = get_network(path, size);

	const char *clu_File = SRC_PATH "optimal_network.cl"; // path to file containing OpenCL kernel(s) code

	// // Initialize OpenCL
	cluInit();

	// // Load Program
	cl::Program *program = cluLoadProgram(clu_File);

	cl::Kernel *kernel = cluLoadKernel(program, "optimal_network");

	cl::Buffer buffer(*clu_Context, CL_MEM_READ_WRITE, size * sizeof(int));
	cl::Buffer buffer_pairs(*clu_Context, CL_MEM_READ_WRITE, 2 * size * sizeof(int));

	clu_Queue->enqueueWriteBuffer(buffer, true, 0, size * sizeof(int), input_data);

	int *C = new int[size];

	kernel->setArg(0, buffer);
	kernel->setArg(1, buffer_pairs);

	for (int i = 0; i < network.size(); ++i)
	{
		const Iteration &it = network[i];

		clu_Queue->enqueueWriteBuffer(buffer_pairs, true, 0, it.size() * sizeof(int), it.data());

		clu_Queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(it.size()));
		clu_Queue->finish();

		int *c = new int[size];
		clu_Queue->enqueueReadBuffer(buffer, true, 0, size * sizeof(int), C);
		cout << "iteration " << i << ":\n";
		print_array(C, size);
	}

	return C;
}