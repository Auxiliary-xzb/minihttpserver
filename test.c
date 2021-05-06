#include "utilities.h"

int main(int arg, char *argv[]) {
    int lisfd = getListener(9527);  
    eventTree = getEventTree(300);    
    thread_pool = getThreadPool(10, 30, 50);

	event_t *new = getNewEvent(lisfd, EPOLLIN, acceptConnection);
	addEvent(eventTree, new);
  
    startEventListener(eventTree);
    return 0;
}
