/*************************************************************************
	> File Name: readline.h
	> Author:zhaoyihong 
	> Mail:zhaoyihong_at_126_dot_com 
	> Created Time: 2015年01月03日 星期六 19时41分12秒
 ************************************************************************/

#ifndef _READLINE_H
#define _READLINE_H
ssize_t recv_peek(int sockfd,void *buf,size_t len);
void err_exit(const char *msg);
int readn(int fd,void *buf,size_t len);
ssize_t readline(int sockfd,void *buf,size_t maxlen);
int writen(int fd,void *buf,size_t len);
#endif
