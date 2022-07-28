#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define OUT_FILE "main.pid"

void* thread(void* arg)
{
    FILE* fout;
    if (!(fout = fopen(OUT_FILE, "w")))
        handle_error("fopen");

    fprintf(fout, "%d", *(pid_t *)arg);
    
    fclose(fout);

    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    pthread_t thid;
    pid_t cur_pid = getpid();
    pthread_create(&thid, NULL, thread, &cur_pid); 
    pthread_join(thid, NULL);
    
    return 0;
}
