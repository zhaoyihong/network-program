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
//#include <sys/epoll.h>
//#include <vector>
#include <algorithm>
#include <errno.h>
#include  <fcntl.h>
#include <sys/select.h>

using namespace std;
void server_service(int);

void chld_handle(int sig)
{
    while(waitpid(-1,NULL,WNOHANG)>0)
    {
        puts("子进程退出");
    }
}


const int MAX_CLIENTS=100;

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


    //SOMMAXCONN 可连接的队列大小,队列满后拒绝新的请求
    if(listen(listenfd,SOMAXCONN) == -1)
    {
        err_exit("listen");
    }
  
    int clients[MAX_CLIENTS];
    for(int i=0;i<MAX_CLIENTS;++i)
    {
        clients[i] = -1;
    }

    //需要2个fd_set来管理
    fd_set rset; //给select的参数,select函数返回后只有可读的是SET状态.
    fd_set rset_all;//管理所有监听的套接口
    FD_ZERO(&rset_all);
    FD_SET(listenfd,&rset_all);
    rset = rset_all;

    int nmax = listenfd;
    int client_cnt = 0;

    while(1)
    {
        //puts("select");
        //第一个参数是最大的文件描述符+1
        int nready = select(nmax+1,&rset,NULL,NULL,NULL);
        //printf("nready:%d\n",nready);
        if(nready == -1)
        {
            if(errno == EINTR)
            {
                continue;
            }

            err_exit("select");
        }

        if(nready  == 0)
        {
            continue;
        }
        
        if(FD_ISSET(listenfd,&rset))
        {
            if(client_cnt < MAX_CLIENTS)
            {
                int conn = accept(listenfd,NULL,NULL);
                if(-1 == conn)
                {
                    err_exit("accept");
                }

                for(int i=0;i<MAX_CLIENTS;++i)
                {
                    if(clients[i] == -1)
                    {
                        clients[i] = conn;
                        if(nmax < conn)
                        {
                            nmax = conn;
                        }
                        break;
                    }
                }
                
                FD_SET(conn,&rset_all);
                ++ client_cnt;
                puts("client connected");
            }
            else
            {
                printf("cannot accept more clients!\n");
                exit(-1);
            }

        }

        for(int i=0;i<MAX_CLIENTS;++i)
        {
            //检测客户端是否可读
            if(clients[i]!= -1 && FD_ISSET(clients[i],&rset)) //FD_ISSET(-1,&rset)会出错
            {
                char recv_buf[1024] = {
                    0
                };
                int nread = readline(clients[i],recv_buf,1024);
                if(-1 == nread)
                {
                    err_exit("readline");
                }

                if(0 == nread)
                {   
                    //客户端关闭连接
                    printf("client closed the connect!\n");
                    int nclient = clients[i];
                    clients[i] = -1;
                    FD_CLR(nclient,&rset_all);
                    shutdown(nclient,SHUT_RDWR);
            //        printf("clr %d\n",nclient);
             //       printf("lis %d\n",listenfd);
                    if(nclient >= nmax)
                    {
                        nmax = listenfd;
                        for(int i=0;i<MAX_CLIENTS;++i)
                        {
                            if(clients[i] > nmax)
                            {
                                nmax = clients[i];
                            }
                        }
                    }
          //          printf("nmax %d\n",nmax);
                    --client_cnt;
                }
                else
                {
                    printf("%s",recv_buf);
                    if(-1 == writen(clients[i],recv_buf,strlen(recv_buf)))
                    {
                        err_exit("writen");
                    }

                }//if(nread == 0) end
            }//if isset end
        }//for end
    
        rset = rset_all; //将rset重置为所有监听可读的套接口
    
    }//while

    close(listenfd);
    return 0;
}

