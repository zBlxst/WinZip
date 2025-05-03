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

char get_bit(char* raw_content, int reset) {
    static size_t n_bit = 0;
    if (reset) {
        n_bit = 0;
        return 0;
    }
    return (raw_content[n_bit / 8] >> (n_bit++ % 8)) & 1;
}

void reset_get_bit() {
    get_bit("", 1);
}

int16_t get_bits(char* raw_content, int count) {
    int16_t res = 0;
    for (int i = 0; i < count; i++) {
        res |= get_bit(raw_content, 0) << i;
    }
    return res;
}


