#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "zip.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage %s <zip_file>\n", argv[0]);
        exit(1);
    } 
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }
    struct stat st;
    if (fstat(fd, &st)) {
        perror("fstat");
        exit(1);
    }
    
    char* file_content = malloc_or_error(st.st_size * sizeof(char));
    if (read(fd, file_content, st.st_size) != st.st_size) {
        perror("read");
        exit(1);
    }

    if (!strncmp(file_content, "PK\x03\x04", 4)) {
        extract_zip(file_content, st.st_size);
    }

    return 0;
}