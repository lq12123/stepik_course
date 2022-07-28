#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define PATH_TO_SHARED_FILE "shared_file.temp"
#define OUT_FILE "out_file.txt"
#define MSG_PERM (S_IRUSR|S_IWUSR)
#define MSG_SIZE 256

struct msg
{
	long mtype;
	char mtext[MSG_SIZE];
};

int main(int argc, char* argv[])
{
	key_t id = ftok(PATH_TO_SHARED_FILE, 1);
	int msgid = msgget(id, IPC_CREAT|IPC_EXCL|MSG_PERM);

    struct msg mymsg;
	while (1)
	{
		ssize_t size;
        size = msgrcv(msgid, (void *) &mymsg, sizeof(mymsg.mtext), 0, MSG_NOERROR | IPC_NOWAIT);

        if (size > 0)
        {
            int fout = open(OUT_FILE, O_WRONLY|O_CREAT|O_TRUNC);
            write(fout, mymsg.mtext, size);
            write(fout, "\n", 1);
            close(fout);
        }
	}

	return 0;
}
