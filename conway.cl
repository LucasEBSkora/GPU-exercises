__kernel void conway(int width, int heigth,
                     __global int *board,
                     bool periodic)
{

    int id = get_global_id(0);
    int j = id % width;
    int i = id / width;

    int n_neighbors = 0;

    if (i != 0)
    {
        if (j != 0)
            n_neighbors += board[(i - 1) * width + (j - 1)];

        n_neighbors += board[(i - 1) * width + (j)];

        if (j != width - 1)
            n_neighbors += board[(i - 1) * width + (j + 1)];
    }

    if (j != 0)
        n_neighbors += board[(i)*width + (j - 1)];
    if (j != width - 1)
        n_neighbors += board[(i)*width + (j + 1)];

    if (i != heigth - 1)
    {
        if (j != 0)
            n_neighbors += board[(i + 1) * width + (j - 1)];
        n_neighbors += board[(i + 1) * width + (j)];
        if (j != width - 1)
            n_neighbors += board[(i + 1) * width + (j + 1)];
    }

    int new_value = board[id];

    if (n_neighbors < 2 || n_neighbors > 3)
        new_value = 0;
    else if (n_neighbors == 3)
        new_value = 1;

    barrier(CLK_GLOBAL_MEM_FENCE);

    board[id] = new_value;
}