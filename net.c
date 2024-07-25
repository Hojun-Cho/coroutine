#include "taskimpl.h"
#include "print.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define CLASS(p) ((*(unsigned char*)(p)) >> 6)
static int
parseip(char *name, uint32_t *ip)
{
	uchar addr[4];
	char *p;
	int i, x;

	p = name;
	for(i = 0; i < 4 && *p; ++i){
		x = strtoul(p, &p, 0);
		if(x < 0 || x >= 256)
			return -1;
		if(*p != '.' && *p != 0)
			return -1;
		if(*p == '.')
			++p;
		addr[i] = x;
	}
	switch(CLASS(addr)){
	case 0:
	case 1:
		if(i == 3){
			addr[3] = addr[2];
			addr[2] = addr[1];
			addr[1] = 0;
		}else if(i == 2){
			addr[3] = addr[1];
			addr[1] = addr[2] = 0;
		}else if(i != 4)
			return -1;
		break;
	case 2:
		if(i == 3){
			addr[3] = addr[2];
			addr[2] = 0;
		}else if(i != 4)
			return -1;
		break;
	}
	*ip = *(uint32_t*)addr;
	return 0;
}

static int
netlookup(char *name, uint32_t *ip)
{
	struct hostent *he;

	if(parseip(name, ip) == 0)
		return 0;
	if((he = gethostbyname(name)) != 0){
		*ip = *(uint32_t*)he->h_addr;
		return 0;
	}
	return -1;
}

/* bind and listen to server:port */
int
netannounce(char *server,int port)
{
	int fd, n;
	struct sockaddr_in sa;
	socklen_t sn;
	uint32_t ip;

	memset(&sa, 0, sizeof sa);
	sa.sin_family = AF_INET;
	if(server != nil && strcmp(server, "*") != 0){
		if(netlookup(server, &ip) == -1)
			return -1;
		memcpy(&sa.sin_addr, &ip, 4);
	}
	sa.sin_port = htons(port);
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		return -1;
	}
	if(getsockopt(fd, SOL_SOCKET, SO_TYPE, (void*)&n, &sn) != -1){
		n = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&n, sizeof n);
	}
	if(bind(fd, (struct sockaddr*)&sa, sizeof sa) == -1){
		perror("bind");
		close(fd);
		return -1;
	}
	listen(fd, 16);
	fdnoblock(fd);
	return fd;
}

/* accept client */
int
netaccept(int fd, char *server, int *port)
{
	int cfd, one;
	struct sockaddr_in sa;
	uchar *ip;
	socklen_t len;

	fdwait(fd, 'r');
	len = sizeof sa;
	if((cfd = accept(fd, (void*)&sa, &len)) < 0){
		perror("accept");
		return -1;
	}
	if(server){
		ip = (uchar*)&sa.sin_addr;
		snprint(server, 16, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	}
	if(port){
		*port = ntohs(sa.sin_port);
	}
	fdnoblock(cfd);
	one = 1;
	setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof one);
	return cfd;
}

/* connect to a remote service */
int
netdial(char *server, int port)
{
	int fd, n;
	uint32_t ip;
	struct sockaddr_in sa;
	socklen_t sn;

	if(netlookup(server, &ip) < 0)
		return -1;
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("socket");
		return -1;
	}
	fdnoblock(fd);
	memset(&sa, 0, sizeof sa);
	memcpy(&sa.sin_addr, &ip, 4);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	if(connect(fd, (struct sockaddr*)&sa, sizeof sa) < 0 && errno != EINPROGRESS){
		perror("connect");
		close(fd);
		return -1;
	}
	fdwait(fd, 'w');
	sn = sizeof sa;
	if(getpeername(fd, (struct sockaddr*)&sa, &sn) != -1){
		return fd;
	}
	perror("getpeername");
	sn = sizeof sa;
	getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*)&n, &sn);
	if(n == 0)
		errno = ECONNREFUSED;	
	close(fd);
	return -1;
}
