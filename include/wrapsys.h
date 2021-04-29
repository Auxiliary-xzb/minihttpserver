#ifndef _WRAPSYS_H
#define _WRAPSYS_H
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "wrappthread.h"
#include "log.h"

void Close(int fd);

#endif
