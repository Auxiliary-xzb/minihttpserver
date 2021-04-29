#include "wrapsys.h"

void Close(int fd) {
    int ret;

    if ( (ret = close(fd)) == -1 ) {
	int errno_saved = errno;
	Pthread_mutex_lock(&_loggerLock);
	log_error("Can't close the file. errno is %d", errno_saved);
	Pthread_mutex_unlock(&_loggerLock);
    }
}

