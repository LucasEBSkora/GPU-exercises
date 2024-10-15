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
#include <tuple>
#include <vector>

using namespace std;

// ----------------------------------------------------------

// We provide a small library so we can easily setup OpenCL
#include "clutils.h"

// ----------------------------------------------------------

typedef struct board_s
{
	int width;
	int height;
	int *board;
} Board;

Board *build_board(const char *string);
void print_conway(Board *board, int generation);

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "usage: conway <n_generations> <periodic>\n";
		return -1;
	}
	int n_generations = atoi(argv[1]);
	int periodic = 0;
	if (argc >= 3)
	{
		periodic = atoi(argv[2]);
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

	const char *blinker = "XXXXX\n"
						  "XXXXX\n"
						  "XOOOX\n"
						  "XXXXX\n";

	const char *glider = "XXXXX\n"
						 "XXOXX\n"
						 "XXXOX\n"
						 "XOOOX\n"
						 "XXXXX\n";

	Board *board = build_board(glider);

	const int size = board->width * board->height;

	cl::Buffer buf1(*clu_Context, CL_MEM_READ_WRITE, size * sizeof(int));
	cl::Buffer buf2(*clu_Context, CL_MEM_READ_WRITE, size * sizeof(int));
	clu_Queue->enqueueWriteBuffer(buf1, true, 0, size * sizeof(int), board->board);

	kernel->setArg(0, board->width);
	kernel->setArg(1, board->height);
	kernel->setArg(2, periodic);

	print_conway(board, 0);

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
		clu_Queue->enqueueReadBuffer((i % 2) ? buf1 : buf2, true, 0, size * sizeof(int), board->board);

		print_conway(board, i + 1);
	}

	delete[] board;
}

bool process_char(char c, vector<int> &vector)
{
	if (c == 'O')
	{
		vector.push_back(1);
		return true;
	}
	else if (c == 'X')
	{
		vector.push_back(0);
		return true;
	}
	return false;
}

Board *build_board(const char *string)
{
	Board *board = new Board();
	board->width = 0;
	board->height = 1;
	vector<int> temp_board;

	int i = 0;
	for (; string[i] != '\n'; ++i)
	{
		char val = string[i];
		if (!process_char(val, temp_board))
		{
			cout << "unexpected value in input string!: " << (int)val << endl;
			exit(-1);
		}
	}
	board->width = i;
	++i;
	int running_width = 0;

	for (; string[i] != '\0'; ++i)
	{
		char val = string[i];
		if (process_char(val, temp_board)) {
			running_width++;
		}
		else if (val == '\n')
		{
			if (running_width != board->width)
			{
				cout << i;
				cout << "line " << board->height << " with width " << running_width << " instead of expected " << board->width << '!' << endl;
				exit(-1);
			}
			board->height += 1;
			running_width = 0;
		}
		else
		{
			cout << "unexpected value in input string!: " << (int)val << endl;
			exit(-1);
		}
	}

	board->board = new int[temp_board.size()];
	memcpy(board->board, temp_board.data(), temp_board.size() * sizeof(int));

	return board;
}

void print_conway(Board *board, int generation)
{
	cout << "W=" << board->width << "\tH=" << board->height << "\tgeneration=" << generation << "\tdead X\talive 0\n";
	for (int i = 0; i < board->width * board->height; ++i)
	{
		cout << ((i % board->width == 0) ? '\n' : ' ') << ((board->board[i] == 0) ? 'X' : 'O');
	}
	cout << endl;
}