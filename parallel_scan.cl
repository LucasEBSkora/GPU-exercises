__kernel void parallel_scan(
    __global const int* in,
    __global int* out,
    int iteration
    ) {
    const int id = get_global_id(0);
    if (id >= (1 << iteration)) {
        out[id] = in[id] + in[id - (1 << iteration)];
    } else {
        out[id] = in[id];
    }
}