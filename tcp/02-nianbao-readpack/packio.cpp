/*************************************************************************
	> File Name: readpack.cpp
	> Author: 
	> Mail: 
	> Created Time: 2014年12月29日 星期一 14时52分19秒
 ************************************************************************/

#include<iostream>
#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include"packio.h"
#include <arpa/inet.h>

using namespace std;

void err_exit(const char *msg)
{
    perror(msg);
    exit(-1);
}

int readn(int fd,void *buf,int len)
{
//    cout << "readn:"  << len << endl;
    int left = len;
    char *buf_start = (char *)buf;
    while(left > 0)
    {
        int ret = read(fd,buf_start,left);
        if(ret == -1)
        {
            //信号中断
            if(errno == EINTR)
            {
                continue;
            }

            //读取错误
            err_exit("read");
        }
        else if(ret == 0)
        {
            //写端关闭
            break;
        }
        left -= ret;
        buf_start += ret;
    }

    return len-left;
}

int writen(int fd,void *buf,int len)
{
    int left = len;
    char *buf_start = (char *)buf;
    while(left > 0)
    {
        int ret = write(fd,buf_start,left);
        if(ret == -1)
        {
            if(EINTR == errno)
            {
                continue;
            }
            err_exit("write");
        }

        left -= ret;
        buf_start += ret;  
    }

    return len-left;
}

int writePack(int fd,Pack_t *buf)
{
    int len = ntohl(buf->len);
    int total = writen(fd,buf,sizeof(len)+len);
   // cout << "writepack:" << len << "," << buf->buf << endl; 
    return total;
}


int readPack(int fd,Pack_t *buf)
{
    int ret=0,total=0;
    total = readn(fd,&buf->len,sizeof(buf->len));
    int len = ntohl(buf->len);
    ret = readn(fd,buf->buf,len);
   // cout << "readpack:" << len << "," << buf->buf << endl; 
    return total + ret;
}



