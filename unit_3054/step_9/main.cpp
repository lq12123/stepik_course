#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <mqueue.h>

#define OUT_FILE "message.txt"
#define MQUEUE_NAME "/test.mq"
#define PERMS (S_IRUSR|S_IWUSR)
#define BUF_SIZE 512

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

static void tfunc(union sigval sv)
{
    struct mq_attr attr;
    ssize_t nr;
    void* buf;
    mqd_t mqid = *((mqd_t *) sv.sival_ptr);

    if (mq_getattr(mqid, &attr) == -1)
        handle_error("mq_getattr");
    buf = malloc(attr.mq_msgsize);
    if (buf == NULL)
        handle_error("malloc");

    nr = mq_receive(mqid, (char *)buf, attr.mq_msgsize, NULL);
    if (nr == -1)
        handle_error("mq_receive");

    int fout = open(OUT_FILE, O_WRONLY|O_CREAT|O_TRUNC, PERMS);
    write(fout, buf, nr); 
    write(fout, "\n", 1);
    close(fout);

    free(buf);
    mq_unlink(MQUEUE_NAME);
    mq_close(mqid);
    exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[])
{
    mq_unlink(MQUEUE_NAME);

    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = 512;
    attr.mq_flags = 0;

    mqd_t mqid = mq_open(MQUEUE_NAME, O_CREAT|O_EXCL|O_NONBLOCK, PERMS, &attr);
    if (mqid == (mqd_t) -1)
        handle_error("mq_open");

    struct sigevent ev;
    ev.sigev_notify = SIGEV_THREAD; 
    ev.sigev_notify_function = tfunc;
    ev.sigev_notify_attributes = NULL;
    ev.sigev_value.sival_ptr = &mqid;
    if (mq_notify(mqid, &ev) == -1)
        handle_error("mq_notify");

    pause();
    
    return 0;
}
