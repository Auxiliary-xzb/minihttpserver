#include "writetools.h"

ssize_t writen(int fd, const void *buf, size_t n) {
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = buf;
    nleft = n;
    while (nleft > 0) {
	if ( (nwritten = write(fd, ptr, nleft)) <= 0 ) {
	    if (nwritten < 0) {
		int errno_saved = errno;
		if ( errno == EWOULDBLOCK||errno_saved == EAGAIN || errno_saved == EINTR) {
		    nwritten = 0;
		}
	    } else {
		int errno_saved = errno;
		Pthread_mutex_lock(&_loggerLock);
		log_error("writen occur error. errno is %d", errno_saved);
		Pthread_mutex_unlock(&_loggerLock);
		return -1;
	    }
	}
	nleft -= nwritten;
	ptr   += nwritten;
    }
    return n;
}



