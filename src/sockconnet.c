#include "eventctl.h"
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

    //这里使用了一个技巧，我让链接在可读的时候没有直接调用HTTP的
    //处理函数，而是让他们在可读的时候先调用onConnection函数去检
    //查它的请求头，识别它们是HTTP请求后才将服务器的HTTP处理函数
    //交给该链接处理。
    //这样做的好处是我可以直接将acceptConnection函数抽象化，让它
    //可以处理多种请求，这样就不用为每一种服务器请求都设计一个对
    //应的acceptConnection函数了。
    //而且这种情况下的完全就可以不让httptools拿到event_t结构，这
    //样系统的结构就灵活多了。
    event_t *new = getNewEvent(connfd , EPOLLIN | EPOLLET, onConnection);  
    addEvent(eventTree, new);
}

/*
 * 这里偷懒了，之后再补全。我没判断直接让它处理HTTP
 * 这里应该是先用recv的预读链接中的数据，判断是不是HTTP请求，是的话
 * 就将HTTP请求和当前链接的套接字的描述符都作为任务信息传递给线程池
 * */
void onConnection(void *arg) {
    event_t *e = arg;

    char buf[128] = {'\0'};
    int n = recv(e->fd, buf, sizeof(buf), MSG_PEEK);
    //出错和结束就直接关闭链接
    if (n < 0 || n == 0) {
	removeEvent(eventTree, e);
	Close(e->fd);
    } else {
	pthread_task_t task;
	task.task = dealHttp;
	task.arg = &e->fd;
	addTaskToThreadPool(thread_pool, &task);
	//removeEvent(eventTree, e);
    }
}
