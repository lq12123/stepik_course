#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define OUT_FILE "main.pid"
#define NUM_OF_THREADS 4

pthread_t thid[NUM_OF_THREADS];
pthread_mutex_t m;
pthread_spinlock_t slock;
pthread_rwlock_t rwlock[2];

void* do_something1(void*)
{
    pthread_mutex_lock(&m);
    
    /* ... */

    pthread_mutex_unlock(&m);

    pthread_exit(NULL);
}

void* do_something2(void*)
{
    pthread_spin_lock(&slock);
    /* ... */
    pthread_spin_unlock(&slock);

    pthread_exit(NULL);
}

void* do_something3(void*)
{
    // SHARED LOCK
    pthread_rwlock_rdlock(&rwlock[0]);
    /* ... */
    pthread_rwlock_unlock(&rwlock[0]);

    pthread_exit(NULL);
}

void* do_something4(void*)
{
    // EXCLUSIVE LOCK
    pthread_rwlock_wrlock(&rwlock[1]);
    /* ... */
    pthread_rwlock_unlock(&rwlock[1]);

    pthread_exit(NULL);
}

int main(int argc, char* argv[])
{
    pthread_mutex_init(&m, NULL);
    pthread_spin_init(&slock, PTHREAD_PROCESS_PRIVATE);
    pthread_rwlock_init(&rwlock[0], NULL);
    pthread_rwlock_init(&rwlock[1], NULL);

    pthread_create(&thid[0], NULL, do_something1, NULL);
    pthread_create(&thid[1], NULL, do_something2, NULL);
    pthread_create(&thid[2], NULL, do_something3, NULL);
    pthread_create(&thid[2], NULL, do_something4, NULL);

    pthread_mutex_lock(&m);
    pthread_spin_lock(&slock);
    pthread_rwlock_rdlock(&rwlock[0]);
    pthread_rwlock_wrlock(&rwlock[1]);

    FILE* fout = fopen(OUT_FILE, "w");
    fprintf(fout, "%d", getpid());
    fclose(fout);

    // destruction of mutexes
    pthread_mutex_destroy(&m);
    pthread_spin_destroy(&slock);
    pthread_rwlock_destroy(&rwlock[0]);
    pthread_rwlock_destroy(&rwlock[1]);

    return 0;
}
