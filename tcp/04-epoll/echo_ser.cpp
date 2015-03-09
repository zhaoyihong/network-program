/*************************************************************************
	> File Name: echo_ser.cpp
	> Author: 
	> Mail: 
	> Created Time: 2014年12月28日 星期日 13时18分27秒
 ************************************************************************/
#include<sys/types.h>      
#include<iostream>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include  <signal.h>
#include "readline.h"
#include <sys/epoll.h>
#include <vector>
#include <algorithm>
#include <errno.h>
#include  <fcntl.h>

using namespace std;
void server_service(int);

void chld_handle(int sig)
{
    while(waitpid(-1,NULL,WNOHANG)>0)
    {
        puts("子进程退出");
    }
}

typedef vector<struct epoll_event> EventList;

int main()
{
    signal(SIGCHLD,chld_handle);
    signal(SIGPIPE,SIG_IGN);
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd < 0 )  err_exit("socket"); 
    struct sockaddr_in ser_addr;
    memset(&ser_addr,0,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(5188);
 
    int on = 1;
    if(setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0 )
        err_exit("setsockopt");
   
    if(bind(listenfd,(sockaddr *)&ser_addr,sizeof(ser_addr)) == -1 )
       err_exit("bind"); 

    struct sockaddr_in local_addr;
    socklen_t local_len = sizeof(local_addr);
    if(getsockname(listenfd,(struct sockaddr *)&local_addr,&local_len) < 0)
    {
        err_exit("getsockname");
    }
    cout << "local addr:" << inet_ntoa(local_addr.sin_addr);
    cout << " port:" << ntohs(local_addr.sin_port) << endl;


    if(listen(listenfd,100) == -1)
    {
        err_exit("listen");
    }
 
    //设置为非阻塞
    active_nonblock(listenfd);

    struct epoll_event ev; 
    int epfd = epoll_create(EPOLL_CLOEXEC);
    ev.data.fd = listenfd;
    ev.events = EPOLLIN | EPOLLET; //有数据到来，边缘方式触发
    //将监听套接口以及感兴趣的时间加入到epoll来管理
    if(epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev) < 0 )
    {
        err_exit("epoll_ctl");
    }

    EventList events(16);
    vector<int> clients;
    int count = 0;
    int nready;
    while(1)
    {
        nready = epoll_wait(epfd,&*events.begin(),static_cast<int>(events.size()),-1);
        if(-1 == nready)
        {
            if(EINTR == errno)
            {
                continue;
            }
            err_exit("epoll_wait");
        }

        //超时
        if(0 == nready)
        {
            continue;
        }

        if((int)events.size() == nready)
        {
            events.resize(2*nready);
        }

        for(int i=0;i<nready;++i)
        {
            if(events[i].data.fd == listenfd)
            {
                struct sockaddr_in peer_addr;
                socklen_t peer_len = sizeof(peer_addr);
                int conn = accept(listenfd,(sockaddr *)&peer_addr,&peer_len);
                if(-1 == conn )
                {
                    //排除信号中断,非阻塞模式的EAGAIN,连接中断,协议错误等原因. 特别是要注意EAGAIN
                    if(errno != EINTR && errno!=EAGAIN && errno!=ECONNABORTED && errno!=EPROTO)
                    {
                        err_exit("accept");
                    }

                    continue;
                }
                printf("peer addr%s,port:%d\n",inet_ntoa(peer_addr.sin_addr),ntohs(peer_addr.sin_port));
                //保存已连接的套接字
                clients.push_back(conn);
                
                //将套接字设置为非阻塞
                active_nonblock(conn);

                ev.data.fd = conn;
                ev.events = EPOLLIN | EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_ADD,conn,&ev); //添加到监听时间中

                count ++;
            }
            else if(events[i].events & EPOLLIN)
            {
                int conn = events[i].data.fd;
                if(conn < 0)
                {
                    continue;
                }
                
                char recvbuf[1024] = {0};
                int ret;
                ret = readline(conn,recvbuf,1024);
                
                
                if(-1 == ret)
                {
                    err_exit("readline");
                }
                else if(0 == ret)
                {
                    cout << "clients closed" << endl;
                    ev = events[i];
                    epoll_ctl(epfd, EPOLL_CTL_DEL,conn,&ev);
                    clients.erase(remove(clients.begin(),clients.end(),conn),clients.end());
                    count--;
                    close(conn);
                }
                else
                {
                    cout << "recv:" << recvbuf;
                    ret = writen(conn,recvbuf,strlen(recvbuf));
                    if(-1 == ret)
                    {
                        err_exit("writen");
                    }
                }

            }
        }
    }
    close(listenfd);
    return 0;
}

