#include "eventctl.h"

event_tree_t *eventTree;

event_tree_t *getEventTree(int activeEvent) {
    event_tree_t *tree = malloc(sizeof(event_tree_t));

    tree->event_active_num = activeEvent;
    tree->epoll_fd = epoll_create(10);
    tree->event_on_tree = 0;
    tree->active_event = malloc(sizeof(struct epoll_event)*activeEvent);
    pthread_mutex_init(&tree->epollfd_lock, NULL);    
    return tree;
}

void startEventListener(event_tree_t *eventTree) {
    while (1) {
	int ready = epoll_wait(eventTree->epoll_fd, eventTree->active_event, eventTree->event_active_num, -1);

	for (int i = 0; i < ready; i++) {
	    event_t *active = (event_t *)eventTree->active_event[i].data.ptr;
	    active->callBack(active->arg);
	}
    }
}

void addEvent(event_tree_t *eventTree, event_t *newEvent) {
    struct epoll_event tmp;
    tmp.data.ptr = newEvent;
    tmp.events = newEvent->events;

    Pthread_mutex_lock(&eventTree->epollfd_lock);
    epoll_ctl(eventTree->epoll_fd, EPOLL_CTL_ADD, newEvent->fd, &tmp);
    eventTree->event_on_tree++;
    Pthread_mutex_unlock(&eventTree->epollfd_lock);
}

void removeEvent(event_tree_t *eventTree, event_t *oldEvent) {
    Pthread_mutex_lock(&eventTree->epollfd_lock);
    Close(oldEvent->fd);
    epoll_ctl(eventTree->epoll_fd, EPOLL_CTL_DEL, oldEvent->fd, NULL);
    eventTree->event_on_tree--;
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
