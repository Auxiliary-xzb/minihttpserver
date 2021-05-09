#include "eventctl.h"

//这里偷懒了，解释看eventctl.h
event_tree_t *eventTree;

event_tree_t *getEventTree(int maxEvent) {
    //根据用户给定的数值创建用于epoll_wait接受信息的
    //节点数量。
    event_tree_t *tree = malloc(sizeof(event_tree_t));

    tree->active_event_max = maxEvent;
    tree->epoll_fd = epoll_create(10);
    tree->active_event = malloc(sizeof(struct epoll_event)*maxEvent);
    pthread_mutex_init(&tree->epollfd_lock, NULL);    
    return tree;
}

void startEventListener(event_tree_t *eventTree) {
    while (1) {
	//epoll_wait返回的就是我们挂载到树上的信息，所以ptr部
	//分就可以拿到挂载上去的事件信息了。
	int ready = epoll_wait(eventTree->epoll_fd, eventTree->active_event, eventTree->active_event_max, -1);

	//遍历可以处理的事件，然后调用它们实现准备好的处理函数
	for (int i = 0; i < ready; i++) {
	    event_t *activeEvent = (event_t *)eventTree->active_event[i].data.ptr;
	    activeEvent->callBack(activeEvent->arg);
	}
    }
}

void addEvent(event_tree_t *eventTree, event_t *newEvent) {
    struct epoll_event tmp;

    //指针部分想挂什么就挂什么，所以可以将事件结构挂载上去
    //这样epoll_wait返回时就可以拿到这个事件结构了。
    tmp.data.ptr = newEvent;
    tmp.events = newEvent->events;

    Pthread_mutex_lock(&eventTree->epollfd_lock);
    epoll_ctl(eventTree->epoll_fd, EPOLL_CTL_ADD, newEvent->fd, &tmp);
    Pthread_mutex_unlock(&eventTree->epollfd_lock);
}

void removeEvent(event_tree_t *eventTree, event_t *oldEvent) {
    Pthread_mutex_lock(&eventTree->epollfd_lock);
    Close(oldEvent->fd);
    epoll_ctl(eventTree->epoll_fd, EPOLL_CTL_DEL, oldEvent->fd, NULL);
    free(oldEvent);
    Pthread_mutex_unlock(&eventTree->epollfd_lock);
}

event_t *getNewEvent(int fd, int event, void *callBack) {
    event_t *new = malloc(sizeof(event_t));
    new->arg = new;
    new->callBack = callBack;
    new->fd = fd;
    new->events = event;
    return new;
}
