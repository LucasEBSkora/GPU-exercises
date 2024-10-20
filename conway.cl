__kernel void conway(int width, int heigth, int periodic,
                     __global int *in, __global int *out)
{

    int id = get_global_id(0);
    const int i = id / width;
    const int j = id % width;

    int n_neighbors = 0;

    if (periodic)
    {

        int up = width * (i == 0 ? heigth - 1 : i - 1);
        int down = width * (i >= (heigth - 1) ? 0 : i + 1);
        int left = j == 0 ? width - 1 : j - 1;
        int right = (j >= (width - 1)) ? 0 : j + 1;

        n_neighbors = in[up + left] +
                      in[up + j] +
                      in[up + right] +

                      in[i * width + left] +
                      in[i * width + right] +

                      in[down + left] +
                      in[down + j] +
                      in[down + right];
    }
    else
    {
        if (i != 0)
        {
            int top_middle = id - width;
            if (j != 0)
                n_neighbors += in[top_middle - 1];

            n_neighbors += in[top_middle];

            if (j != width - 1)
                n_neighbors += in[top_middle + 1];
        }

        if (j != 0)
            n_neighbors += in[id - 1];
        if (j != width - 1)
            n_neighbors += in[id + 1];

        if (i != heigth - 1)
        {
            int bottom_middle = id + width;
            if (j != 0)
                n_neighbors += in[bottom_middle - 1];

            n_neighbors += in[bottom_middle];

            if (j != width - 1)
                n_neighbors += in[bottom_middle + 1];
        }
    }

    int new_value = in[id];

    if (n_neighbors < 2 || n_neighbors > 3)
        new_value = 0;
    else if (n_neighbors == 3)
        new_value = 1;

    out[id] = new_value;
}