/*************************************************************************
	> File Name: systutil.h
	> Author:zhaoyihong 
	> Mail:zhaoyihong_at_126_dot_com 
	> Created Time: 2015年03月07日 星期六 15时54分24秒
 ************************************************************************/

#ifndef _SYSTUTIL_H
#define _SYSTUTIL_H


int read_timeout(int fd,unsigned int wait_seconds);
int write_timeout(int fd,unsigned int wait_seconds);
int accept_timeout(int fd,unsigned int wait_seconds);

#endif
