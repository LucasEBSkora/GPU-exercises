// je pense que tu ne peux pas declarer et utiliser des structs ici
// mais un tableau de couples de taille N est la même chose dans 
// la memoire qu'un tableau de taille 2*N
void __kernel network_sort(__global int *a, __global int* table) { 
        int id = get_global_id(0);
        int val;
        int indice1 = 2*id;
        int indice2 = 2*id+1;
        if(a[table[indice1]] > a[table[indice2]]) {
            val = a[table[indice1]];
            a[table[indice1]] = a[table[indice2]];
            a[table[indice2]] = val;
        }
    }