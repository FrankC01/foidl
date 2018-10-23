/*
    foidl_errors.c
    Library main entry

    Copyright Frank V. Castellucci
    All Rights Reserved
*/

#define SYSCALLS_IMPL
#include <foidlrt.h>
#ifdef _MSC_VER
#include <io.h>
#else
#include    <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// #include <io.h>
#include <sys/mman.h>


void foidl_exit() {
    exit(0);
}

ft foidl_fclose(ft file) {
    return close(file);
}

ft foidl_fopen_read_only(char *fname) {
    return open(fname, O_RDONLY);
}

ft foidl_fopen_create_truncate(char *fname) {
    return open(fname, O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644);
}

void *foidl_open_ro_mmap_file(char * fname) {

    // Open the file for reading
    int fd = open(fname, O_RDONLY);

    // Get the size of the file.
    struct stat s;
    int size;

    int status = fstat (fd, & s);
    size = s.st_size;

    // Get the memory mapped file for reading
    return mmap(0, size, PROT_READ, MAP_PRIVATE, fd, 0);
}

void foidl_deallocate_mmap(char *fbuffer, ft fsize) {
    int res = munmap(fbuffer, fsize);
    return;
}