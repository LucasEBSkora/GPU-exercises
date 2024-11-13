int low(int val, __global int* P, int size);

__kernel void parallel_merge (
    __global int *A,
    __global int *B,
    __global int *C,
    int size
    )
{
    const int id = get_global_id(0);
    int val = (id < size) ? A[id] : B[id - size];

    int low_val = low(val, A, size) + low(val, B, size);
    C[low_val] = val;
}

int low(int val,__global int* P, int size) {
    int L = 0;
    int R = size - 1;
    
    int i;
    while(L <= R) {
        i = (R+L)/2;
        if (val == P[i]) {
            return i;
        }
        if ( P[i] < val) {
            L = i + 1;
        } else {
            R = i - 1;
        }
    }
    return L;
}