//偷懒的原因在最后
#ifndef _EVENTCTL_H
#define _EVENTCTL_H
#include <sys/epoll.h>

#include "log.h"
#include "wrapsys.h"
#include "wrapsock.h"
#include "wrappthread.h"
#include "pthreadpool.h"

#define MAX_EVENT 1024

/*
 * epoll内部的实现为红黑树，所以epoll_ctl本身
 * 就是向树上挂载节点，只是该节点表示的是一个
 * 事件的结构。所以可以直接将epoll的操作当作
 * 操作一颗红黑树来理解就很简单了。
 * */
typedef struct event_tree_t{
	//epoll的标识符，也就是epoll_create的返回值
	int epoll_fd;
	//这个是epoll_wait返回的数组的最大值，也就是第三个参数
	int active_event_max;

	//多线程中需要操作同一个红黑树，所以需要一个锁
	pthread_mutex_t epollfd_lock;
	//保存用于epoll_wait存储数据的数组，这样可以
	//将多个红黑树分开。
	struct epoll_event *active_event;
}event_tree_t;

/*
 * 挂载到红黑树上的节点信息，包含当前该节点的描述符
 * 需要监听的时间，事件发生时的回调函数及其参数。
 * */
typedef struct event_t {
    int fd;
    int events;
    void (*callBack)(void *arg);
    void *arg;
}event_t;

//获取一颗红黑树用来挂载事件，参数为epoll_wait可接受的最大的返回值
//其实也就是第三个参数
event_tree_t *getEventTree(int maxEvent);

//获取一个挂载到红黑树上的节点
event_t *getNewEvent(int fd, int event, void *callBack);

//开启epoll的监听，就是在一个死循环中调用epoll_wait
void startEventListener(event_tree_t *eventTree);

//将要处理的事件节点挂载到红黑树上，就是调用epoll_ctr的add
void addEvent(event_tree_t *evetnTree, event_t *newTree);

//在每个描述符完成了监听事件之后都需要将该节点从树上摘下来
void removeEvent(event_tree_t *eventTree, event_t *oldEvent); 

#endif

/*
 * 这里我设置了两个全局变量：红黑树、线程池，这也是最为关键的两个结构，它们的存在
 * 会影响到挂载到红黑树上的节点的设计。当然，设置他们为全局的肯定是存在问题的，但是
 * 对于本程序来说这样做使得程序很简单，不用重新修改程序内部结构。
 * 首先说明在哪些地方需要这两个结构：
 *  1.红黑树：
 *	1.1 在创建一个红黑树时需要一个变量保存该结构，这里是没有问题的，使用全局变
 *	    量和局部变量都能完成保存工作。
 *	1.2 在将服务器的监听套接字挂载到上面获取的红黑树时需要该结构，这也是没问题
 *	    的，因为他们都在main中顺序完成。所以红黑树用全局变量或者局部变量都没问题。
 *	1.3 在接受链接时，即acceptConnection时需要将节点挂载到红黑树上。这时就出现了问题。
 *	    监听套接字只把本身信息挂载上树，acceptConnection在被调用时只拿到了包装监听
 *	    套接字的event_t，并没有树本身的信息。那么该函数就无法完成新节点的挂载了。
 *	    所以此时就设计到了修改挂载到红黑树上的结构的设计了。为了不使用全局的红黑树
 *	    就必须在挂载信息时将树的信息挂载上去，这样在新链接到来时acceptConnection才
 *	    会能获取到该结构，从而完成节点的挂载。
 *	1.4 在onConnection函数中，如果recv函数发生了错误，那么此时就不应该再继续监听
 *	    该描述符了而是直接将其从树上摘下。
 *	1.5 在链接的工作完成之后，对挂载在树上的节点的处理也需要树信息。
 *	    这也是本程序存在的一个问题，也不能说是问题应该说是我对服务器工作方式的理解不到位。
 *	    我不清楚是否应该在将任务丢给线程池后立刻将该节点从树上摘下，还是应该在用户
 *	    请求处理结束之后再摘下（或者更改监听事件）。这也是为什么我在onConnection中
 *	    注释了remove的原因。
 *	    如果要在处理完成之后再摘下的话，问题就便复杂了。我就应该确认交给线程池
 *	    的工作何时处理完。
 *
 *  线程池：
 *	1.  只有在onConnection确认了服务器工作之后我需要向线程池添加任务。
 *	    那么问题就来了，是为每一个红黑树创建一个线程池还是一个进程公用一个线程池。
 *	    如果是一个红黑树创建一个线程池，那么就需要将传递accpectConnection，从而
 *	    使得onConnection可以访问到特定树的专有线程池。
 *	    但其实我觉得一个进程拥有一个线程池就足够了，让它成为通用结构处理任何任务,
 *	    这就是为什么我封装了一个pthread_task_t结构的原因，这样不管什么任务只要
 *	    传递函数和参数，线程都能完成，只是说我的线程池性能不是很强，管理者线程
 *	    的管理方案没有设计好。
 *	    所以将线程池设计为全局变量其实并不是一个很坏的打算。
 *
 * 可以看到如果希望程序有多个红黑树时，就设计到挂载节点的设计问题。并且多个红黑树应该
 * 还会引发其他的问题，所以这里就偷懒下。
 *  
 * */
