#include "deflate.h"

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define HISTORY_SIZE (32*1024)

void decompress_huffman_block(char* raw_content, char* output, int max_length) {
        
    int* dist_code;
    int* lit_len_code_len;

    decode_huffman_codes(raw_content, &lit_len_code_len, &dist_code);

    char history[HISTORY_SIZE] = {0};
    int history_index = 0;
    int history_read_index = 0;
    int output_index = 0;
    
    while (1) {
        int symbol = decode_next_symbol(raw_content, lit_len_code_len);
        if (symbol == 256) {
            break;
        }
        if (symbol < 256) {
            // printf("%02hhx ", symbol);
            output[output_index++] = symbol;
            history[history_index++] = symbol;
            history_index %= HISTORY_SIZE;
        } else {
            int run = decode_run_length(raw_content, symbol);
            if (run < 3 || run > 258) {
                printf("Invalid run length\n");
                exit(1);
            }
            int dist_symbol = decode_next_symbol(raw_content, dist_code);
            long dist = decode_dist(raw_content, dist_symbol);
            if (dist < 1 || dist > 32768) {
                printf("Invalid distance\n");
                exit(1);
            }
            
            // printf("\n\tCopying %d bytes from history at dist %ld\n", run, dist);
            history_read_index = (HISTORY_SIZE - dist + output_index) % HISTORY_SIZE;
            // printf("\tIndex : %d, Read Index : %d (output_index %d)\n", history_index, history_read_index, output_index);
            for (int i = 0; i < run; i++) {
                char to_add = history[history_read_index++];
                output[output_index++] = to_add;
                history[history_index++] = to_add;
                history_index %= HISTORY_SIZE;
                history_read_index %= HISTORY_SIZE;
                // printf("%02hhx ", to_add);
            }
            
            // printf("\nNot found yet (%d)!\n", symbol);
        }
    }
}

void decode_huffman_codes(char* raw_content, int** lit_len_code_len, int** dist_code) {

    int num_lit_len_codes = 257 + get_bits(raw_content, 5); 
    int num_dist_codes = 1 + get_bits(raw_content, 5);
    int num_code_len_codes = 4 + get_bits(raw_content, 4);
    
    int code_len_code_len[19] = {0};
    code_len_code_len[16] = get_bits(raw_content, 3);
    code_len_code_len[17] = get_bits(raw_content, 3);
    code_len_code_len[18] = get_bits(raw_content, 3);
    code_len_code_len[0] = get_bits(raw_content, 3);
    for (int i = 0; i < num_code_len_codes - 4; i++) {
        int j = (i % 2 == 0) ? (8 + i / 2) : (7 - i / 2);
        code_len_code_len[j] = get_bits(raw_content, 3);
    }
    
    
    int* code = code_bits_symbols_from_lengths(code_len_code_len, 19);
    int* code_lens = malloc_or_error((num_lit_len_codes + num_dist_codes)*sizeof(int));
    
    
    int i;
    for (i = 0; i < (num_lit_len_codes + num_dist_codes);) {
        int sym = decode_next_symbol(raw_content, code);
        if (0 <= sym && sym < 16) {
            code_lens[i++] = sym;
            continue;
        }
        int run_len;
        int run_val = 0;
        switch (sym)
        {
            case 16:
            if (i == 0) {
                printf("No code length to copy\n");
                exit(1);
            }
            run_len = 3 + get_bits(raw_content, 2);
            run_val = code_lens[i-1];
            break;
            case 17:
            run_len = 3 + get_bits(raw_content, 3);
            break;
            case 18:
            run_len = 11 + get_bits(raw_content, 7);
            break;
            default:
            printf("Symbol out of range\n");
            exit(1);
        }
        for (int j = 0; j < run_len; j++) {
            code_lens[i++] = run_val;
        }
    }
    if (i != num_lit_len_codes + num_dist_codes) {
        printf("Run exceeds number of code\n");
        exit(1);
    }
    
    if (code_lens[256] == 0) {
        printf("End-of-block symbol has zero code length\n");
        exit(1);
    }
    
    *lit_len_code_len = code_bits_symbols_from_lengths(code_lens, num_lit_len_codes);
    int* dist_code_len_ = code_lens + num_lit_len_codes;
    int* dist_code_len = malloc_or_error(32*sizeof(int));
    for (int i = 0; i < num_dist_codes; i++) dist_code_len[i] = dist_code_len_[i];
    
    if (num_dist_codes == 1 && dist_code_len[0] == 0) {
        *dist_code = NULL;
    } else {
        size_t one_count = 0;
        size_t other_positive_count = 0;
        for (int i = 0; i < num_dist_codes; i++) {
            if (dist_code_len[i] == 1) one_count++;
            if (dist_code_len[i] > 1) other_positive_count++;
        }
        
        if (one_count == 1 && other_positive_count) {
            for (int i = num_dist_codes; i < 31; i++) {
                dist_code_len[i] = 0;
            }
            dist_code_len[31] = 1;
        }
        *dist_code = code_bits_symbols_from_lengths(dist_code_len, 32);
    }

    
}

int decode_dist(char* raw_content, int symbol) {
    if (symbol < 0 || symbol > 31) {
        printf("Dist symbol not in range\n");
        exit(1);
    }
    if (symbol <= 3) return symbol + 1;
	if (symbol <= 29) {
		int extra = symbol / 2 - 1;
		return ((symbol % 2 + 2) << extra) + 1 + get_bits(raw_content, extra);
	}
}

int decode_run_length(char* raw_content, int symbol) {
    if (symbol <= 256 || symbol > 285) {
        printf("Run symbol not in range\n");
        exit(1);
    }
    if (symbol <= 264) return symbol - 254;
    if (symbol <= 284) {
        int extra = (symbol - 261) / 4;
        return (((symbol - 265) % 4 + 4) << extra) + 3 + get_bits(raw_content, extra);
    }
    if (symbol == 285) return 258;
}

int* code_bits_symbols_from_lengths(int* code_len_code_len, int max_len) {
    int max_length = -1;
    for (int i = 0; i < max_len; i++) {
        max_length = max(max_length, code_len_code_len[i]);
    }

    int* code_bits_to_symbol = malloc_or_error((1<<(max_length+1))*sizeof(int));
    for (int i = 0; i < (1<<(max_length+1)); i++) {
        code_bits_to_symbol[i] = -1;
    }

    long nextcode = 0;
    for (int codeLength = 1; codeLength < 16; codeLength++) {
        nextcode <<= 1;
        long start_bit = 1 << codeLength;
        for (int symbol = 0; symbol < max_len; symbol++) {
            if (code_len_code_len[symbol] != codeLength) {
                continue;
            }
            code_bits_to_symbol[start_bit | nextcode] = symbol;
            if (nextcode >= start_bit) {
                printf("Over-full Huffman code tree");
                exit(1);
            }
            nextcode++;
        }
    }
    return code_bits_to_symbol;
}


int decode_next_symbol(char *raw_content, int* code_bits_to_symbol) {
    int acc = 1;
    while (1) {
        acc <<= 1;
        acc |= get_bits(raw_content, 1);
        if (code_bits_to_symbol[acc] != -1) {
            return code_bits_to_symbol[acc];
        }
    }
}

void deflate(char* raw_content, char* output, int max_length) {
    int last_block = 1;
    int compression_type;
    do {
        last_block = get_bits(raw_content, 1);
        compression_type = get_bits(raw_content, 2);
        
        switch (compression_type)
        {
        case 0:
            break;
        case 1:
            break;
        case 2:
            decompress_huffman_block(raw_content, output, max_length);
            break;
        default:
            printf("This compression type is an error\n");
            exit(1);
        }


    } while (!last_block);
    

}