#include "sockconnect.h"

extern event_tree_t *eventTree;
extern pthread_pool_t *thread_pool;

void onConnection(void *arg);

int getListener(int port) {
    int lisfd;
    struct sockaddr_in servaddr;

    lisfd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    bzero(&servaddr, sizeof(servaddr));
    setsockopt(lisfd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    Bind(lisfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    Listen(lisfd, 10);

    return lisfd;
}

void acceptConnection(void *arg) {
    event_t *e = arg;

    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int connfd = Accept(e->fd, (struct sockaddr *)&client, &len);

    int flags = fcntl(connfd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(connfd, F_SETFL, flags);

	Pthread_mutex_lock(&_loggerLock);
	char name[128];
	log_info("connection from %s:%d", inet_ntop(AF_INET, &client.sin_addr, name, sizeof(name)), ntohs(client.sin_port));	
	Pthread_mutex_unlock(&_loggerLock);
    event_t *new = getNewEvent(connfd , EPOLLIN | EPOLLET, onConnection);  

    addEvent(eventTree, new);
}

void onConnection(void *arg) {
    event_t *e = arg;

    char buf[128] = {'\0'};
    int n = recv(e->fd, buf, sizeof(buf), MSG_PEEK);
    if (n < 0 || n == 0) {
	removeEvent(eventTree, e);
    } else {
	pthread_task_t task;
	task.task = dealHttp;
	task.arg = &e->fd;
	addTaskToThreadPool(thread_pool, &task);
    }
}
