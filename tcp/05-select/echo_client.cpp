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
#include  <sys/select.h>



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
    fd_set rset,rsetall;
    FD_ZERO(&rsetall);
    int stdin_fd = fileno(stdin);
    FD_SET(stdin_fd,&rsetall);
    FD_SET(conn,&rsetall);
    bool is_stdin_closed = false;
    int maxfd = max(stdin_fd,conn);

    while(1)
    {
        rset = rsetall;
        int nready = select(maxfd+1,&rset,NULL,NULL,NULL);
        if(-1 == nready)
        {
            err_exit("select");
        }
        if(0 == nready)
        {
            continue;
        }
        
        if(FD_ISSET(stdin_fd,&rset))
        {
            char recv_buf[1024] = {
                0
            };

            if(NULL == fgets(recv_buf,1024,stdin))
            {
                is_stdin_closed = true;
                FD_CLR(stdin_fd,&rsetall);
                shutdown(conn,SHUT_WR);
                maxfd = conn;
            }
            else
            {
                if(-1 == writen(conn,recv_buf,strlen(recv_buf)))
                {
                    err_exit("writen");
                }
            }
        }

        if(FD_ISSET(conn,&rset))
        {
            char recv_buf[1024] = {
                0
            };
            int nread = readline(conn,recv_buf,1024);
            if(-1 == nread)
            {
                err_exit("readline");
            }

            if(0 == nread)
            {
                if(is_stdin_closed)
                {
                    puts("client closed");
                    break;
                }
                else
                {
                    puts("server closed");
                    break;
                }
            }
            else
            {
                printf("recv:%s",recv_buf);
            }
        }

    }

    close(conn);
}

