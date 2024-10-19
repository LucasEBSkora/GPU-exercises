__kernel void conway(int width, int heigth, int periodic,
                     __global int *in, __global int *out)
{

    int id = get_global_id(0);
    int i = id / width;
    int j = id % width;
    int left = j - 1;
    int right = j + 1;
    int up = i - 1;
    int down = i + 1;

    int n_neighbors = 0;

    if (periodic)
    {
        if (i == 0)
            up = heigth - 1;
        else if (i >= (heigth - 1))
            down = 0;
        if (j == 0)
            left = width - 1;
        else if (j >= (width - 1))
            right = 0;

        n_neighbors = in[up * width + left] +
                      in[up * width + j] +
                      in[up * width + right] +

                      in[i * width + left] +
                      in[i * width + right] +

                      in[down * width + left] +
                      in[down * width + j] +
                      in[down * width + right];
    }
    else
    {
        if (i != 0)
        {
            if (j != 0)
                n_neighbors += in[up * width + left];

            n_neighbors += in[up * width + j];

            if (j != width - 1)
                n_neighbors += in[up * width + right];
        }

        if (j != 0)
            n_neighbors += in[i * width + left];
        if (j != width - 1)
            n_neighbors += in[i * width + right];

        if (i != heigth - 1)
        {
            if (j != 0)
                n_neighbors += in[down * width + left];
            n_neighbors += in[down * width + j];
            if (j != width - 1)
                n_neighbors += in[down * width + right];
        }
    }

    int new_value = in[id];

    if (n_neighbors < 2 || n_neighbors > 3)
        new_value = 0;
    else if (n_neighbors == 3)
        new_value = 1;

    out[id] = new_value;
}