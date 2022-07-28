#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SEM_NAME "/test.sem"
#define START_VAL 66
#define PERMS 0666

int main(int argc, char* argv[])
{
    sem_unlink(SEM_NAME);
    sem_t* semid = sem_open(SEM_NAME, O_CREAT, PERMS, START_VAL);

    return 0;
}
