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

ssize_t recv_peek(int sockfd,void *buf,size_t len)
{
    while(1)
    {
        int ret = recv(sockfd,buf,len,MSG_PEEK);
        
        if(ret == -1 && errno==EINTR)
        {
            continue;
        }

        return ret;
    }
}








