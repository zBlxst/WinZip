#pragma once

#include <sys/types.h>

#define max(a,b) (a>b?a:b)
#define min(a,b) (a<b?a:b)

void* malloc_or_error(size_t size);
void print_byte_array(char* array, size_t size, char* line_prefix);

char get_bit(char* raw_content, int reset, int align);
void align_get_bit();
void reset_get_bit();
int16_t get_bits(char* raw_content, int count);
