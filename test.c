#include "utilities.h"

int main(int arg, char *argv[]) {
    int lisfd = getListener(9527);
printf("hahah");   
    eventTree = getEventTree(300); 
printf("hahah");   
    thread_pool = getThreadPool(10, 30, 50);
    for (int i = 0; i < 5; i++) {
	event_t *new = getNewEvent(lisfd, EPOLLIN, acceptConnection);
	
	addEvent(eventTree, new);
    }
    
    startEventListener(eventTree);
    return 0;
}
