#include "wrapsock.h"

extern pthread_mutex_t _loggerLock;
static pthread_mutex_t _acceptLock = PTHREAD_MUTEX_INITIALIZER;

void Listen(int fd, int backlog) {
    if (listen(fd, backlog) < 0) {
	int errno_saved = errno;
	Pthread_mutex_lock(&_loggerLock);
	log_error("Can't get fd listened, errno is %d", errno_saved);
	Pthread_mutex_unlock(&_loggerLock);
    }
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr) {
    Pthread_mutex_lock(&_acceptLock);
    int connfd;

    if ( (connfd = accept(fd, sa, salenptr)) == -1 ) {
	int errno_saved = errno;
	switch (errno_saved)
	{
	    case EAGAIN:
	    case ECONNABORTED:
	    case EINTR:
	    case EPROTO: // ???
	    case EPERM:
	    case EMFILE: // per-process lmit of open file desctiptor ???
		// expected errors
		errno = errno_saved;
		break;
	    case EBADF:
	    case EFAULT:
	    case EINVAL:
	    case ENFILE:
	    case ENOBUFS:
	    case ENOMEM:
	    case ENOTSOCK:
	    case EOPNOTSUPP:
		Pthread_mutex_lock(&_loggerLock);
		log_fatal("unexpected error of accept %d", errno_saved);
		Pthread_mutex_unlock(&_loggerLock);
		break;
	    default:
		Pthread_mutex_lock(&_loggerLock);
		log_fatal("unexpected error of accept %d", errno_saved);
		Pthread_mutex_unlock(&_loggerLock);
		break;
	}
    }
    Pthread_mutex_unlock(&_acceptLock);

    return connfd;
}

void Bind(int fd, const struct sockaddr *addr, socklen_t addrlen) {
    if (bind(fd, addr, addrlen) != 0) {
	int errno_saved = errno;
	Pthread_mutex_lock(&_loggerLock);
	log_error("Can't bind the addr. errno is %d", errno_saved);
	Pthread_mutex_unlock(&_loggerLock);
    }
}


