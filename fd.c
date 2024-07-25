#include "taskimpl.h"
#include <sys/poll.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

enum{MAXFD = 1024};

static struct pollfd pollfd[MAXFD];
static Task *polltask[MAXFD];
static int npollfd;
static int startedfdtask;
static Tasklist sleeping;
static int sleepingcounted;
static uvlong nsec(void);

static uvlong
nsec(void)
{
	struct timeval tv;

 	gettimeofday(&tv, 0);
	return (uvlong)tv.tv_sec*1000*1000*1000 + tv.tv_usec*1000;
}

static void
tasksystem(void)
{
	if(taskrunning->system == 0){
		taskrunning->system = 1;
		--taskcount;
	}
}

static void
fdtask(void *v)
{
 	int ms;
 	Task *t;
	uvlong now;

   tasksystem();
   for(;;){
		while(taskyield() > 1)
		   ;
		errno = 0;
		if((t = sleeping.head) == nil){
			ms = -1;
		}else{
		   ms = 5000;
		}
		if(poll(pollfd, npollfd, ms) < 0){
			if(errno == EINTR)
				continue;
		   fprintf(stderr, "poll: %s\n", strerror(errno));
		   taskexitall(0);
		}
		for(int i = 0; i < npollfd; ++i){
			while(i < npollfd && pollfd[i].revents){
			   taskready(polltask[i]);
			   --npollfd;
			   pollfd[i] = pollfd[npollfd];
			   polltask[i] = polltask[npollfd];
			}
		}
		now = nsec();
		while((t = sleeping.head) && now >= t->alarmtime){
		  deltask(&sleeping, t);
		  if(t->system == 0 && --sleepingcounted == 0)
			--taskcount;
		  taskready(t);
		}
	}
}

void
fdwait(int fd, int rw)
{
	int bits;

	if(startedfdtask == 0){
		startedfdtask = 1;
		taskcreate(fdtask, 0, 32768);
	}
	if(npollfd >= MAXFD){
		fprintf(stderr, "Too many poll file descriptors");
		exit(1);
	}
	bits = 0;
	switch(rw){
	case 'r': bits |= POLLIN; break;
	case 'w': bits |= POLLOUT; break;
	}
	polltask[npollfd] = taskrunning;
	pollfd[npollfd].fd = fd;
	pollfd[npollfd].events = bits;
	pollfd[npollfd].revents = 0;
	npollfd++;
	taskswitch();
}

int
fdread1(int fd, void *buf, int n)
{
	int m;

	do{
		fdwait(fd, 'r');
	}while((m = read(fd, buf, n)) < 0 && errno == EAGAIN);
	return m;
}

int
fdwrite1(int fd, void *buf, int n)
{
	int m, tot;

	tot = 0;
	for(tot = 0; tot < n; tot += m){
		fdwait(fd, 'w');
		m = write(fd, (char*)buf + tot, n - tot);
		switch (m){
		case -1: 
			if(errno == EAGAIN)
				continue;
		case 0: return tot;
		default: tot += m;
		}
	}
	return tot;
}

int
fdread(int fd, void *buf, int n)
{
	int m;

	while((m = read(fd, buf, n)) < 0 && errno == EAGAIN){
		fdwait(fd, 'r');
	}
	return m;
}

int
fdwrite(int fd, void *buf, int n)
{
	int m, tot;

	for(tot = 0; tot < n; tot += m){
		while((m = write(fd, (char*)buf + tot, n - tot)) < 0
				&& errno == EAGAIN)
			fdwait(fd, 'w');
		if(m < 0)
			return m;
		if(m == 0)
			break;
	}
	return tot;
}

int
fdnoblock(int fd)
{
	return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL)|O_NONBLOCK);
}
