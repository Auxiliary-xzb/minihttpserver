#include "pthreadpool.h"

extern pthread_mutex_t _loggerLock;

pthread_pool_t *thread_pool;

static int maxThreadLimited;

static void *waitThreadTask(); 
static void *adjustThreadPool();

pthread_pool_t *getThreadPool(int thrmin, int thrmax, int quemax) {
    pthread_pool_t *_threadPool = malloc(sizeof(pthread_pool_t));
    _threadPool->bsy_thr_num = 0;
    _threadPool->alv_thr_num = thrmin;
    _threadPool->max_thr_num = thrmax;
    _threadPool->min_thr_num = thrmin;

    _threadPool->max_que_size = quemax;
    _threadPool->cur_que_size = 0;
    _threadPool->front_queue = 0;
    _threadPool->rear_queue = 0;

    if (thrmax < 1000)
	_threadPool->step = thrmax * 0.05;
    else 
	_threadPool->step = 30;

    struct rlimit limit;
    getrlimit(RLIMIT_NPROC, &limit);
    maxThreadLimited = limit.rlim_cur;

    Pthread_mutex_init(&_threadPool->self_lock, NULL);
    Pthread_mutex_init(&_threadPool->mutex_bsy, NULL);
    Pthread_cond_init(&_threadPool->queue_not_full, NULL);
    Pthread_cond_init(&_threadPool->queue_not_empty, NULL);
    Pthread_cond_init(&_threadPool->do_adjust, NULL);

    _threadPool->pthreads = (pthread_t *)malloc(sizeof(pthread_t)*thrmax);
    _threadPool->task_list = (pthread_task_t *)malloc(sizeof(pthread_task_t)*quemax);
    bzero(_threadPool->task_list, sizeof(pthread_task_t)*quemax);

    Pthread_create(&_threadPool->adjust_thread, NULL, adjustThreadPool, (void *)_threadPool);

    int i;
    for (i = 0; i < thrmin; i++) {
	Pthread_create(&_threadPool->pthreads[i], NULL, waitThreadTask, (void *)_threadPool);
    }

    return _threadPool;
}

void addTaskToThreadPool(pthread_pool_t * _threadPool, pthread_task_t *task) {
    Pthread_mutex_lock(&_threadPool->self_lock);

    //如果队列满了就等待
    if (_threadPool->cur_que_size == _threadPool->max_que_size) {
	Pthread_cond_wait(&_threadPool->queue_not_full, &_threadPool->self_lock);
    }
    //将任务添加到任务队列
    _threadPool->task_list[_threadPool->rear_queue].task = task->task;
    _threadPool->task_list[_threadPool->rear_queue].arg = task->arg;
    _threadPool->rear_queue = (_threadPool->rear_queue + 1) % _threadPool->max_que_size;
    _threadPool->cur_que_size++;
    //加入队列之后通知消费者可以从队列中拿去任务了
    Pthread_mutex_unlock(&_threadPool->self_lock);
    Pthread_cond_broadcast(&_threadPool->queue_not_empty);
}

void *adjustThreadPool(void *arg) {
    pthread_pool_t *_threadPool = arg;
    //管理者的生命周期和整个程序的生命周期相同，因此在死循环中
    while(1) {
	/*
	 * 管理者线程通常情况下处于休眠状态，所以将其放在一个
	 * 死循环中等待通知，只有通知到来时才会执行任务。
	 */
	Pthread_mutex_lock(&_threadPool->self_lock);
	while (1) {
	    Pthread_cond_wait(&_threadPool->do_adjust, &_threadPool->self_lock);
	    break;
	}
	int alv_thr_num = _threadPool->alv_thr_num;                  /* 存活 线程数 */	
	Pthread_mutex_unlock(&_threadPool->self_lock);

	Pthread_mutex_lock(&_threadPool->mutex_bsy);
	int bsy_thr_num = _threadPool->bsy_thr_num;
	Pthread_mutex_unlock(&_threadPool->mutex_bsy);

	if (bsy_thr_num > alv_thr_num * 0.8) {
	    Pthread_mutex_lock(&_threadPool->self_lock);
	    for (int i = 0 ; i < _threadPool->step ; i++) {
		Pthread_create(&_threadPool->pthreads[alv_thr_num + i], NULL, waitThreadTask, (void *)_threadPool);
	    }
	    Pthread_mutex_unlock(&_threadPool->self_lock);
	} else if (bsy_thr_num < alv_thr_num * 0.25) {
	    Pthread_mutex_lock(&_threadPool->self_lock);
	    for (int i = 0; i < _threadPool->step ; i++) {
		//因为在队列中拿到的全是空，所以会自动推出
		Pthread_cond_signal(&_threadPool->queue_not_empty);
	    }
	    Pthread_mutex_unlock(&_threadPool->self_lock);
	}

    }
    return NULL;
}
void *waitThreadTask(void *arg) {
    pthread_pool_t *_threadPool = arg;
    pthread_task_t t;
    /*
     * 这里设计的是每个线程处理完任务之后就返回
     * 线程池中继续等待拿去下一个任务，因此将获
     * 取任务处理任务的过程放在一个死循环中。
     */
    while(1) {
	/*
	 * 这里存在一个问题：在默认开始时创建的
	 * 等待线程在获取锁时都不会出现什么问题，
	 * 因为每一个线程获取锁后立即放弃锁开始
	 * 等待，那么就不会出现有线程拿着锁，而
	 * 新创建的线程在调用此函数时却拿不到锁
	 * 从而失败。
	 * 但是对于已经有线程在工作而此时管理线
	 * 程需要添加额外的等待线程时，那么就可
	 * 能出现，创建的新线程会拿不到锁的情况，
	 * 此时新的线程直接创建失败。
	 */
	Pthread_mutex_lock(&_threadPool->self_lock); 

	//当任务队列中没有任务时则等待队列不为空
	while (_threadPool->cur_que_size == 0) {
	    Pthread_cond_wait(&_threadPool->queue_not_empty, &_threadPool->self_lock);
	}

	t.task = (_threadPool->task_list[_threadPool->front_queue]).task;
	t.arg  = (_threadPool->task_list[_threadPool->front_queue]).arg;

	_threadPool->front_queue = (_threadPool->front_queue + 1) % _threadPool->max_que_size;
	_threadPool->cur_que_size--;

	Pthread_mutex_unlock(&_threadPool->self_lock);

	//拿了东西就告知对方，你可以放了
	Pthread_cond_signal(&_threadPool->queue_not_full);

	if (_threadPool->bsy_thr_num > _threadPool->alv_thr_num * 0.8 ||
		_threadPool->bsy_thr_num < _threadPool->alv_thr_num * 0.25) {
	    Pthread_cond_signal(&_threadPool->do_adjust);
	}
	//在执行任务前后都需要更新忙碌线程数量
	Pthread_mutex_lock(&_threadPool->mutex_bsy);	
	_threadPool->bsy_thr_num++;
	Pthread_mutex_unlock(&_threadPool->mutex_bsy);

	(*t.task)(t.arg);

	Pthread_mutex_lock(&_threadPool->mutex_bsy);
	_threadPool->bsy_thr_num--;
	Pthread_mutex_unlock(&_threadPool->mutex_bsy);
    }
    return NULL;
}

