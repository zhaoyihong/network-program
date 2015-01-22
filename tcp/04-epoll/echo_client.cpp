/*************************************************************************
	> File Name: echo_clien.cpp
	> Author: 
	> Mail: 
	> Created Time: 2014年12月28日 星期日 14时08分59秒
 ************************************************************************/
#include<sys/types.h>      
#include<iostream>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include <string.h>
#include <netinet/in.h>
#include  <arpa/inet.h>
#include  <signal.h>
#include "readline.h"
#include  <vector>
#include  <sys/epoll.h>

using namespace std;

void client_service(int);

int main()
{
    signal(SIGPIPE,SIG_IGN);
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0 )  err_exit("socket"); 
    
    struct sockaddr_in ser_addr;
    memset(&ser_addr,0,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ser_addr.sin_port = htons(5188);
    
    int ret = connect(sock,(struct sockaddr*)&ser_addr,sizeof(ser_addr));
    
    if(ret == -1)
    {
        err_exit("conn");
    }
    
    client_service(sock);

    close(sock);

    return 0;
    
}


void client_service(int conn)
{
    int epfd = epoll_create1(EPOLL_CLOEXEC); 
    if(epfd < 0)
    {
        err_exit("epoll_create1");
    }

    struct epoll_event ev;
    int stdinfd = fileno(stdin);
    memset(&ev,0,sizeof(ev));
    ev.data.fd = stdinfd;
    ev.events = EPOLLIN || EPOLLET;
    
    int ret;
    ret = epoll_ctl(epfd,EPOLL_CTL_ADD,stdinfd,&ev);
    if(ret < 0)
    {
        err_exit("epoll_ctl");
    }

    ev.data.fd = conn;
    ret = epoll_ctl(epfd,EPOLL_CTL_ADD,conn,&ev);
    if(ret < 0)
    {
        err_exit("epoll_ctl");
    }

    vector<epoll_event> events(16);
    bool stdin_close = false;
    while(1)
    {
        ret = epoll_wait(epfd,&(*events.begin()),events.size(),-1);
        if(ret == -1)
        {
            err_exit("epoll_wait");
        }
        else if(ret == 0)
        {
            continue;
        }
        else if(ret == (int)events.size())
        {
            events.resize(2*ret);
        }

        for(int i=0;i<ret;++i)
        {
            if(events[i].events & EPOLLIN)
            {
                if(events[i].data.fd == stdinfd)
                {
                    char buf[1024] = {0};
                    
                    //标准输入终止
                    if(NULL == fgets(buf,(int)(sizeof(buf)),stdin))
                    {   
                        stdin_close = true;  
                        shutdown(conn,SHUT_WR);
                        ret = epoll_ctl(epfd,EPOLL_CTL_DEL,stdinfd,NULL);
                        if(ret == -1)
                        {
                            err_exit("epoll_ctl");
                        }
                        continue;    
                    }
                    else
                    {
                        ret = writen(conn,buf,strlen(buf));
                        if(ret != (int)strlen(buf))
                        {
                            err_exit("writen");
                        }
                    }

                }
                else if(events[i].data.fd == conn)
                {
                    char recvbuf[1024] = {0};
                    ret = readline(conn,recvbuf,sizeof(recvbuf));
                    if(ret == 0)
                    {
                        if(stdin_close)
                        {
                            //客户端正常终止 (关闭stdin然后接收完数据)
                            printf("client close\n");
                            return;
                        }
                        else
                        {
                            //服务器提前终止(服务器主动断开连接)
                            err_exit("server terminated prematurely");
                        }

                    }
                    else 
                    {
                        printf("recv:%s",recvbuf);
                    }
                }
            }
        }
    }


}

/*
void client_service(int conn)
{
    char sendbuf[1024] = {0};
    char recvbuf[1024] ={0};
    int ret;

    while(fgets(sendbuf,sizeof(sendbuf),stdin) != NULL)
    {
        writen(conn,sendbuf,strlen(sendbuf)); 

        ret = readline(conn,recvbuf,1024);
        if(0 == ret)
        {
            cout << "服务器关闭" << endl;
            break;
        }

        printf("receiv:%s",recvbuf);
        memset(&sendbuf,0,sizeof(sendbuf));
        memset(&recvbuf,0,sizeof(recvbuf));
    }

}
*/
