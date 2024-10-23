__kernel void calculate_distances(
    __global const int* in,
    __global int* out,
    int n_points
    ) {
    const int id = get_global_id(0);
    int i = 0;
    int j = id;

    for (int k = n_points - 1; k > 0; --k) {
        if (j < k) break;
        j -= k;
        i++;
    }
    j += i + 1;

    const int x1 = in[2 * i];
    const int x2 = in[2 * j];
    const int y1 = in[2 * i + 1];
    const int y2 = in[2 * j + 1];
    const int dx = x2 - x1;
    const int dy = y2 - y1;
    const int distance = dx * dx + dy * dy;
    out[id] = distance;
}

__kernel void hierarchical_reduction_min(
    __global const int* in,
    __global int* out,
    const int size
) {
    const int first = get_global_id(0);
    const int second = get_global_size(0) + first;
    int new_value = in[first];
    if (second < size && in[second] < new_value)
        new_value = in[second];
    out[first] = new_value;
}