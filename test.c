#include "utilities.h"

int main(int arg, char *argv[]) {
    //这里应该接受一个参数，让服务器只将那一个目录
    //作为服务器的工作目录，但是我之前好像在chdir的
    //时候失败了，也没有去追究原因，因为做Qt去了。
    int lisfd = getListener(9527);  
    eventTree = getEventTree(300);    
    thread_pool = getThreadPool(10, 30, 50);
    
    event_t *new = getNewEvent(lisfd, EPOLLIN, acceptConnection);
    addEvent(eventTree, new);
  
    startEventListener(eventTree);
    return 0;
}
