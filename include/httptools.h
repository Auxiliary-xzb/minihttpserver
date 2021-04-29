#ifndef _HTTPTOOLS_H
#define _HTTPTOOLS_H
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "log.h"
#include "eventctl.h"
#include "readtools.h"
#include "writetools.h"
#include "wrappthread.h"

typedef struct response_head_t {
	int status_number;
	int content_length;
	char status_description[128];
	char content_encoding[128];
	char content_type[128];
	char date[128];
}response_head_t;

void dealHttp(void *arg);

void deUnicode(char *to, char *from);
void enUnicode(char* to, int tosize, const char* from);
#endif
