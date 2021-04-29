#ifndef _SOCKCONNECT_H
#define _SOCKCONNECT_H

#include "wrapsock.h"
#include "eventctl.h"
#include "httptools.h"

int getListener(int port);
void acceptConnection(void *arg);
#endif
