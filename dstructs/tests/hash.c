#include "Hash.h"


int main(int argc, char *argv[]) {
    Hash *hash =  Hash_malloc();

    printf("GETTING C\n");
    Hash_get(hash, "c");
    printf("GETTING A\n");
    Hash_get(hash, "a");
    printf("GETTING B\n");
    Hash_get(hash, "b");

    Hash_free(hash);
    return 0;
}

