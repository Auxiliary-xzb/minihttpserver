#include "httptools.h"
#include <sys/epoll.h>

static int hexit(char c);
static const char *getContentType(const char *name);
static void sendFile(int connfd, const char* filename);
static void sendSubdir(int connfd, const char* dirname);
static void sendErrorPage(int connfd, int status, char *title, char *text);
static void writeHttpHeaderLine(int connfd, response_head_t *header);
static ssize_t readHttpHeaderLine(int connfd, char *buf, size_t size);
static void dealHttpRequest(int connfd, void *buf); 

void dealHttp(void *arg) { 
    int connfd = *(int *)arg;

    char line[128] = {'\0'};
    int len = readHttpHeaderLine(connfd, line, sizeof(line));
    if (len == 0) {
	Pthread_mutex_lock(&_loggerLock);
	log_info("User disconnect.");
	Pthread_mutex_unlock(&_loggerLock);
	pthread_exit(NULL);
    } else {
	while (1) {
	    char buf[1024] = {'\0'};
	    len  = readHttpHeaderLine(connfd, buf, sizeof(buf));
	    if (buf[0] == '\n')
		break;
	    else if (len == -1)
		break;
	}
    }	

    if (strncasecmp("get", line, 3) == 0) {
	dealHttpRequest(connfd, line);
	close(connfd);
    }
}

void dealHttpRequest(int connfd, void *buf) {
    // 拆分http请求行
    char method[12], path[1024], protocol[12];
    sscanf(buf, "%[^ ] %[^ ] %[^ ]", method, path, protocol);

    // 对名字进行Unicode解码，中文原因所以需要编码再传
    deUnicode(path, path);

    //请求的文件部分都是以/开始的。
    char *file = path+1; 

    // 如果没有指定访问的资源, 则现实项目的当前根路径
    if(strcmp(path, "/") == 0) {    
	file = "./";
    }

    // 判断文件是否存在
    struct stat st;
    int ret;
    if ( (ret = stat(file, &st)) != 0) {
	// 回发浏览器 404 错误页面	
	sendErrorPage(connfd, 404, "Not Found", "NO such file or direntry");
    }

    response_head_t re;
    bzero(&re, sizeof(re));
    // 判断是目录还是文件
    if(S_ISDIR(st.st_mode)) {  
	re.status_number = 200;
	re.content_length = -1; 
	strncpy(re.status_description, "OK", 3);   
	const char *type = getContentType(".html");
	strncpy(re.content_type, type, strlen(type));

	writeHttpHeaderLine(connfd, &re);
	sendSubdir(connfd, file);
    } else if(S_ISREG(st.st_mode)) { 
	re.status_number = 200;
	re.content_length = st.st_size; 
	strncpy(re.status_description, "OK", 3);   
	const char *type = getContentType(file);
	strncpy(re.content_type, type, strlen(type)); 

	writeHttpHeaderLine(connfd, &re);
	sendFile(connfd, file);
    }
}

ssize_t readHttpHeaderLine(int connfd, char *buf, size_t size) {
    int i = 0;
    char c = '\0';
    int n;

    while ((i < size-1) && (c != '\n')) {
	n = recv(connfd, &c, 1, 0);
	if (n > 0) {
	    if (c == '\r') {
		n = recv(connfd, &c, 1, MSG_PEEK);
		if ((n > 0) && (c == '\n')) 
		    recv(connfd, &c, 1, 0);
		else 
		    c = '\n';
	    }
	    buf[i] = c;
	    i++;
	} else {
	    c = '\n';
	}
    }
    buf[i] = '\0';

    if (-1 == n)
	i = n;

    return i;
}

void writeHttpHeaderLine(int connfd, response_head_t *header) {
    char buf[1024];

    bzero(buf, sizeof(buf));
    sprintf(buf, "HTTP/1.1 %d %s\r\n", header->status_number, header->status_description);

    sprintf(buf+strlen(buf), "Content-Type:%s\r\n", header->content_type);
    sprintf(buf+strlen(buf), "Content-Length:%d\r\n", header->content_length);
    sprintf(buf+strlen(buf), "\r\n");

    int ret = writen(connfd, buf, strlen(buf));
    if (ret == -1) {
	Pthread_mutex_lock(&_loggerLock);
	log_info("writeHttpHeaderLine occur error, and exit");
	Pthread_mutex_unlock(&_loggerLock);
	close(connfd);
	pthread_exit(NULL);
    }
}

const char *getContentType(const char *name) {
    char* dot;

    // 找到文件后缀名，然后比较后缀名
    dot = strrchr(name, '.');   
    if (dot == NULL)
	return "text/plain; charset=utf-8";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
	return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
	return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
	return "image/gif";
    if (strcmp(dot, ".png") == 0)
	return "image/png";
    if (strcmp(dot, ".css") == 0)
	return "text/css";
    if (strcmp(dot, ".au") == 0)
	return "audio/basic";
    if (strcmp( dot, ".wav" ) == 0)
	return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
	return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
	return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
	return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
	return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
	return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
	return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
	return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
	return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}

void sendSubdir(int connfd, const char* dirname) {
    // 构建一个html文件，一边构建一边发送
    char buf[1024];

    bzero(buf, sizeof(buf));
    sprintf(buf, "<html><head><title>目录名: %s</title></head>", dirname);
    sprintf(buf+strlen(buf), "<body><h1>当前目录: %s</h1><table>", dirname);

    char unicodeName[1024] = {'\0'};
    char path[1024] = {'\0'};

    struct dirent **subdir;
    int ret = scandir(dirname, &subdir, NULL, alphasort);

    int i;
    for(i = 0; i < ret; ++i) {
	char *name = subdir[i]->d_name;

	sprintf(path, "%s%s", dirname, name);

	struct stat st;
	stat(path, &st);

	// 对名字进行Unicode编码，中文原因所以需要编码再传
	enUnicode(unicodeName, sizeof(unicodeName), name);

	// 如果是目录则要在后面加一个/表示目录
	if(S_ISREG(st.st_mode)) {       
	    sprintf(buf+strlen(buf), 
		    "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
		    unicodeName, name, (long)st.st_size);
	} else if(S_ISDIR(st.st_mode)) {		      
	    sprintf(buf+strlen(buf), 
		    "<tr><td><a href=\"%s/\">%s/</a></td><td>%ld</td></tr>",
		    unicodeName, name, (long)st.st_size);
	}
	int ret = writen(connfd, buf, strlen(buf));
	if (ret == -1) {
	    Pthread_mutex_lock(&_loggerLock);
	    log_info("sendSubdir occur error.");
	    Pthread_mutex_unlock(&_loggerLock);
	    close(connfd);
	    pthread_exit(NULL);
	}
	bzero(buf, sizeof(buf));
    }
    sprintf(buf+strlen(buf), "</table></body></html>");
    ret = writen(connfd, buf, strlen(buf));
    if (ret == -1) {
	Pthread_mutex_lock(&_loggerLock);
	log_info("the last sendSubdir occur error.");
	Pthread_mutex_unlock(&_loggerLock);
	close(connfd);
	pthread_exit(NULL);
    }
}

void sendFile(int connfd, const char* filename) {
    // 打开文件
    int fd = open(filename, O_RDONLY);
    if(fd == -1) {   
	sendErrorPage(connfd, 404, "Not Found", "NO such file or direntry");
    }

    char buf[4096] = {'\0'};
    int len = 0;
    while( (len = readn(fd, buf, sizeof(buf)) ) > 0 ) {   
	// 发送读出的数据
	if ( writen(connfd, buf, len) == -1) {
	    close(connfd); 
	    pthread_exit(NULL);
	}
    }
    close(fd);
}

void sendErrorPage(int connfd, int status, char *title, char *text) {
    char buf[1204];

    bzero(buf, sizeof(buf));
    sprintf(buf, "%s %d %s\r\n", "HTTP/1.1", status, title);
    sprintf(buf+strlen(buf), "Content-Type:%s\r\n", "text/html");
    sprintf(buf+strlen(buf), "Content-Length:%d\r\n", -1);
    sprintf(buf+strlen(buf), "Connection: close\r\n");
    sprintf(buf+strlen(buf), "\r\n");
    sprintf(buf+strlen(buf), "<html><head><title>%d %s</title></head>\n", status, title);
    sprintf(buf+strlen(buf), "<body bgcolor=\"#cc99cc\"><h2 align=\"center\">%d %s</h4>\n", status, title);
    sprintf(buf+strlen(buf), "%s\n", text);
    sprintf(buf+strlen(buf), "<hr>\n</body>\n</html>\n");

    int ret = writen(connfd, buf, strlen(buf));
    if (ret == -1) {
	close(connfd);
	pthread_exit(NULL);
    }   
    return ;
}

int hexit(char c) {
    if (c >= '0' && c <= '9')
	return c - '0';
    if (c >= 'a' && c <= 'f')
	return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
	return c - 'A' + 10;

    return 0;
}

void enUnicode(char* to, int tosize, const char* from) {
    int tolen;

    for (tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from) {    
	if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0) {      
	    *to = *from;
	    ++to;
	    ++tolen;
	} else {
	    sprintf(to, "%%%02x", (int) *from & 0xff);
	    to += 3;
	    tolen += 3;
	}
    }
    *to = '\0';
}

void deUnicode(char *to, char *from) {
    for ( ; *from != '\0'; ++to, ++from  ) {     
	if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {       
	    *to = hexit(from[1])*16 + hexit(from[2]);
	    from += 2;                      
	} else {
	    *to = *from;
	}
    }
    *to = '\0';
}
