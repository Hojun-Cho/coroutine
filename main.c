#include "task.h"
#include "print.h"

enum{ STACK = 32768 };

char *server = 0;
int port = 8080;

void
clienttask(void *v)
{
	int fd;
	char buf[1024];
	int rc;

	for(;;){
		fd = *(int*)taskarg();
		fdwait(fd, 'r');
		rc = fdread(fd, buf, sizeof(buf) - 1);
		buf[rc] = 0;
		print("%s\n", buf);
		taskyield();
	}
	taskexit(0);
}

void
taskmain(int argc, char *argv[])
{
	int fd;
	if((fd = netannounce(0, port)) == -1)
		return;
	for(;;){
		int cfd = netaccept(fd, 0, &port);
		taskcreate(clienttask, &cfd, STACK);
	}
}
