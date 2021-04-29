#ifndef _UTILITIES_H
#define _UTILITIES_H

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

#include "include/log.h"
#include "include/eventctl.h"
#include "include/wrapsys.h"
#include "include/wrapsock.h"
#include "include/wrappthread.h"
#include "include/pthreadpool.h"
#include "include/readtools.h"
#include "include/httptools.h"
#include "include/sockconnect.h"

extern pthread_mutex_t _loggerLock;
extern event_tree_t *eventTree;
extern pthread_pool_t *thread_pool;

#endif
