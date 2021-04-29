#ifndef _WRITETOOLS_H
#define _WRITETOOLS_H
#include <unistd.h>
#include <stdio.h>

#include "log.h"
#include "wrappthread.h"

ssize_t writen(int fd, const void *buf, size_t n);

#endif
