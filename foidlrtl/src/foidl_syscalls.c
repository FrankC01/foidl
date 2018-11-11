/*
    foidl_errors.c
    Library main entry

    Copyright Frank V. Castellucci
    All Rights Reserved
*/

#define SYSCALLS_IMPL
#include <foidlrt.h>
#include <stdio.h>
#ifdef _MSC_VER
#include <io.h>
#include <stdlib.h>
#else
#include <unistd.h>
#include <sys/mman.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include <io.h>


void foidl_exit() {
    exit(0);
}

ft foidl_fclose(ft file) {
    #ifdef _MSC_VER
    return _close(file);
    #else
    return close(file);
    #endif
}

ft foidl_fopen_read_only(char *fname) {
    #ifdef _MSC_VER
    return _open(fname, O_RDONLY);
    #else
    return open(fname, O_RDONLY);
    #endif
}

ft foidl_fopen_create_truncate(char *fname) {
    #ifdef _MSC_VER
    return _open(fname, O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644);
    #else
    return open(fname, O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644);
    #endif
}

void *foidl_open_ro_mmap_file(char * fname) {

    // Open the file for reading
    #ifdef _MSC_VER
    int fd = _open(fname, O_RDONLY);
    #else
    int fd = open(fname, O_RDONLY);
    #endif

    // Get the size of the file.
    struct stat s;
    int size;

    int status = fstat (fd, & s);
    size = s.st_size;

    // Get the memory mapped file for reading
    #ifdef _MSC_VER
    char *mscbuffer = (char *) malloc(size);
    _read(fd, mscbuffer, size);
    return mscbuffer;
    // return malloc(size);
    #else
    return mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
    #endif
}

void foidl_deallocate_mmap(char *fbuffer, ft fsize) {
    #ifdef _MSC_VER
    free(fbuffer);
    #else
    int res = munmap(fbuffer, fsize);
    #endif
    return;
}