#ifndef _SOCKCONNECT_H
#define _SOCKCONNECT_H

#include "wrapsock.h"
#include "eventctl.h"
#include "httptools.h"

//获取监听套接字
int getListener(int port);

//链接到来时的处理函数
void acceptConnection(void *arg);
#endif
