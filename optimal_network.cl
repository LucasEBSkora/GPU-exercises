__kernel void optimal_network(
    __global int *buf,
    __global int *compare_pairs)
{
    const int id = get_global_id(0);
    const int first = compare_pairs[2*id];
    const int second = compare_pairs[2*id+1];

    if(buf[first] > buf[second]) {
        int aux = buf[first];
        buf[first] = buf[second];
        buf[second] = aux;
    }
}