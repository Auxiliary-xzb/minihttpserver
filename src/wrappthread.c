#include "wrappthread.h"

pthread_mutex_t _loggerLock = PTHREAD_MUTEX_INITIALIZER;

int Pthread_create(pthread_t *threadID, const pthread_attr_t *attr, void *(*pthreadTask)(void *), void *arg) {
    int ret;

    if ( (ret = pthread_create(threadID, attr, pthreadTask, arg)) != 0 ) {
	int errno_saved = errno;
	log_error("Can't create the thread. errno is %d", errno_saved);
    } else {
	return 0;
    }
    return -1;
}

void Pthread_exit(void *arg) {
    pthread_exit(arg);
}

void Pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) {
    int ret;

    if ( (ret = pthread_mutex_init(mutex, attr)) != 0 ) {
	int errno_saved = errno;
	log_error("Can't init the thread. errno is %d", errno_saved);
    }
}
void Pthread_mutex_lock(pthread_mutex_t *lock) {
    int ret;

    if ( (ret = pthread_mutex_lock(lock)) != 0 ) {
	int errno_saved = errno;
	log_error("Can't get the mutex lock. errno is %d", errno_saved);
    }
}

void Pthread_mutex_unlock(pthread_mutex_t *lock) {
    int ret;

    if ( (ret = pthread_mutex_unlock(lock)) != 0 ) {
	int errno_saved = errno;
	log_error("Can't release the mutex lock. errno is %d", errno_saved);
    }
}
void Pthread_mutex_destroy(pthread_mutex_t *lock) {
    int ret;

    if ( (ret = pthread_mutex_destroy(lock)) != 0 ) {
	int errno_saved = errno;
	log_error("Can't destroy the mutex lock. errno is %d", errno_saved);
    }
}

void Pthread_cond_init(pthread_cond_t *mutex,const pthread_condattr_t *attr) {
    int ret;

    if ( (ret = pthread_cond_init(mutex, attr)) != 0 ) {
	int errno_saved = errno;
	log_error("Can't init the condition lock. errno is %d\n", errno_saved);
    }
}

void Pthread_cond_wait(pthread_cond_t *condLock, pthread_mutex_t *mutexLock) {
    int ret;

    if ( (ret = pthread_cond_wait(condLock, mutexLock)) != 0 ) {
	int errno_saved = errno;
	log_error("Can't get the condition lock. errno is %d", errno_saved);
    }
}

void Pthread_cond_destroy(pthread_cond_t *lock) {
    int ret;

    if ( (ret = pthread_cond_destroy(lock)) != 0 ) {
	int errno_saved = errno;
	log_error("Can't destroy the condition lock. errno is %d", errno_saved);
    }
}

void Pthread_cond_signal(pthread_cond_t *lock) {
    int ret;

    if ( (ret = pthread_cond_signal(lock)) != 0 ) {
	int errno_saved = errno;
	log_error("Error occur in signal thread. errno is %d", errno_saved);
    }
}
void Pthread_cond_broadcast(pthread_cond_t *lock) {
    int ret;

    if ( (ret = pthread_cond_broadcast(lock)) != 0 ) {
	int errno_saved = errno;
	log_error("Error occur in broadcast thread. errno is %d", errno_saved);
    }  
}


