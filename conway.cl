__kernel void conway(int width, int heigth,
                     __global int *board,
                     int periodic)
{

    int id = get_global_id(0);
    int i = id / width;
    int j = id % width;
    int left = j - 1;
    int right = j + 1;
    int up = i - 1;
    int down = i + 1; 

    int n_neighbors = 0;

    if (periodic) {
        if (i == 0) up = heigth - 1;
        else if (i >= (heigth - 1)) down = 0;
        if (j == 0) left = width - 1;
        else if (j >= (width - 1)) right = 0;
    }
    if (periodic || i != 0)
    {
        if (periodic || j != 0)
            n_neighbors += board[up * width + left];

        n_neighbors += board[up * width + (j)];

        if (periodic || j != width - 1)
            n_neighbors += board[up * width + right];
    }

    if (periodic || j != 0)
        n_neighbors += board[(i)*width + left];
    if (periodic || j != width - 1)
        n_neighbors += board[(i)*width + right];

    if (periodic || i != heigth - 1)
    {
        if (periodic || j != 0)
            n_neighbors += board[down * width + left];
        n_neighbors += board[down * width + (j)];
        if (periodic || j != width - 1)
            n_neighbors += board[down * width + right];
    }

    int new_value = board[id];

    if (n_neighbors < 2 || n_neighbors > 3)
        new_value = 0;
    else if (n_neighbors == 3)
        new_value = 1;

    barrier(CLK_GLOBAL_MEM_FENCE);

    board[id] = new_value;
}