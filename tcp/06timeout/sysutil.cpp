/*************************************************************************
	> File Name: sysutil.cpp
	> Author:zhaoyihong 
	> Mail:zhaoyihong_at_126_dot_com 
	> Created Time: 2015年03月07日 星期六 15时40分52秒
 ************************************************************************/

#include  <sys/select.h>
#include  <stdio.h>
#include  <sys/time.h>
#include  <sys/types.h>
#include  <errno.h>
#include  <stdlib.h> 
#include  <sys/socket.h>
#include  <netinet/in.h>
#include  <fcntl.h>
#include  "readline.h"

/*
void err_exit(const char *msg)
{
    perror(msg);
    exit(-1);
}
*/

void active_nonblock(int fd)
{
    int flag = fcntl(fd,F_GETFL,NULL);
    if(-1 == flag)
    {
        err_exit("fcntl");
    }

    if(-1 == fcntl(fd,F_SETFL,flag|O_NONBLOCK))
    {
        err_exit("fcntl");
    }
}

void deactive_nonblock(int fd)
{
    int flag = fcntl(fd,F_GETFL,NULL);
    if(-1 == flag)
    {
        err_exit("fcntl");
    }

    if(-1 == fcntl(fd,F_SETFL,flag&(~O_NONBLOCK)))
    {
        err_exit("fcntl");
    }
}


/**
 *read_timeout - 读超时检测函数,不包含读操作
 *@fd:文件描述符
 *@wait_seconds:等待时间描述,如果<0表示不检测
 *成功返回0,失败返回-1,超时返回-1且errno=ETIMEDOUT
 */
int read_timeout(int fd,unsigned int wait_seconds)
{
    int ret = 0;
    if(wait_seconds > 0)
    {
        fd_set read_fdset;
        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        FD_ZERO(&read_fdset);
        FD_SET(fd,&read_fdset);

        do
        {
            ret = select(fd+1,&read_fdset,NULL,NULL,&timeout);
        }while(ret < 0 && errno == EINTR);

        if(ret == 0) //超时返回-1,errno=ETIMEDOUT
        {
            errno = ETIMEDOUT;
            ret = -1;
        }
        else if ( ret > 0) //成功返回0
        {
            ret = 0;
        }
    }

    return ret ;
}

/**
 *write_timeout - 写超时检测函数,不包含写操作
 *@fd:文件描述符
 *@wait_seconds:等待时间描述,如果<0表示不检测
 *成功返回0,失败返回-1,超时返回-1且errno=ETIMEDOUT
 */
int write_timeout(int fd,unsigned int wait_seconds)
{
    int ret = 0;
    if(wait_seconds > 0)
    {
        fd_set write_fdset;
        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        FD_ZERO(&write_fdset);
        FD_SET(fd,&write_fdset);

        do
        {
            ret = select(fd+1,NULL,&write_fdset,NULL,&timeout);
        }while(ret < 0 && errno == EINTR);

        if(ret == 0) //超时返回-1,errno=ETIMEDOUT
        {
            errno = ETIMEDOUT;
            ret = -1;
        }
        else if ( ret > 0) //成功返回0
        {
            ret = 0;
        }
    }

    return ret ;
}



/**
 *accept_timeout - accept超时检测函数,包含accept操作
 *@fd:文件描述符
 *@addr:accept返回的对方地址
 *@wait_seconds:等待时间描述,如果<0表示不检测
 *成功返回已连接的套接字,失败返回-1,超时返回-1且errno=ETIMEOUT
 */
int accept_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds)
{
    int ret = 0;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    
    if(wait_seconds > 0)
    {
        fd_set read_fdset;
        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        FD_ZERO(&read_fdset);
        FD_SET(fd,&read_fdset);

        do
        {
            ret = select(fd+1,&read_fdset,NULL,NULL,&timeout);
        }while(ret < 0 && errno == EINTR);

        if(ret == -1)
        {
            return -1;
        }
        else if(ret == 0) //超时返回-1,errno=ETIMEDOUT
        {
            errno = ETIMEDOUT;
            return -1;
        }
    }

    if(addr != NULL)
    {
        ret = accept(fd,(sockaddr *)addr,&addrlen);
        
    }
    else
    {
        ret = accept(fd,NULL,NULL);
    }

    if(ret < 0)
    {
        err_exit("accept");
    }
    
    return ret ;
}



/**
 *connect_timeout - connect超时检测函数,包含connect操作
 *@fd:文件描述符
 *@addr:accept返回的对方地址
 *@wait_seconds:等待时间描述,如果<0表示使用系统默认超时时间75秒
 *成功返回已连接的套接字,失败返回-1,超时返回-1且errno=ETIMEDOUT
 */
int connect_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds)
{
    int ret = 0;
    socklen_t addrlen = sizeof(struct sockaddr_in);
   
    //设置为非阻塞模式
    if(wait_seconds > 0)
    {
        active_nonblock(fd);    
    }

    //connect在非阻塞模式下会立即返回,在阻塞模式下会的等待默认1.5倍RTT75s
    ret = connect(fd,(sockaddr *)addr,addrlen);
    
    if(-1 == ret && errno == EINPROGRESS) //非阻塞模式时 可能无法返回成功
    {

        //使用select来检测fd是否可写
        fd_set connect_fdset;
        FD_ZERO(&connect_fdset);
        FD_SET(fd,&connect_fdset);
        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do
        {
            //一旦建立连接,套接字就可写
            ret = select(fd+1,NULL,&connect_fdset,NULL,&timeout);
        }while(ret < 0 && errno == EINTR);

        //超时
        if(ret == 0)
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if(ret == 1)
        {
            //ret = 1 有两种情况:1 连接建立成功 2套接字产生错误
            //套接字错误不会保存在errno中,需要使用getsockopt来获取
            int err;
            socklen_t socklen = sizeof(err);
            int sockopterr = getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,&socklen);
            if(-1 == sockopterr)
            {
                return -1;
            }

            if(0 == err) //没有错误
            {
                ret = 0; //连接建立成功
            }
            else
            {
                errno = err;
                ret = -1;
            }
        }
    }// end if ret == -1

    //文件描述符重新置为阻塞模式
    if(wait_seconds > 0)
    {
        deactive_nonblock(fd);
    }

    return ret;
}



