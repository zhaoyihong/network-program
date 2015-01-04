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
#include "packio.h"
#include <signal.h>
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
    Pack_t sendbuf;
    Pack_t recvbuf;

    while(fgets(sendbuf.buf,sizeof(sendbuf.buf),stdin) != NULL)
    {
        sendbuf.len = htonl(strlen(sendbuf.buf));
        writePack(conn,&sendbuf);
        
        int ret = readPack(conn,&recvbuf);
        if(0 == ret)
        {
            cout << "服务器关闭" <<endl;
            break;
        }

        fputs(recvbuf.buf,stdout);

        memset(&sendbuf,0,sizeof(sendbuf));
        memset(&recvbuf,0,sizeof(recvbuf));
    }
}
