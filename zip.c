#include "zip.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "utils.h"

void print_local_file_header(local_file_header* head) {
    printf("File\n");
    printf("\tSignature : %04x\n", head->signature);
    printf("\tVersion min : %d\n", head->version_min);
    printf("\tBit flag : %02x\n", head->bit_flag);
    printf("\tCompression : %x\n", head->compression);
    printf("\tLast modification time : %hu\n", head->last_modif_time);
    printf("\tLast modification date : %hu\n", head->last_modif_date);
    printf("\tCRC32 : %04x\n", head->crc_32);
    printf("\tCompressed size : %d\n", head->compressed_size);
    printf("\tDecompressed size : %d\n", head->decompressed_size);
    printf("\tFile name length : %d\n", head->file_name_length);
    printf("\tExtra field length : %d\n", head->extra_field_length);
    printf("\tFile name : %s\n", head->file_name);
    printf("\tExtra field : \n");
    print_extra_field_entries(head->extra_field_entries);
    // print_byte_array(head->extra_field, head->extra_field_length, "\t");
}

void print_local_file_headers(file_entry_list* file_entry_list) {
    if (file_entry_list == NULL) {
        return;
    }
    print_local_file_headers(file_entry_list->next);
    puts("\n");
    print_local_file_header(file_entry_list->entry->head);
}

void print_extra_field_entry(extra_field_entry* entry) {
    printf("\t\tId : %02hx\n", entry->id);
    printf("\t\tSize : %02hx\n", entry->size);
    print_byte_array(entry->content, entry->size, "\t\t");
}

void print_extra_field_entries(extra_field_entries* extra_field_entries) {
    if (extra_field_entries == NULL) {
        return;
    }
    print_extra_field_entries(extra_field_entries->next);
    print_extra_field_entry(extra_field_entries->entry);
    puts("");
}

void print_cd_entry(CDEntry* cd) {
    printf("CD Entry\n");
    printf("\tSignature : %04x\n", cd->signature);
    printf("\tVersion made : %d\n", cd->version_made);
    printf("\tVersion min : %d\n", cd->version_min);
    printf("\tBit flag : %02x\n", cd->bit_flag);
    printf("\tCompression : %x\n", cd->compression);
    printf("\tLast modification time : %hu\n", cd->last_modif_time);
    printf("\tLast modification date : %hu\n", cd->last_modif_date);
    printf("\tCRC32 : %04x\n", cd->crc_32);
    printf("\tCompressed size : %d\n", cd->compressed_size);
    printf("\tDecompressed size : %d\n", cd->decompressed_size);
    printf("\tFile name length : %d\n", cd->file_name_length);
    printf("\tExtra field length : %d\n", cd->extra_field_length);
    printf("\tFile comment length : %d\n", cd->file_comment_length);
    printf("\tDisk where file starts : %d\n", cd->n_disk);
    printf("\tInternal file attributes : %x\n", cd->internal_file_attributes);
    printf("\tExternal file attributes : %x\n", cd->external_file_attributes);
    printf("\tLocal file header offset : %x\n", cd->local_file_header_offset);
    printf("\tFile name : %s\n", cd->file_name);
    printf("\tExtra field : \n");
    // print_byte_array(cd->extra_field, cd->extra_field_length, "\t");
    print_extra_field_entries(cd->extra_field_entries);
    printf("\tComment : \n");
    print_byte_array(cd->file_comment, cd->file_comment_length, "\t");
}

void print_cd_entries(CDEntry_list* cdentry_list) {
    if (cdentry_list == NULL) {
        return;
    }
    print_cd_entries(cdentry_list->next);
    puts("\n");
    print_cd_entry(cdentry_list->entry);
}

void print_eocd(EOCD* eocd) {
    printf("EOCD\n");
    printf("\tSignature : %04x\n", eocd->signature);
    printf("\tNumber of the disk : %d\n", eocd->n_disk);
    printf("\tDisk where CD starts : %d\n", eocd->disk_where_cd_starts);
    printf("\tNumber of CD records on this disk : %d\n", eocd->n_cd_records_on_disk);
    printf("\tNumber of CD records total : %d\n", eocd->n_cd_records_total);
    printf("\tSize of CD : %d\n", eocd->cd_size);
    printf("\tOffset of CD : %x\n", eocd->cd_offset);
    printf("\tComment size : %d\n", eocd->comment_length);
    printf("\tComment : \n");
    print_byte_array(eocd->comment, eocd->comment_length, "\t");
}

void find_and_parse_eocd(char* file_content, off_t size, zip* zip) {
    size_t pos;
    for (pos = size - 22; pos > 0; pos--) {
        if (file_content[pos] == 'P' &&
            file_content[pos+1] == 'K' &&
            file_content[pos+2] == '\x05' &&
            file_content[pos+3] == '\x06') {
                break;
        }
    }
    if (pos == 0) {
        puts("Didn't find EOCD, is this a real zip file ?");
        exit(1);
    }

    EOCD* eocd = malloc_or_error(sizeof(EOCD));
    memcpy(eocd, file_content+pos, sizeof(EOCD) - sizeof(char*)); // Remove sizeof(char*) to avoid copying on the pointer
    zip->eocd = eocd;

    eocd->comment = malloc_or_error(eocd->comment_length);
    memcpy(eocd->comment, file_content+pos+(sizeof(EOCD) - sizeof(char*)), eocd->comment_length);
}

off_t parse_extra_field_entry_cd(char* file_content, off_t size, off_t offset, CDEntry* cd) {
    extra_field_entry* entry = malloc_or_error(sizeof(extra_field_entry));
    memcpy(entry, file_content+offset, sizeof(extra_field_entry) - sizeof(char*));
    
    entry->content = malloc_or_error(entry->size*sizeof(char));
    memcpy(entry->content, file_content+offset+(sizeof(extra_field_entry) - sizeof(char*)), entry->size);

    extra_field_entries* extra_field_entries = malloc_or_error(sizeof(extra_field_entries));
    extra_field_entries->entry = entry;
    extra_field_entries->next = cd->extra_field_entries;
    cd->extra_field_entries = extra_field_entries;

    return offset+(sizeof(extra_field_entry) - sizeof(char*))+entry->size;
}

void parse_extra_field_entries_cd(char* file_content, off_t size, off_t offset, off_t extra_field_length, CDEntry* cd) {
    off_t base_offset = offset;
    for (; offset < base_offset + extra_field_length;) {
        offset = parse_extra_field_entry_cd(file_content, size, offset, cd);
    }
}

off_t parse_extra_field_entry_file_header(char* file_content, off_t size, off_t offset, local_file_header* head) {
    extra_field_entry* entry = malloc_or_error(sizeof(extra_field_entry));
    memcpy(entry, file_content+offset, sizeof(extra_field_entry) - sizeof(char*));

    printf("Id : %02x\n", entry->id);
    printf("Size : %02x\n", entry->size);
    entry->content = malloc_or_error(entry->size*sizeof(char));
    memcpy(entry->content, file_content+offset+(sizeof(extra_field_entry) - sizeof(char*)), entry->size);

    extra_field_entries* extra_field_entries = malloc_or_error(sizeof(extra_field_entries));
    extra_field_entries->entry = entry;
    extra_field_entries->next = head->extra_field_entries;
    head->extra_field_entries = extra_field_entries;

    return offset+(sizeof(extra_field_entry) - sizeof(char*))+entry->size;
}

void parse_extra_field_entries_file_header(char* file_content, off_t size, off_t offset, off_t extra_field_length, local_file_header* head) {
    off_t base_offset = offset;
    for (; offset < base_offset + extra_field_length;) {
        offset = parse_extra_field_entry_file_header(file_content, size, offset, head);
    }
}



off_t parse_cd_entry(char* file_content, off_t size, off_t offset, zip* zip) {
    CDEntry* entry = malloc_or_error(sizeof(CDEntry));
    memcpy(entry, file_content+offset, sizeof(CDEntry) - 3*sizeof(char*)); // Remove 3*sizeof(char*) to avoid copying on the pointers
    
    entry->file_name = malloc_or_error(entry->file_name_length*sizeof(char));
    entry->file_comment = malloc_or_error(entry->file_comment_length*sizeof(char));
    
    memcpy(entry->file_name, file_content+offset + (sizeof(CDEntry)-3*sizeof(char*)), entry->file_name_length);
    parse_extra_field_entries_cd(file_content, size, offset + (sizeof(CDEntry)-3*sizeof(char*)) + entry->file_name_length, entry->extra_field_length, entry);
    memcpy(entry->file_comment, file_content+offset + (sizeof(CDEntry)-3*sizeof(char*)) + entry->file_name_length + entry->extra_field_length, entry->file_comment_length);
    
    CDEntry_list* new_entry_list = malloc_or_error(sizeof(CDEntry_list));
    new_entry_list->entry = entry;
    new_entry_list->next = zip->cd_entries;
    zip->cd_entries = new_entry_list;

    return offset + (sizeof(CDEntry)-3*sizeof(char*)) + entry->file_name_length + entry->extra_field_length + entry->file_comment_length;
}

void parse_cd_entries(char* file_content, off_t size, zip* zip) {
    off_t offset = zip->eocd->cd_offset; 
    for (int i = 0; i < zip->eocd->n_cd_records_total; i++) {
        offset = parse_cd_entry(file_content, size, offset, zip);
    }
}

void parse_file_entry(char* file_content, off_t size, off_t offset, zip* zip) {
    local_file_header* head = malloc_or_error(sizeof(local_file_header));
    file_entry* entry = malloc_or_error(sizeof(file_entry));
    
    memcpy(head, file_content+offset, (sizeof(local_file_header) - 2*sizeof(char*)));
    
    head->file_name = malloc_or_error(head->file_name_length * sizeof(char));
    memcpy(head->file_name, file_content+offset+(sizeof(local_file_header) - 2*sizeof(char*)), head->file_name_length);
    
    parse_extra_field_entries_file_header(file_content, size, offset + (sizeof(local_file_header)-2*sizeof(char*)) + head->file_name_length, head->extra_field_length, head);

    entry->head = head;
    entry->raw_content = malloc_or_error(head->compressed_size*sizeof(char));
    memcpy(entry->raw_content, file_content+offset+(sizeof(local_file_header) - 2*sizeof(char*))+head->file_name_length+head->extra_field_length, head->compressed_size);
    

    file_entry_list* new_entry_list = malloc_or_error(sizeof(file_entry_list));
    new_entry_list->entry = entry;
    new_entry_list->next = zip->file_entry_list;
    zip->file_entry_list = new_entry_list;

}

void parse_file_entries(char* file_content, off_t size, zip* zip) {
    CDEntry_list* cd_entry_list = zip->cd_entries;
    for (int i = 0; i < zip->eocd->n_cd_records_total; i++) {
        parse_file_entry(file_content, size, cd_entry_list->entry->local_file_header_offset, zip);
        cd_entry_list = cd_entry_list->next;
    }
}

void extract_zip(char *file_content, off_t size) {
    zip* zip = malloc_or_error(sizeof(zip));
    zip->cd_entries = NULL;
    find_and_parse_eocd(file_content, size, zip);
    parse_cd_entries(file_content, size, zip);
    parse_file_entries(file_content, size, zip);

    // print_byte_array(zip->file_entry_list->entry->raw_content, zip->file_entry_list->entry->head->compressed_size, "");
}