#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define SHARED_FILE "/test.shm"
#define SEG_SIZE 1024*1024
#define PERMS 0777

int main(int argc, char* argv[])
{
    int shmid = shm_open(SHARED_FILE, O_RDWR|O_CREAT, PERMS);
    ftruncate(shmid, SEG_SIZE);

    char* addr_ptr = (char *)mmap(NULL, SEG_SIZE, PROT_WRITE, MAP_SHARED, shmid, 0);
    if (addr_ptr == MAP_FAILED)
        handle_error("mmap");

    memset(addr_ptr, 13, SEG_SIZE);

    munmap(addr_ptr, SEG_SIZE);
    shm_unlink(SHARED_FILE);

    return 0;
}
