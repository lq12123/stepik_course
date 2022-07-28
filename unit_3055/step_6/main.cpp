#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define SEM_PATH "sem.tmp"
#define PERMS 0666
#define CNT_SEM 16

int main(int argc, char* argv[])
{
    key_t key = ftok(SEM_PATH, 1);
    if (key == -1)
        handle_error("ftok");

    int semid = semget(key, CNT_SEM, IPC_CREAT|IPC_EXCL);
    if (semid == -1)
        handle_error("semget");
   
    struct sembuf sops[CNT_SEM];
    for (size_t i = 0; i < CNT_SEM; ++i)
    {
        sops[i].sem_num = i;
        sops[i].sem_op = i;
        sops[i].sem_flg = 0;
    }
    
    if (semop(semid, sops, CNT_SEM) == -1)
        handle_error("semop");
         
    return 0;
}
