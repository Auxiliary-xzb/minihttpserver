#ifndef _WRAPSOCK_H
#define _WRAPSOCK_H
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "log.h"
#include "wrappthread.h"

void Listen(int fd, int backlog);
int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr);
void Bind(int fd, const struct sockaddr *addr, socklen_t addrlen);

#endif
