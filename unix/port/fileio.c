#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>

uint8_t* read_all(char const* file, size_t* len) {
    FILE* fd = fopen(file, "rb");

    if (fd == NULL) {
        goto _read_all_error_0;
    }

    if (fseek(fd, 0L, SEEK_END)) {
        goto _read_all_error_1;
    }

    long size = ftell(fd);
    if (size < 0) {
        goto _read_all_error_1;
    }
    *len = (size_t) size;

    if (fseek(fd, 0L, SEEK_SET)) {
        goto _read_all_error_1;
    }

    uint8_t* buf = malloc((size_t) size);

    size_t ret = 0;
    size_t nbytes = 0;
    while ((ret = fread(buf + nbytes, 1, (size_t) (((size_t)size) - nbytes), fd))) {
        nbytes += ret;
    }

    return buf;

    //free(buf);
_read_all_error_1:
    fclose(fd);
_read_all_error_0:
    return NULL;
}

