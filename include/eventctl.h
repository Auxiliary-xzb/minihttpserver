#ifndef _EVENTCTL_H
#define _EVENTCTL_H
#include <sys/epoll.h>

#include "log.h"
#include "wrapsys.h"
#include "wrapsock.h"
#include "wrappthread.h"
#include "pthreadpool.h"

#define MAX_EVENT 1024

typedef struct event_tree_t{
	int epoll_fd;
	pthread_mutex_t epollfd_lock;
	int event_active_num;
	int event_on_tree;
	struct epoll_event *active_event;
}event_tree_t;

typedef struct event_t {
    int fd;
    int events;
    void (*callBack)(void *arg);
    void *arg;
}event_t;

event_tree_t *getEventTree(int maxEvent);
event_t *getNewEvent(int fd, int event, void *callBack);
void startEventListener(event_tree_t *eventTree);
void addEvent(event_tree_t *evetnTree, event_t *newTree);
void removeEvent(event_tree_t *eventTree, event_t *oldEvent); 

#endif
