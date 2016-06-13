/*
	http://www.cnblogs.com/xuxm2007/category/217480.html
	http://blog.csdn.net/cywosp/article/details/27316803
	http://www.hilyp.com/2016/04/
	https://www.owent.net/


*/



// server.cpp

#include "unp.h"

void str_echo1(int connfd)
{
	struct sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);

	if (getpeername(connfd, (SA *)&clientAddr, &len) < 0 )
	{
		return;
	}

	char lcBuffer[MAXLIENE] = {0};
	sprintf(lcBuffer,"%u Echo 1",clientAddr.sin_addr.s_addr);

	printf("%s\n",lcBuffer);
	write(connfd,lcBuffer,MAXLIENE);

}

void str_echo2(int connfd)
{
	struct sockaddr_in clientAddr;
	socklen_t len = sizeof(clientAddr);

	if (getpeername(connfd,(SA *)&clientAddr, &len) < 0)
	{
		return;
	}

	char lcBuffer[MAXLIENE] = {0};
	sprintf(lcBuffer,"%u Echo 2",clientAddr.sin_addr.s_addr);

	printf("%s\n",lcBuffer);
	write(connfd,lcBuffer,MAXLIENE);
}
 
int main(int argc, char **argv) 
{
	struct sockaddr_in cliaddr;
	pid_t childpid;

	int listenfd1 = socket(AF_INET, SOCK_STREAM,0);

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(1000);

	bind(listenfd1,(SA *)&servaddr, sizeof(servaddr));

	listen(listen,LISTENQ);

	int listenfd2 = socket(AF_INET,SOCK_STREAM, 0);

	struct sockaddr_in servaddr2;
	bzero(&servaddr2,sizeof(servaddr2));
	servaddr2.sin_family = AF_INET;
	servaddr2.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr2.sin_port = htons(1001);

	bind(listenfd2, (SA *)&servaddr2, sizeof(servaddr2));

	listen(listenfd2,LISTENQ);

	int maxfdp1 = max(listenfd1,listenfd2) + 1;
	fd_set rset;
	FD_ZERO(&rset);

	for( ; ; )
	{
		FD_SET(listenfd1, &rset);
		FD_SET(listenfd2, &rset);


		int nready = -1;
		if ( (nready = select(maxfdp1, &rset, NULL, NULL, NULL)) < 0 )
		{
			if (EINTR == errno) 
			{
				continue;
			}
			else
			{
				err_sys("Select error.\n");
			}
		}

		if (FD_ISSET(listenfd1,&rset) )
		{
			socklen_t len = sizeof(cliaddr);
			int connfd = accept(listenfd1,(SA *)&cliaddr, &len);
			
			if ( 0 == (childpid=fork()) )
			{
				Close(listenfd1);
				str_echo1(connfd);
				exit(0);
			}
			Close(connfd);
		}

		if (FD_ISSET(listenfd2, &rset))
		{
			socklen_t len = sizeof(cliaddr);
			int connfd = accept(listenfd2,(SA *)&cliaddr, &len);

			if ( 0 == (childpid = fork()) )
			{
				close(listenfd2);
				str_echo2(connfd);
				exit(0);
			}	

			close(connfd);
		}

	}

	exit(0);

}


// client_server.cpp

#include "unp.h"

int main( int argc, char **argv)
{
	int sockfd, n;
	char recvline[MAXLIENE+1];

	struct sockaddr_in servaddr;
	if (argc != 3)
		err_quit("usage: a.out <IPAddress> <IPPort>");

	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		err_sys("socket error!");

	int port = atoi(argv[2]);

	bzero(&servaddr,sieof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	if (inet_pton(AF_INET,argv[1],&servaddr.sin_addr) <= 0)
		err_quit("inet_pton error  for %s",argv[1]);

	if (connect(sockfd,(SA *)&servaddr,sizeof(servaddr)) < 0)
		err_sys("connect error!");

	printf("Connect Ok\n");

	while ( n = read(sockfd,recvline, MAXLIENE) > 0)
	{
		recvline[n] = 0;

		struct in_addr svraddr;
		svraddr.s_addr = strtoul(recvline, NULL, 10);
		char *pszsvraddr = inet_ntoa(svraddr);

		printf("%s : %s \n",pszsvraddr,recvline);
	}
	if (n < 0)
		err_sys("read error");

	exit(0);
}


// select_server.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>

#define backlog 5
const int port = 8888;

void select_test(int port, int backlog)
{
	int rcd;
	int new_cli_fd;
	int maxfd;
	int socklen, server_len;
	int ci;
	int watch_fd_list[backlog+1];
	for (ci = 0; ci <= backlog; ci ++)
	{
		watch_fd_list[i] = -1;
	}

	int server_sockfd;

	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (server_sockfd == -1)
	{
		printf("create socket error!\n");
		exit(1);
	}

	if (fcntl(server_sockfd,F_SETFL,O_NONBLOCK) == -1)
	{
		printf("set server socket nonblock!\n");
		exit(1);
	}

	struct sockaddr_in server_sockaddr;
	memset(&server_sockaddr, 0, sizeof(server_sockaddr));
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sockaddr.sin_port = htons(port);
	server_len = sizeof(server_sockaddr);

	// 绑定 
	rcd = bind(server_sockfd,(struct sockaddr *)&server_sockaddr,server_len);
	if (rcd == -1)
	{
		printf("bind port %d error!\n",ntohs(server_sockaddr.sin_port));
		exit(1);
	}

	// 监听
	rcd = listen(server_sockfd,backlog);
	if (rcd == -1)
	{
		printf("listen error!\n");
		exit(1);
	}
	printf("server is waitinf on socket=%d\n",server_sockfd);

	watch_fd_list[0] = server_sockfd;
	maxfd = server_sockfd;

	fd_set watchset;
	FD_ZERO(&watchset);
	FD_SET(server_sockfd,&watchset);

	struct timeval tv;
	struct sockaddr_in cli_sockaddr;
	while (1) {

		// 设置select等待的最大时间是20秒
		tv.tv_sec = 20;
		tv.tv_usec = 0;
		// 每次都要重新设置集合才能激发事件
		FD_ZERO(&watchset);
		FD_SET(server_sockfd, &watchset);

		// 对已存在的socket重新设置
		for (ci = 0; ci <= backlog; ci ++ )
		{
			if (watch_fd_list[ci] != -1)
				FD_SET(watch_fd_list[ci], &watchset);
		}

		rcd = select(maxfd + 1, &watchset, NULL, NULL, &tv);
		switch (rcd)
		{
			case -1:
				printf("Select error!\n");
				exit(1);
			case 0:
				FD_ZERO(&watchset);
				for (ci = 1; ci <= backlog; ci ++)
				{
					shutdown(watch_fd_list[ci], 2);
					watch_fd_list[ci] = -1;
				}

				FD_CLR(server_sockfd, &watchset);
				FD_SET(server_sockfd, &watchset);
				continue;
			default:
				// new connection 
				if (FD_ISSET(server_sockfd, &watchset))
				{
					socklen = sizeof(cli_sockaddr);
					new_cli_fd = accept(server_sockfd,
						(struct sockaddr *)&cli_sockaddr,&socklen);
					if (new_cli_fd < 0)
					{
						porintf("Accept error!\n");
						exit(1);
					}
					printf("\nopen communication with Client %s on socket %d\n",
						inet_ntoa(cli_sockaddr.sin_addr),new_cli_fd);

					for (ci = 1; ci <= backlog; ci ++ )
					{
						if ( watch_fd_list[ci] == -1)
						{
							watch_fd_list[ci] = new_cli_fd;
							break;
						}
					}

					FD_SET(new_cli_fd, &watchset);
					if (maxfd < new_cli_fd)
					{
						maxfd = new_cli_fd;
					}
					continue;
				}
				else 
				{
					for (ci = 1; ci <= backlog; ci ++ )
					{
						if (watch_fd_list[ci] == -1)
							continue;
						if (!FD_ISSET(watch_fd_list[ci], &watchset))
							continue;

						char buffer[128];
						// 接收
						int len = recv(watch_fd_list[ci],buffer,128,0);
						if (len < 0)
						{
							printf("recv error!\n");
							exit(1);
						}
						buffer[len] = 0;

						// 获取客户端的IP地址
						struct sockaddr_in sockaddr;
						getpeername(watch_fd_list[ci], (struct sockaddr *)&sockaddr,
							sizeof(sockaddr));
						printf("read data [%s] from client %s on socket %d\n",buffer,
							inet_ntoa(sockaddr.sin_addr),watch_fd_list[ci]);

						// 发送接收到的数据
						len = send(watch_fd_list[ci], buffer, strlen(buffer), 0);

						if (len < 0)
						{
							printf("send error!\n");
							exit(1);
						}
						printf("write data [%s] to client %s on socket %d\n",buffer,inet_ntoa(sockaddr.sin_addr),watch_fd_list[ci]);

						shutdown(watch_fd_list[ci], 2);
						watch_fd_list[ci] = -1;
						FD_CLR(watch_fd_list[ci], &watchset);

						// 接收到的关闭命令
						if (strcmp(buffer,"quit") == 0) {
							for (ci = 0; ci <= backlog; ci ++)
							{
								if (watch_fd_list[ci] != -1) {
									shutdown(watch_fd_list[ci],2);
								}
							}
							printf("\nWeb Server Quit!\n");
							exit(0);
						}
					}
					break;
				}
		}
	}
}

int main()
{
	select_test(port,backlog);
	return 0;
}

// select_client.cpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int client_tcp(char *serIP, in_port_t serPort,char *data)
{
	// 创建socket
	int sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (sock < 0)
	{
		printf("socket error!");
		exit(0);
	}

	// 填充sockaddr_in
	struct sockaddr_in serAddr;
	memset(serAddr, 0, sizeof(serAddr) );
	serAddr.sim_family = AF_INET;
	serAddr.sin_port = htons(serPort);
	int rtn = inet_pton(AF_INET, serIP, &serAddr.sin_addr.s_addr);
	// serAddr.sin_addr.s_addr = inet_addr(serIP);
	if (rtn < 0)
	{
		printf("inet_pton error!");
		exit(0);
	}

	printf("目标服务器地址：%s: %d\n",inet_ntoa(seraddr.sin_addr),
		ntohs(serAddr.sin_port));
	printf("网络层协议: %s\n",serAddr.sin_family == 2? ""IPV4): "IPV6";
	printf("传输层协议: TCP \n");

	// connect to server
	if (connect(sock,(struct sockaddr *)&serAddr, sizeof(serAddr)) < 0)
	{
		printf("connect error!\n");
		exit(0);
	}

	// show the other side 
	printf("connected server %s : %d\n",inet_ntoa(serAddr.sin_addr),
		ntohs(serAddr.sin_port));

	// send data
	int bufsize = strlen(data);
	int num = send(sock,data,bufsize,0);
	if (num < 0)
	{
		printf("send error!\n");
		exit(0);
	}

	fputs("Received: ",stdout);
	char buffer[100];
	int n = recv(sock,buffer,100-1,0);
	if (n <= 0)
	{
		printf("Receive Error !!\n");
		exit(0);
	}
	else
	{
		buffer[n] = '\0';
		printf("%s\n",buffer);
	}

	// close fd
	close(sock);

	return 0;
}

int main(int atgc, char **argv)
{
	int port = 8888;
	client_tcp("127.0.0.1",port,"Hello Server1!");
	client_tcp("127.0.0.1",port,"Hello Server2!");
	client_tcp("127.0.0.1",port,"Hello Server3!");
	client_tcp("127.0.0.1",port,"quit");

	return 0;
}

// https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/
// epoll_server.c

static int create_and_bind(char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset(&hints,0,sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	s = getaddrinfo( NULL, port, &hints, &result);
	if (s != 0)
	{
		fprintf(stderr,"getaddrinfo : %s \n",gai_strerror(s));
		return -1;
	}
	for (rp = result; rp != NUULL; rp = rp->ai_next)
	{
		std = socket(rp->ai_family,
			rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;

		s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0)
			break;

		close(sfd);
	}
	if (rp == NULL)
	{
		fprintf(stderr, "Could not bind!\n");
		return -1;
	}
	freeaddrinfo(result);
	return sfd;
}

static int 
make_socket_non_blocking( int sfd )
{
	int flags, s;

	flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror("fcntl");
		return -1;
	}

	return 0;
}

#define MAXEVENTS 64

int 
main(int argc, char **argv)
{
	int sfd, s;
	int efd;
	struct epoll_event event;
	struct epoll_event *events;

	if (argc != 2)
	{
		fprintf(stderr,"Usage: %s [port]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	sfd = create_and_bind(argv[1]);
	if (sfd == -1)
		abort();

	s = make_socket_non_blocking(sfd);
	if (s == -1)
		abort();

	s = listen(sfd, SOMAXCONN);
	if (s == -1)
	{
		perror("listen");
		abort();
	}

	efd = epoll_create1(0);
	if (s == -1)
	{
		perror("epoll_create");
		abort();
	}

	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
	if (s == -1)
	{
		perror("epoll_ctl");
		abort():
	}

	// buffer where events are returned 
	events = calloc(MAXEVENTS, sizeof event);

	// the event loop
	while(1)
	{
		int n, i;
		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i ++)
		{
			if ((events[i].events & EPOLLERR) || 
				(events[i].events & EPOLLHUP) || 
				(!events[i].events & EPOLLIN)) {
				/* An error had occured on this fd, or 
					the socket is not ready for reading (why were we notified then?)
				*/
				fprintf(stderr,"epoll error!\n");
				close(events[i].data.fd);
				continue;
			}
			else if (sfd == events[i].data.fd)
			{
				/*We have a notifivation on the listening socket,Which 
				means one or more incoming connections.*/
				while(1)
				{
					struct sockaddr in_addr;
					socklen_t in_len;
					int infd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

					in_len = sizeof in_addr;
					infd = accept(sfd,&in_addr,&in_len);
					if (infd == -1)
					{
						if (errno == EAGAIN || errno == EWOULDBLOCK)
						{
							/*we have processed all incoming connections.*/
							break;
						}
						else 
						{
							perror("accept");
							break;
						}
					}
					s = getnameinfo(&in_addr,in_len,hbuf,sizeof hbuf,
							sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV);

					if (s == 0)
					{
						printf("Accepted connection on descriptor %d,
							host = %s, port = %d\n",infd,hbuf,sbuf);
					}

					s = make_socket_non_blocking(infd);
					if (s == -1)
						abort();

					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					s = epoll_ctl(efd,EPOLL_CTL_ADD, infd,&event);
					if ( s == -1)
					{
						perror("epoll_ctl");
						abort();
					}
				}
				continue;
			}
			else 
			{
				int done = 0;
				while(1)
				{
					ssize_t count;
					char buf[512];

					count = read(events[i].data.fd, buf, sizeof buf);
					if (count == -1)
					{
						if (errno != EAGAIN)
						{
							perror("read");
							done = 1;
						}
						break;
					}
					else if (count == 0)
					{
						done = 1;
						break;
					}

					s = write(1, buf, count);
					if (s == -1)
					{
						perror("write");
						abort();
					}
				}
				if(done)
				{
					printf("closed connection on descriptor %d\n",
						events[i].data.fd);

					close(events[i].data.fd);
				}
			}

		}
	}

	free(events);
	close(sfd);

	return EXIT_SUCCESS;
}


// http://www.cnblogs.com/xuxm2007/archive/2011/08/18/2144980.html

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define KEY 1234
#define SIZE 1024
#define PORT 9999
#define MAXFDS 5000
#define EVENTSIZE 100

void process();
int fd, cfd, opt = 1;
int shmid;
char *shmaddr;
struct shmid_ds buf;
int num = 0;

int main(int argc, char *argv[]) {
	// 建立内存共享
	shmid = shmget(KEY,SIZE,IPC_CREATE|0600);
	if (shmid == -1) {
		printf("create share memory failed.\n");
	}
	shmaddr = (char *)shmat(shmid,NULL,0);
	if (shmaddr == (void *)-1) {
		printf("connect to the share memory failed.\n");
	}
	strcpy(shmaddr,"1\n");

	struct sockaddr_in sin, cin;
	socklen_t sin_len = sizeof(struct sockaddr_in);
	if ( (fd = socket(AF_INET,SOCK_STREAM, 0) ) <= 0 )
	{
		fprintf(stderr,"socket failed.\n");
		return -1;
	}
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons((short)(PORT));
	sin.sin_addr.s_addr = INADDR_ANY;
	if (bind(fd,(struct sockaddr *)&sin,sizeof(sin)) != 0) {
		fprintf(stderr,"bind failed.\n");
		return -1;
	}
	if (listen(fd, 32) != 0) {
		fprintf(stderr,"listen failed.\n");
		return -1;
	}
	int i;
	for (i = 0; i < 2; i ++) {
		int pid = fork();
		if (pid == 0) {
			process();
		}
	}
	while(1);

	return 0;
}

void process() 
{
	struct epoll_event ev;
	struct epoll_event events[1000];
	int kdpfd = epoll_create(1000);
	int len = sizeof(struct sockaddr_in);
	ev.events = POLLIN | EPOLLET;
	ev.data.fd = fd;
	int new_fd;

	if ( (fcntl(fd, F_GETFL, 0)&O_NONBLOCK)) {
		printf("ok non block\n");
	} else {
		printf("wrong non block.\n");
	}

	printf("sub socket is %d\n",fd);

	if (epoll_ctl(fdpfd,EPOLL_CTL_ADD,fd,&ev) < 0 ) {
		frpitf(stderr,"epoll set insertion error: fd = %d\n",fd);
		return ;
	} else {
		printf("监听 socket 加入 epoll 成功.\n");
	}

	struct sockaddr_in my_addr, their_addr;

	while (1) {
		/* 等待事情发生 */
		int nfds = epoll_wait(fdpfd, events, 20, 500);
		if (nfds == -1) {
			perror("epoll_wait.\n");
			break;
		}

		/*处理所有 事件*/
		int n;
		for (n = 0; n < nfds; ++ n) {
			if (events[n].data.fd == fd) {
				new_fd = accept(fd, (struct sockaddr_in *)&their_addr,&len);
				if (new_fd < 0) {
					printf("accept error.\n");
					continue;
				} else {
					printf("%d create new socket: %d\n",getpid(),new_fd);
				}
			}
		}

	}
}

// 郭无心  知乎 
// Linux下的echo服务器 
// https://zhuanlan.zhihu.com/p/20303550?refer=guowuxin-study
/*
	1) 多进程阻塞的echo服务器 
	2) 多线程阻塞的echo服务器 
	3) select IO复用的echo服务器
*/
// server_multiprocess.cpp
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

#define LISTEN_PORT 8080
void runchild_echo(int sockfd)
{
	ssize_t n;
	char line[512];

	printf("ready to read.\n");
	while ( (n = read(sockfd, line, 512)) > 0 ) {
		line[n] = '\0';
		printf("Client : %s \n",line);
		bzero(&line,sizeof(line));
	}
	printf("end read.\n");
}

int main(int argc, char *argvp[]) {
	int listenfd, connfd;
	pid_t pid;
	socklen_t chilen;

	struct sockaddr_in chiaddr, servaddr;

	if ((listenfd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
		printf("socket established error: %s\n",(char *)strerror(errno));
		return -1;
	}

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(LISTEN_PORT);

	if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) == -1) {
		printf("bind error: %s\n",strerror(errno));
		return -1;
	}

	listen(listenfd, 5);
	while(1)
	{
		clilen = sizeof(chiaddr);
		connfd = accept(listenfd, (struct sockaddr *)&chiaddr, &chilen);
		if (connfd == -1) {
			printf("accept client error: %s\n",strerror(errno));
			return -1;
		} else {
			printf("client  connected.\n");
		}

		// 创建新进程的系统函数调用fork()
		pid = fork();
		if (pid == 0)  // child process
		{
			close(listenfd);
			printf("client from %s\n",inet_ntoa(chiaddr.sin_addr));
			runchild_echo(connfd);
			exit(0);
		} else if (pid < 0)
		{
			printf("fork error: %s\n",strerror(errno));
		}
		else {  // 父进程中返回
			close(connfd);
		}
	}

}

// client_multiprocess.cpp
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 8080

void str_cli(char *data, int sockfd) {
	char recv[512];

	int wc = write(sockfd, data, strlen(data));

	exit(0);
}

int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in servaddr;

	if (argc != 3)
		return -1;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd==-1)
		printf("socket established error : %s \n",(char*)strerror(errno));

	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET,argv[1],&servaddr.sin_addr);

	printf("client try to connect.\n");
	int conRes = connect(sockfd,(struct sockaddr *)&servaddr,sizeof
		servaddr);
	if (conRes == -1) {
		printf("connect error: %s \n",strerror(errno));
		return -1;
	}

	str_cli(argv[2], sockfd);

	exit(0);
}

// 多线程的echo服务器(采用read阻塞)
// server_multithread.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
using namespace std;

#define SERVERIP "192.168.1.117"
#define SERVERPORT 12345
#define MAXBUFFER 256

// 线程
pthread_t ntid;
// 客户端连接的connfd
int connfd;

struct sockaddr_in clientaddr;

void *printContext(void *arg) {
	char ip[40] = {0};
	char readBuf[MAXBUFFER] = {0};
	int ret;

	// 线程退出时，可以清理内存
	pthread_detach(ntid);

	//客户端连接的ID(main函数中的accept的返回值) 
    //线程自己要保存连接符 ID，因为进程在第二个客户端 
    //连接后，会覆盖connfd 
    int pconnfd = connfd;

    printf("%s\n",inet_ntop(AF_INET,&clientaddr.sin_addr,ip,
    	sizeof(ip)));
    cout << "connected to the server." << endl;

    while (ret = read(pconnfd,readBuf,MAXBUFFER))
    {
    	write(pconnfd,readBuf,MAXBUFFER);
    	printf("%s\n",readBuf);
    	bzero(readBuf,MAXBUFFER);
    }
    if (ret == 0)
    	printf("the connection of client is close.!\n");
    else
    	printf("read error: %s\n",strerror(errno));
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
	socklen_t len;
	int serverFd, ret;
	struct sockaddr_in serveraddr;

	serverFd = socket(AF_INET,SOCK_STREAM,0);
	if (serverFd < 0) {
		prtintf("socket error: %s\n",strerror(errno));
		exit(-1);
	}

	bzero(&serveraddr,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);
	// 将C字节转化成网络字节序
	inet_pton(AF_INET,SERVERIP,&serveraddr.sin_addr);

	ret = bind(serverFd,(struct sockaddr *)&serveraddr,sizeof(serveraddr));
	if (ret != 0) {
		close(serverFd);
		printf("bind error:%s\n",strerror(errno));
		exit(-1);
	}

	ret = listen(serverFd, 5);
	if (ret != 0) {
		close(serverFd);
		printf("listen error: %s\n",strerror(errno));
		exit(-1);
	}

	// clientaddr 清零
	len = sizeof(clientaddr);
	bzero(&clientaddr,sizeof(clientaddr));
	while (1) {
		/* 接受客户端的连接，然后启动线程去处理客户
		端的请求。线程只要保存connfd就好了 进程在第二个客户端连接进来的时候 会
		覆盖第一个客户端的connfd*/
		connfd = accept(serverFd,(struct sockaddr *)&clientaddr,
			&len);
		if (connfd < 0) {
			printf("accept error: %d\n",strerror(errno));
			continue;
		}

		int err;
		err = pthreaf_create(&ntid,NULL,printContext,NULL);
		if (err != 0) {
			cout << "Cann't create pthread" << endl;
		}
		// close(connfd);
	}
	close(serverFd);

	return 0;
}

// client_multithread.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVERIP "192.168.1.117"
#define SERVERPORT 12345
#define MAXBUFFER 256

int main(int argc, char *argv[]) {
	int clientFd, ret;
	struct sockaddr_in serveraddr;
	char buf[MAXBUFFER];

	// 创建socket
	clientFd = socket(AF_INET,SOCK_STREAM,0);

	if (clientFd < 0) {
		printf("socket error: %s\n",strerror(errno));
		exit(-1);
	}

	bzero(&serveraddr,sizeof(serveraddr));
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVERPORT);
	inet_pton(AF_INET,SERVERIP,&serveraddr.sin_addr);

	// connect to server.
	ret = connect(clientFd,(struct sockaddr *)&serveraddr,
		sizeof(serveraddr));
	if (ret != 0) {
		close(clientFd);
		printf("connect error: %s\n",strerror(errno));
		exit(-1);
	}
	// 接收控制台输入send到服务端，并接收服务端的返回
	while(1) {
		bzero(buf,sizeof(buf));
		scanf("%s",&buf);
		write(clientFd,buf,sizeof(buf)); 
		bzero(buf,sizeof buf);
		reaf(clientFd,buf,sizeof(buf));
		prtintf("%s\n",buf);
	}
	close(clientFd);
	return (EXIT_SUCCESS);
}


// select IO 复用echo服务器

//server_select.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>

#define backlog 5
const int port = 8888;

void select_test(int port,
	int backlog) 
{
	int rcd;
	int new_cli_fd;
	int maxfd;
	int socklen, server_len;
	int ci;
	int watch_fd_list[backlog+1];
	for (ci = 0; i <= backlog; ++ ci)
		watch_fd_list[ci] = -1;

	int server_sockfd;
	server_sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (server_sockfd == -1)
	{
		printf("create server_socket error!\n");
		exit(-1);
	}

	// set nonblock
	if (fcntl(server_sockfd,F_SETFL,O_NONBLOCK) == -1) {
		printf("set server socket nonblock failed.\n");
		exit(-1);
	}

	struct sockaddr_in server_sockaddr;
	memset(&server_sockaddr,0,sizeof(server_sockaddr));
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sockaddr.sin_port = htons(port);
	server_len = sizeof(server_sockaddr);

	rcd = bind(server_sockfd,(struct sockaddr *)&server_sockaddr,
		sizeof(server_sockaddr));
	if (rcd == -1) {
		printf("bind port %d error!\n",ntohs(server_sockaddr.sin_port));
		exit(-1);
	}

	rcd = listen(server_sockfd,backlog);
	if (rcd == -1) {
		printf("listen error!\n");
		exit(-1);
	}

	printf("server is waiting on socket=%d\n",server_sockfd);

	watch_fd_list[0] = server_sockfd;
	maxfd = server_sockfd;
	fd_set watchset;
	FD_ZERO(watchset);
	FD_SET(server_sockfd,&watchset);

	struct timeval tv;
	struct sockaddr_in cli_sockaddr;
	while (1) {
		tv.tv_sec = 20;
		tv.tv_usec = 0;

		FD_ZERO(&watchset);
		FD_SET(server_sockfd,&watchset);

		for( ci = 0; ci <= backlog; ci ++ )
		{
			if (watch_fd_list[ci] != -1)
				FD_SET(watch_fd_list[ci],&watchset);
		}

		rcd = select(maxfd + 1, &watchset,NULL, NULL, &tv);
		switch(rcd)
		{
			case -1:
				printf("select error!\n");
				exit(1);
			case 0:
				printf("select time out!\n");
				//超时则清理掉所有集合元素并关闭所有与客户端的socket
				FD_ZERO(&watchset);
				for(ci=1;ci<=backlog;ci++)
				{
					shutdown(watch_fd_list[ci],2);
					watch_fd_list[ci] = -1;
				}
				//重新设置监听socket，等待链接
				FD_CLR(server_sockfd, &watchset);
				FD_SET(server_sockfd, &watchset);
				continue;
			default:
				//检测是否有新连接建立
				if (FD_ISSET(server_sockfd,&watchset)) {
					socklen = sizeof(cli_sockaddr);
					new_cli_fd = accept(server_sockfd,
						(struct sockaddr *)&cli_sockaddr,&socklen);
					if(new_cli_fd < 0) {
						printf("accept error!\n");
						exit(-1);
					}

					printf("\nopen communnication with client %s on socket %d\n",
						inet_ntoa(cli_sockaddr.sin_addr), new_cli_fd);

					for(ci = 1; ci <= backlog; ci ++ ) {
						if (watch_fd_list[i] == -1) {
							watch_fd_list[i] = new_cli_fd;
							break;
						}
					}

					FD_SET(new_cli_fd,&watch_fd_list);
					if (maxfd < new_cli_fd )
						maxfd = new_cli_fd;
					continue;

				} else {//已有连接的数据通信
						//遍历每个设置过的集合元素
						for (ci = 1; ci <= backlog; ci ++ )
						{
							if (watch_fd_list[ci] == -1)
								continue;
							if (!FD_SET(watch_fd_list[i],
								&watchset))
							{
								continue;
							}

							char buffer[128] = {0};
							int len = recv(watch_fd_list[ci],buf,sizeof(buf));
							if(len < 0) {
								printf("recv error!\n");
								exit(-1);
							}
							buffer[len] = 0;

							//获得客户端的IP地址
							struct sockaddr_in sockaddr;
							getpeername(watch_fd_list,(struct sockaddr *)&sockaddr,
								sizeof(sockaddr));
							printf("read data [%s] from client %s on socket %d\n",
								buffer,inet_ntoa(sockaddr.sin_addr),watch_fd_list[ci]);

							//发送接收到到数据
							len = send(watch_fd_list[ci], buffer, strlen(buffer), 0);
							if (len < 0) {
								printf("Send error\n");
								exit(1);
							}

							printf("write data [%s] to client %s on socket %d.\n",
								buffer,inet_ntoa(sockaddr.sin_addr),watch_fd_list[ci]);

							shutdown(watch_fd_list[ci],2);
							FD_CLR(watch_fd_list[ci], &watchset);
							watch_fd_list[ci] = -1;

							if (strcmp(buffer,"quit") == 0) {
								for ( ci=0; ci <= backlog; ci ++ )
								{
									if (watch_fd_list[ci] != -1)
									{
										shutdown(watch_fd_list[ci], 2);
									}
									printf("\nweb server quit!\n");
									exit(0);
								}
							}
						}

				}
		}
	}
}

int main()
{
	select_test();
	return 0;
}


// client _select.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int client_tcp(char *serIp,in_port_t serPort,char *data){

}

/* main.cc
*  Created on: 2009-11-30
*      Author: liheyuan
*    Describe: epoll实现阻塞模式服务器(Echo服务器)
*
*   Last Date: 2009-11-30
*   CopyRight: 2009 @ ICT LiHeyuan
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>

#define EPOLL_SIZE 10
#define EVENT_ARR 20
#define BACK_QUEUE 10
#define PORT 18001
#define BUF_SIZE 16

void setnonblocking(int sockfd)
{
	int opt ;

	opt = fcntl(sockfd, F_GETFL);
	if (opt < 0) {
		printf("fcntl(F_GETFL fail.");
		exit(1);
	}

	opt |= O_NONBLOCK;
	if (fcntl(sockfd,F_SETFL, opt) < 0) {
		printf("fcntl(F_SETFL) fail.");
		exit(-1);
	}
}

int main() {
	int serverFd;
	//创建服务器fd
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	setnonblocking(serverFd);

	//创建epoll，并把serverFd放入监听队列
	int epFd = epoll_create(EPOLL_SIZE);
	struct epoll_event ev, evs[EVENT_ARR];
	ev.data.fd = serverFd;
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epFd, EPOLL_CTL_ADD, serverFd, &ev);
	//绑定服务器端口
	struct sockaddr_in serverAddr;
	socklen_t serverLen = sizeof(struct sockaddr_in);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(PORT);
	if (bind(serverFd, (struct sockaddr *) &serverAddr, serverLen)) {
		printf("bind() fail.\n");
		exit(-1);
	}

	//打开监听
	if (listen(serverFd, BACK_QUEUE)) {
		printf("Listen fail.\n");
		exit(-1);
	}

	//死循环处理
	int clientFd;
	sockaddr_in clientAddr;
	socklen_t clientLen;
	char buf[BUF_SIZE];
	while (1) {
	//等待epoll事件的到来，最多取EVENT_ARR个事件
		int nfds = epoll_wait(epFd, evs, EVENT_ARR, -1);
		//处理事件
		for (int i = 0; i < nfds; i++) {
			if (evs[i].data.fd == serverFd && evs[i].data.fd & EPOLLIN) {
			//如果是serverFd,表明有新连接连入
				if ((clientFd = accept(serverFd,
					(struct sockaddr *) &clientAddr, &clientLen)) < 0) {
					printf("accept fail.\n");
			}
			printf("Connect from %s:%d\n", inet_ntoa(clientAddr.sin_addr),
				htons(clientAddr.sin_port));
			setnonblocking(clientFd);
			//注册accept()到的连接
			ev.data.fd = clientFd;
			ev.events = EPOLLIN | EPOLLET;
			epoll_ctl(epFd, EPOLL_CTL_ADD, clientFd, &ev);
		} else if (evs[i].events & EPOLLIN) {
			//如果不是serverFd,则是client的可读
			if ((clientFd = evs[i].data.fd) > 0) {
					//先进行试探性读取
					int len = read(clientFd, buf, BUF_SIZE);
					if (len > 0) {
					//有数据可以读，Echo写入
						do {
							if (write(clientFd, buf, len) < 0) {
								printf("write() fail.\n");
							}
							len = read(clientFd, buf, BUF_SIZE);
						} while (len > 0);
					} else if (len == 0) {
					//出发了EPOLLIN事件，却没有可以读取的，表示断线
						printf("Client closed at %d\n", clientFd);
	                    epoll_ctl(epFd, EPOLL_CTL_DEL, clientFd, &ev);
	                    close(clientFd);
	                    evs[i].data.fd = -1;
	                    break;
	                } else if (len == EAGAIN) {
	                    continue;
	                } else {
	                    //client读取出错
	                    printf("read() fail.");
	                }
            	}
        	} else {
            	printf("other event.\n");
       	 	}
    	}
	}
return 0;
}


//  thread_create_key.cpp

#include <stdio.h>
#include <string.h>
#include <pthread.h>

pthread_key_t key;

void *thread2(void *arg) {
	int tsd = 5;

	printf("thread %d is running \n",pthread_self());
	pthread_setspecific(key,(void *)tsd);
	printf("thread %d returns %d\n",pthread_slef(),pthread_getspecific(key));

}

void *thread1(void *arg) {
	int tsd = 0;
	pthread_t thid2;

	printf("thread %s is running \n",pthread_slef());
	pthread_setspecific(key,(void *)tsd);
	pthread_create(&thid2,NULL,thread2,NULL);
	sleep(2);
	printf("thread %d return %d\n",pthread_self(),pthread_getspecificv());
}

int main(int argc, char *argv[]) {

	pthread_t thid1;
	printf("main thread begins runnings.\n");
	pthread_key_create(&key, NULL);

	pthread_create(&thid1,NULL,thread1,NULL);
	sleep(5);

	pthread_key_delete(key);
	printf("main thread exit!\n");
	return 0;
}