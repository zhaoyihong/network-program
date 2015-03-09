/*************************************************************************
	> File Name: sysutil.h
	> Author:zhaoyihong 
	> Mail:zhaoyihong_at_126_dot_com 
	> Created Time: 2015年03月07日 星期六 19时48分13秒
 ************************************************************************/

#ifndef _SYSUTIL_H
#define _SYSUTIL_H
    
//void err_exit(const char *msg) ;
void active_nonblock(int fd) ;
void deactive_nonblock(int);
int read_timeout(int fd,unsigned int wait_seconds);
int write_timeout(int fd,unsigned int wait_seconds);
int accept_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds);
int connect_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds);


#endif
