
#ifndef _SYSUTILS_H
#define _SYSUTILS_H
#include    "common.h"

/*系统工具模块*/

//错误退出
void err_exit(const char *msg);

//获取本地ip
int getlocalip(char *ip);

//获取本地ip和端口
int getsockaddr(int,int *,char *);


//描述符阻塞与解除阻塞
void active_nonblock(int fd);
void deactive_nonblock(int fd);

//超时读写
int read_timeout(int fd,unsigned int wait_seconds);
int write_timeout(int fd,unsigned int wait_seconds);
int accept_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds);
int connect_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds);


//tcp防粘包读写
ssize_t readn(int fd,void *buf,size_t count);
ssize_t writen(int fd,void *buf,size_t count);

ssize_t recv_peek(int fd,void *buf,size_t len);
ssize_t readline(int fd,void *buf,size_t maxline);

//pack读写
typedef struct pack
{
    int len;
    char buf[1024];
}Pack_t;


int writePack(int fd,Pack_t *buf);
int readPack(int fd,Pack_t *buf);


//发送与接收描述符
void send_fd(int sockfd,int fd);
int recv_fd(int sockfd);

//设置端口reuse
void reuse_addr(int fd);

//设置tcp服务器监听
int tcp_server(const char *host,unsigned int port);

//连接tcp服务器
int tcp_client(const char *host,unsigned int port);

//处理CHILD信号
void __handle_sigchld(int sig);
void process_sigchld(bool isIgnore);

#endif
