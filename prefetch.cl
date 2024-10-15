__kernel void summation(
    __global const int* a,
    __global const int* b,
    __global       int* c, 
    __local int* shared
    ) {
    int i = get_global_id(0);
    shared[get_local_id(0)] = a[i];
    
    barrier(CLK_LOCAL_MEM_FENCE); //every kernel in group at the same point

    //choose just one work item to do something
    if (get_local_id(0) == 0) {
        int local_sum = 0;
        for (int j = 0; j < get_local_size(0); ++j){
            local_sum += shared[j];
        }
        c[i] = local_sum;
        atomic_add(&c[1], c[i]);
    }

    //c[i] = a[i] + b[i];
}