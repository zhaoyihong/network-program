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
#include    "sysutil.h"
#include  <errno.h>


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
   
    
    //含有超时的连接,等待1秒
    int ret = connect_timeout(sock,&ser_addr,1);
    //int ret = connect(sock,(struct sockaddr*)&ser_addr,sizeof(ser_addr));
    
    //连接超时
    if(ret == -1 && errno == ETIMEDOUT)
    {
        err_exit("connect_timeout");
    }
    
    //连接失败
    if(ret == -1)
    {
        err_exit("connect failed");
    }

    client_service(sock);

    close(sock);

    return 0;
    
}


void client_service(int conn)
{
    char sendbuf[1024] = {0};
    char recvbuf[1024] ={0};
    int ret;

    while(fgets(sendbuf,sizeof(sendbuf),stdin) != NULL)
    {
        ret = write_timeout(conn,1);
        if(-1 == ret)
        {
            if(errno == ETIMEDOUT)
            {
                err_exit("write time out");
            }
        
            err_exit("write_timeout function failed");
        }

        writen(conn,sendbuf,strlen(sendbuf)); 

        ret = read_timeout(conn,1);
        if(-1 == ret)
        {
            if(errno == ETIMEDOUT)
            {
                err_exit("read time out");
            }
        
            err_exit("read_timeout function failed");
        }

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
