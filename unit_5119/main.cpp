#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>

#define BUF_SIZE 1024
#define SOCK_NAME "tmp.socket"

ssize_t sock_fd_write(int sock, void* buf, ssize_t buflen, int fd)
{
	ssize_t 	  size;
	struct msghdr msg;
	struct iovec  iov;
	union {
		struct cmsghdr cmsghdr;
		char 		   control[CMSG_SPACE(sizeof(int))];
	} cmsgu;
	struct cmsghdr* cmsg;

	iov.iov_base = buf;
	iov.iov_len = buflen;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	if (fd != -1)
	{
		msg.msg_control = cmsgu.control;
		msg.msg_controllen = sizeof(cmsgu.control);

		cmsg = CMSG_FIRSTHDR(&msg);
		cmsg->cmsg_len = CMSG_LEN(sizeof(int));
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;

		//printf("passing fd %d\n", fd);
		*((int*) CMSG_DATA(cmsg)) = fd;
	}
	else
	{
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		printf("not passing fd\n");
	}

	size = sendmsg(sock, &msg, 0);

	if (size < 0)
		perror("sendmsg");
	return size;
}

ssize_t sock_fd_read(int sock, void* buf, ssize_t bufsize, int* fd)
{
	ssize_t size;

	if (fd)
	{
		struct msghdr msg;
		struct iovec  iov;
		union {
			struct cmsghdr cmsghdr;
			char 		   control[CMSG_SPACE(sizeof(int))];
		} cmsgu;
		struct cmsghdr* cmsg;

		iov.iov_base = buf;
		iov.iov_len = bufsize;

		msg.msg_name = NULL;
		msg.msg_namelen = 0;
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_control = cmsgu.control;
		msg.msg_controllen = sizeof(cmsgu.control);
		size = recvmsg(sock, &msg, 0);
		if (size < 0)
		{
			perror("recvmsg");
			exit(1);
		}
		cmsg = CMSG_FIRSTHDR(&msg);
		if (cmsg && cmsg->cmsg_len == CMSG_LEN(sizeof(int)))
		{
			if (cmsg->cmsg_level != SOL_SOCKET)
			{
				fprintf(stderr, "invalid cmsg_level %d\n", cmsg->cmsg_level);
				exit(1);
			}
			if (cmsg->cmsg_type != SCM_RIGHTS)
			{
				fprintf(stderr, "invalid cmsg_type %d\n", cmsg->cmsg_type);
				exit(1);
			}

			*fd = *((int*) CMSG_DATA(cmsg));
		}
		else
			*fd = -1;
	}
	else
	{
		size = read(sock, buf, bufsize);
		if (size < 0)
		{
			perror("read");
			exit(1);
		}
	}
	return size;
}

void child(int sock)
{
	int fd;
	char buf[BUF_SIZE];
	ssize_t size;

	sleep(1);
	for (;;)
	{
		size = sock_fd_read(sock, buf, sizeof(buf), &fd);
		if (size <= 0) break;
		if (fd != -1)
		{
			write(fd, buf, size);
		}
	}
}

void parent(int sock, int fd)
{
	ssize_t size;
	char Buf[BUF_SIZE];

	size = read(fd, Buf, BUF_SIZE);
	if (size > 0)
	{
		Buf[size] = 0;
		sock_fd_write(sock, Buf, size+1, fd);
	}
}

int main( int argc, char *argv[])
{
	int sv[2];
	socketpair(AF_UNIX, SOCK_SEQPACKET, 0, sv);
	if (!fork())
	{
		// Worker
		int data_socket;
		char Buf[BUF_SIZE];
		struct sockaddr_un addr;
		
		if (-1 == (data_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0)))
		{
			perror("socket");
			exit(EXIT_FAILURE);
		}

		bzero(&addr, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, SOCK_NAME, sizeof(addr.sun_path)-1);

		if (-1 == connect(data_socket, (const struct sockaddr*)&addr, sizeof(addr)))
		{
			perror("The server is down.");
			exit(EXIT_FAILURE);
		}
		
		// send

		if (-1 == write(data_socket, "123", 3))
		{
			perror("write");
			exit(EXIT_FAILURE);
		}

		// Receive result
		ssize_t size;
		if (-1 == (size = read(data_socket, Buf, BUF_SIZE)))
		{
			perror("read");
			exit(EXIT_FAILURE);
		}

		Buf[size] = 0;

		printf("Result = %s\n", Buf);

		close(data_socket);
	}
	else
	{
		// MasterScoket
		int MasterSocket, SlaveSocket;
		struct sockaddr_un addr;

		MasterSocket = socket(AF_UNIX, SOCK_SEQPACKET, 0);

		bzero(&addr, sizeof(addr));
		addr.sun_family = AF_UNIX;
		strncpy(addr.sun_path, SOCK_NAME, sizeof(addr.sun_path)-1);

		if (-1 == bind(MasterSocket, (const struct sockaddr*)&addr, sizeof(addr)))
		{
			perror("bind");
			exit(EXIT_FAILURE);
		}

		if (-1 == listen(MasterSocket, SOMAXCONN))
		{
			perror("listen");
			exit(EXIT_FAILURE);
		}

		char Buf[BUF_SIZE];
		for (;;)
		{
			if (-1 == (SlaveSocket = accept(MasterSocket, 0, 0)))
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}

			pid_t pid;
			switch ((pid = fork()))
			{
			case 0:
				close(sv[0]);
				child(sv[1]);
				break;
			default:
				close(sv[1]);
				parent(sv[0], SlaveSocket);
			}

			close(SlaveSocket);

		}
		close(MasterSocket);
	}
	unlink(SOCK_NAME);

	return 0;
}
