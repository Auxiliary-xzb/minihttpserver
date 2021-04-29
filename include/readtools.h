#ifndef _READTOOLS_H
#define _READTOOLS_H
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>

#include "log.h"
#include "wrappthread.h"

ssize_t readn(int fd, void *buf, size_t n);

#endif
