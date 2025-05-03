#pragma once

void deflate(char* raw_content, char* output, int max_length);

void decode_huffman_codes(char* raw_content, int** lit_len_code_len, int** dist_code);
int decode_next_symbol(char *raw_content, int* code_bits_to_symbol);
int* code_bits_symbols_from_lengths(int* code_len_code_len, int max_len);
int decode_dist(char* raw_content, int symbol);
int decode_run_length(char* raw_content, int symbol);
