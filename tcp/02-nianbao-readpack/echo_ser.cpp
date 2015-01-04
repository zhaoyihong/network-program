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
#include"packio.h"
#include <signal.h>
#include <sys/wait.h>

using namespace std;
//void err_exit(const char *);
void server_service(int);

void chld_hanle(int sig)
{
    while(waitpid(-1,NULL,WNOHANG) > 0)
    {
        puts("子进程退出");
    }
}

int main()
{
    signal(SIGCHLD,chld_hanle);
    signal(SIGPIPE,SIG_IGN);
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0 )  err_exit("socket"); 
    struct sockaddr_in ser_addr;
    memset(&ser_addr,0,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   ser_addr.sin_port = htons(5188);
 
    int on = 1;
    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0 )
        err_exit("setsockopt");
   
    if(bind(sock,(sockaddr *)&ser_addr,sizeof(ser_addr)) == -1 )
       err_exit("bind"); 


    if(listen(sock,100) == -1)
    {
        err_exit("listen");
    }
   
    int conn;

    while(1)
    {
        struct sockaddr_in client_addr;
        int len = sizeof(client_addr);
        conn = accept(sock,(sockaddr *)&client_addr,(socklen_t*)&len);
        if(conn == -1)
        {
            err_exit("conn");
        }
    
        int fd = fork();
        if(fd==-1)
        {
            err_exit("fork");
        }

        //父进程
        if(fd  > 0)
        {
            close(conn);
            continue;
        }
        else //子进程
        {
            close(sock);
            server_service(conn);
            close(conn);
            break;
        }
    }
    close(sock);
    return 0;
}
/*
void err_exit(const char *msg)
{
    perror(msg);
    exit(-1);
}
*/
void server_service(int conn)
{
    Pack_t recvbuf;    
    memset(&recvbuf,0,sizeof(recvbuf));
    while(1)
    {
        int ret = readPack(conn,&recvbuf); 
        if(0 == ret)
        {
            cout << "客户端关闭" << endl;
            break;
        }
        //puts(recvbuf.buf);
        printf("receive:%s",recvbuf.buf);
        writePack(conn,&recvbuf);
        memset(&recvbuf,0,sizeof(recvbuf));
    }
}
