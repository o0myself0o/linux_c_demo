
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

void *testThreadPool(int *t);
pthread_mutex_t clifd_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clifd_cond = PTHREAD_COND_INITIALIZER;
int a = 0;

int main()
{
	int sock_fd, conn_fd;
	int optval;
	socklen_t cli_len;
	struct sockaddr_in cli_addr, serv_addr;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(sock_fd < 0) {
		printf("socket error\n");
	}

	optval = 1;
	if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval,
		sizeof(int)) < 0) {
		printf("setsockopt error\n");
	}

	memset(&serv_addr, 0, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(5556);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sock_fd, (struct sockaddr *)&serv_addr,
		sizeof(struct sockaddr_in)) < 0) {
		printf("bind error\n");
	}

	if(listen(sock_fd, 100) < 0) {
		printf("listen error\n");
	}

	cli_len = sizeof(struct sockaddr_in);
	int t;
	pthread_t *mythread;
	mythread = (pthread_t *)malloc(100 * sizeof(pthread_t));

	for(t = 0; t < 5; t++) {
		int *i = (int *)malloc(sizeof(int));
		*i = t;
		if(pthread_create(&mythread[t], NULL, (void *)testThreadPool, (void *)i) != 0) {
			printf("pthread_create error\n");
		}
	}

	while(1) {
		conn_fd = accept(sock_fd, (struct sockaddr *)&cli_addr, &cli_len);
		if(conn_fd < 0) {
			printf("accept error\n");
		}

		printf("accept a new client, ip:%s\n", inet_ntoa(cli_addr.sin_addr));
		pthread_mutex_lock(&clifd_mutex);
		a = conn_fd;
		pthread_cond_signal(&clifd_cond);
		pthread_mutex_unlock(&clifd_mutex);
	}

	return 0;
}

void *testThreadPool(int *t)
{
	printf("t is %d\n", *t);
	for(; ;) {
		pthread_mutex_lock(&clifd_mutex);
		pthread_cond_wait(&clifd_cond, &clifd_mutex);
		printf("a is %d\n", a);
		printf("t is %d\n", *t);
		pthread_mutex_unlock(&clifd_mutex);
		sleep(100);
	}

	return (void *)0;
}
