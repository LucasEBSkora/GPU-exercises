__kernel void odd_even_sort(
    __global int *buf,
    int iteration,
    int size)
{
    const int id = get_global_id(0);
    int index = id * 2;
    if (iteration % 2)
        index++;

    if (index + 1 < size && buf[index] > buf[index + 1])
    {
        int aux = buf[index];
        buf[index] = buf[index + 1];
        buf[index + 1] = aux;
    }
}