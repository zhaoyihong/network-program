/*************************************************************************
	> File Name: readpack.h
	> Author: 
	> Mail: 
	> Created Time: 2014年12月29日 星期一 14时52分28秒
 ************************************************************************/

#ifndef _PACKIO_H
#define _PACKIO_H

typedef struct Pack
{
    int len;
    char buf[1024];
}Pack_t;

void err_exit(const char *msg);
int readn(int fd,void *buf,int len);
int writen(int fd,void *buf,int len); 
int writePack(int fd,Pack_t *buf);
int readPack(int fd,Pack_t *buf);
int recv_peek(int fd,void *buf,int len);
int readline(int fd,void *buf,int maxlen);


#endif
