#include "utils.h"

#include <stdlib.h>
#include <stdio.h>


void* malloc_or_error(size_t size) {
    void* res = malloc(size);
    if (res == NULL) {
        perror("malloc");
        exit(1);
    }
    return res;
}

void print_byte_array(char* array, size_t size, char* line_prefix) {
    for (size_t i = 0; i < size; i++) {
        if (i % 16 == 0) {
            printf("%s%s%04lx: ", i != 0 ? "\n" : "", line_prefix, i);
        }
        printf("%02hhx ", array[i]);
    }
    printf("\n");
}
