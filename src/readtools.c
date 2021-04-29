#include "readtools.h"

ssize_t readn(int fd, void *buf, size_t n) {
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = buf;
    nleft = n;
    while (nleft > 0) {
	if ( (nread = read(fd, ptr, nleft)) == -1) {
	    int errno_saved = errno;
	    if (errno_saved == EINTR || errno_saved == EAGAIN || errno_saved == EWOULDBLOCK) {
		//如果调用被中断则此次调用的返回值作废
		//继续的读取剩余的数据
		nread = 0;
	    } else {
		Pthread_mutex_lock(&_loggerLock);
		log_error("read occur error. errno is %d", errno_saved);
		Pthread_mutex_unlock(&_loggerLock);
		return -1;
	    }
	} else if (nread == 0) {
	    break;
	}

	nleft -= nread;
	ptr   += nread;
    }
    return (n - nleft);
}

