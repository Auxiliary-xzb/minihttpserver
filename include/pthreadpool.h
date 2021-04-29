#ifndef _PTHREADPOOL_H
#define _PTHREADPOOL_H
#include <strings.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/resource.h>

#include "log.h"
#include "wrappthread.h"

typedef struct pthread_task_t {
    void (*task)(void *arg);
    void *arg;  
}pthread_task_t;

typedef struct pthread_pool_t {
    //线程池允许的最小线程数
    int min_thr_num;
    //线程池允许的最大线程数
    int max_thr_num;
    //线程池中忙碌的线程数
    int bsy_thr_num;
    //线程池中活动的线程数，因为线程数
    //会根据进程中线程的使用情况来扩张
    //和收缩，那么存活的线程量就不一定
    //和最大线程量相同了
    int alv_thr_num;
    //分别锁住bsy_thr_num
    pthread_mutex_t mutex_bsy;

    //线程池由数组实现，因此要存储首地址
    pthread_t *pthreads;
    //管理线程用来执行扩容和收缩工作
    pthread_t adjust_thread;
    //该条件变量用于通知管理线程
    pthread_cond_t do_adjust;
    //增大和减少时的步长
    int step;

    //任务请求队列（循环队列），用户只需要负责将请求
    //来，由线程池保存。之后的调用工作则
    //于用户无关
    pthread_task_t *task_list;
    //任务队列的最大值
    int max_que_size; 
    //任务对了当前含有的任务量
    int cur_que_size;
    //任务队列的头指针
    int front_queue;
    //任务队列的尾指针
    int rear_queue;
    //条件队列涉及到拿出和放入就是生产者
    //和消费者模型，因此需要两个条件变量
    //一个告知可以拿了，一个告知可以放了
    pthread_cond_t queue_not_empty;    
    pthread_cond_t queue_not_full;
    
    pthread_mutex_t self_lock;
}pthread_pool_t;

pthread_pool_t *getThreadPool(int thrmin, int thrmax, int quemax);
void addTaskToThreadPool(pthread_pool_t *_threadPool, pthread_task_t *task);  

#endif
