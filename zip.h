#pragma once

#include <sys/types.h>
#include <stdio.h>

typedef struct {
    int16_t id;
    int16_t size;
    char* content;
} __attribute__((packed)) extra_field_entry;

typedef struct extra_field_entries {
    extra_field_entry* entry;
    struct extra_field_entries* next;
} extra_field_entries;

typedef struct {
    int32_t signature;
    int16_t version_min;
    int16_t bit_flag;
    int16_t compression;
    int16_t last_modif_time;
    int16_t last_modif_date;
    int32_t crc_32;
    int32_t compressed_size;
    int32_t decompressed_size;
    int16_t file_name_length;
    int16_t extra_field_length;
    char* file_name;
    extra_field_entries* extra_field_entries;
} __attribute__((packed)) local_file_header;

typedef struct {
    local_file_header* head;
    char* raw_content;
} file_entry;

typedef struct file_entry_list {
    file_entry* entry;
    struct file_entry_list* next;
} file_entry_list;

typedef struct {
    int32_t signature;
    int16_t version_made;
    int16_t version_min;
    int16_t bit_flag;
    int16_t compression;
    int16_t last_modif_time;
    int16_t last_modif_date;
    int32_t crc_32;
    int32_t compressed_size;
    int32_t decompressed_size;
    int16_t file_name_length;
    int16_t extra_field_length;
    int16_t file_comment_length;
    int16_t n_disk;
    int16_t internal_file_attributes;
    int32_t external_file_attributes;
    int32_t local_file_header_offset;
    char* file_name;
    extra_field_entries* extra_field_entries;
    char* file_comment;
} __attribute__((packed)) CDEntry;

typedef struct CDEntry_list {
    CDEntry* entry;
    struct CDEntry_list* next;
} CDEntry_list;

typedef struct {
    int32_t signature;
    int16_t n_disk;
    int16_t disk_where_cd_starts;
    int16_t n_cd_records_on_disk;
    int16_t n_cd_records_total;
    int32_t cd_size;
    int32_t cd_offset;
    int16_t comment_length;
    char* comment;
} __attribute__((packed)) EOCD;


typedef struct {
    file_entry_list* file_entry_list;
    CDEntry_list* cd_entries;
    EOCD* eocd;
} zip;

void print_extra_field_entry(extra_field_entry* entry);
void print_extra_field_entries(extra_field_entries* extra_field_entries);


void print_local_file_header(local_file_header* head);
void print_local_file_headers(file_entry_list* file_entry_list);
void print_cd_entry(CDEntry* cd);
void print_cd_entries(CDEntry_list* cdentry_list);
void print_eocd(EOCD* eocd);

void parse_file_entry(char* file_content, off_t size, off_t offset, zip* zip);
void parse_file_entries(char* file_content, off_t size, zip* zip);

off_t parse_cd_entry(char* file_content, off_t size, off_t offset, zip* zip);
void parse_cd_entries(char* file_content, off_t size, zip* zip);

void find_and_parse_eocd(char* file_content, off_t size, zip* zip);

void extract_zip(char* file_content, off_t size);
void extract_file(file_entry* file_entry);
