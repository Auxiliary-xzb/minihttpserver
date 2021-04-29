#ifndef _WRAPPTHREAD_H
#define _WRAPPTHREAD_H
#include <pthread.h>
#include <errno.h>

#include "log.h"

extern pthread_mutex_t _loggerLock;

int Pthread_create(pthread_t *threadID, const pthread_attr_t *attr, void *(*pthreadTask)(void *), void *arg);

void Pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
void Pthread_mutex_lock(pthread_mutex_t *lock);
void Pthread_mutex_unlock(pthread_mutex_t *lock);
void Pthread_mutex_destroy(pthread_mutex_t *lock);

void Pthread_cond_init(pthread_cond_t *mutex, const pthread_condattr_t *attr);
void Pthread_cond_wait(pthread_cond_t *condLock, pthread_mutex_t *mutexLock);
void Pthread_cond_destroy(pthread_cond_t *lock);
void Pthread_cond_signal(pthread_cond_t *lock);
void Pthread_cond_broadcast(pthread_cond_t *lock);

#endif

