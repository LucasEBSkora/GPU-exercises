// ----------------------------------------------------------

// ----------------------------------------------------------

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cmath>

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

void print_conway(int W, int H, int generation, int *board);

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cout << "usage: conway <n_generations> <periodic>\n";
		return -1;
	}
	int n_generations = atoi(argv[1]);
	int periodic = 0;
	if (argc >= 3)
	{
		periodic = atoi(argv[2]);
	}

	if (n_generations <= 0)
	{
		return -1;
	}

	const char *clu_File = SRC_PATH "conway.cl"; // path to file containing OpenCL kernel(s) code

	// Initialize OpenCL
	cluInit();

	// After this call you have access to
	// clu_Context;      <= OpenCL context (pointer)
	// clu_Devices;      <= OpenCL device list (vector)
	// clu_Queue;        <= OpenCL queue (pointer)

	// Load Program
	cl::Program *program = cluLoadProgram(clu_File);

	cl::Kernel *kernel = cluLoadKernel(program, "conway");

	// blinker
	// int width = 5;
	// int height = 4;

	// glider
	int width = 5;
	int height = 5;

	const int size = width * height;
	const int group_size = 8;

	int *conway = new int[size];

	for (int i = 0; i < size; i++)
	{
		conway[i] = 0;
	}

	// blinker
	// conway[2*width+1] = conway[2*width+2] = conway[2*width+3] = 1;

	// glider
	conway[1 * width + 2] =
		conway[2 * width + 3] =
			conway[3 * width + 1] = conway[3 * width + 2] = conway[3 * width + 3] = 1;
	cl::Buffer buf1(*clu_Context, CL_MEM_READ_WRITE, size * sizeof(int));
	cl::Buffer buf2(*clu_Context, CL_MEM_READ_WRITE, size * sizeof(int));
	clu_Queue->enqueueWriteBuffer(buf1, true, 0, size * sizeof(int), conway);

	kernel->setArg(0, width);
	kernel->setArg(1, height);
	kernel->setArg(2, periodic);

	print_conway(width, height, 0, conway);

	for (int i = 0; i < n_generations; ++i)
	{
		if (i % 2)
		{
			kernel->setArg(3, buf2);
			kernel->setArg(4, buf1);
		}
		else
		{
			kernel->setArg(3, buf1);
			kernel->setArg(4, buf2);
		}
		clu_Queue->enqueueNDRangeKernel(*kernel, cl::NullRange, cl::NDRange(size));
		clu_Queue->finish();
		clu_Queue->enqueueReadBuffer((i % 2) ? buf1 : buf2, true, 0, size * sizeof(int), conway);

		print_conway(width, height, i + 1, conway);
	}

	delete[] conway;
}

void print_conway(int W, int H, int generation, int *board)
{
	std::cout << "W=" << W << "\tH=" << H << "\tgeneration=" << generation << "\tdead X\talive 0\n";
	for (int i = 0; i < W * H; ++i)
	{
		std::cout << ((i % W == 0) ? '\n' : ' ') << board[i]; // ((board[i] == 0) ? 'X' : '0');
	}
	std::cout << std::endl;
}