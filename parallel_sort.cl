__kernel void odd_even_sort(
    __global const int* in,
    __global int* out,
    int iteration,
    int size
    ) {
    const int id = get_global_id(0);
    int index = id*2;
    if (iteration % 2) index++;

    if (index + 1 < size) {
        if (in[index] > in[index+1]) {
            out[index] = in[index+1];
            out[index+1] = in[index];
        } else {
            out[index] = in[index];
            out[index+1] = in[index+1];
        }

    } else {
        out[index] = in[index];
    }
}