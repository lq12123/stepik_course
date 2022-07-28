#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define OUT_FILE "main.pid"
#define NUM_OF_THREADS 2


pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t bar;

void* do_something1(void*)
{
    pthread_cond_wait(&cond, &mut);

    /* ... */

    pthread_exit(NULL);
}

void* do_something2(void*)
{
    pthread_barrier_wait(&bar);

    /* ... */

    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    pthread_attr_t attr;

    pthread_t thid[NUM_OF_THREADS];

    // pthread_barrier initialization
    pthread_barrier_init(&bar, NULL, 1);

    pthread_create(&thid[0], NULL, do_something1, NULL);
    pthread_create(&thid[1], NULL, do_something2, NULL);

    FILE* fout = fopen(OUT_FILE, "w");
    fprintf(fout, "%d", getpid());
    fclose(fout);

    pthread_barrier_destroy(&bar);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mut);

    return 0;
}
