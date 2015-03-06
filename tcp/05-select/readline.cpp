/*************************************************************************
	> File Name: readline.cpp
	> Author: 
	> Mail: 
	> Created Time: 2015年01月03日 星期六 13时50分50秒
 ************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

ssize_t recv_peek(int sockfd,void *buf,size_t len);
int writen(int fd,void *buf,size_t len);
void err_exit(const char *msg);
int readn(int fd,void *buf,size_t len);
ssize_t readline(int sockfd,void *buf,size_t maxlen);

ssize_t readline(int sockfd,void *buf,size_t maxlen)
{
    int nleft = maxlen;
    int nread,nret;
    char *pbuf = (char *)buf;

    while(nleft > 0)
    {
        //由于缓冲区中不见得有maxlen这么多数据，所以可能需要循环读取

        nret = recv_peek(sockfd,pbuf,nleft);
        if(-1 == nret || 0 == nret)
        {
            return nret;
        }

        nread = nret;

        for(int i=0;i < nread; ++i)
        {    
            if('\n' == pbuf[i])
            {
                nret = readn(sockfd,pbuf,i+1);
                if(nret != i+1)
                {
                    exit(EXIT_FAILURE);
                }
                return nret;
            }
        }

        if(nread > nleft)
        {
            exit(EXIT_FAILURE);
        }
 
        nret = readn(sockfd,pbuf,nread);
        if(nret != nread)
        {
            exit(EXIT_FAILURE);
        }

        nleft -= nread;
        pbuf += nread;
    
    }

    //出错
    return -1;
}


int readn(int fd,void *buf,size_t len)
{
    int nleft = len;
    int ret;
    char *pbuf = (char *)buf;
    while(nleft > 0)
    {
        ret = read(fd,buf,nleft);
        if(ret == -1)
        {
            if(errno == EINTR)
            {
                continue;
            }
            
            err_exit("read");
        }
    
        if(ret == 0)
        {
            return ret;
        }

        nleft -= ret;
        pbuf += ret;
    }

    return len - nleft;
}


int writen(int fd,void *buf,size_t len)
{
    int nleft = len;
    int ret;
    char *pbuf = (char *)buf;

    while(nleft > 0)
    {   
        ret = write(fd,pbuf,nleft);
        if(ret == -1)
        {
            if(errno == EINTR)
            {
                continue;
            }
            err_exit("write");
        }
        
        nleft -= ret;
        pbuf += ret;
    }

    return len - nleft;
}

ssize_t recv_peek(int sockfd,void *buf,size_t len)
{
    while(1)
    {
        //使用MSG_PEEK来读取，但读取的内容不从缓冲区中去除
        int ret = recv(sockfd,buf,len,MSG_PEEK);
        
        if(ret == -1 && errno==EINTR)
        {
            continue;
        }   
        return ret;
    }
}


void err_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}






