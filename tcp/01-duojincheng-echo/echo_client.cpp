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
using namespace std;

void err_exit(const char *);
void client_service(int);

int main()
{
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0 )  err_exit("socket"); 
    
    struct sockaddr_in ser_addr;
    memset(&ser_addr,sizeof(ser_addr),0);
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

void err_exit(const char *msg)
{
    perror(msg);
    exit(-1);
}

void client_service(int conn)
{
    char sendbuf[1024] = {0};
    char recvbuf[1024] = {0};

    while(fgets(sendbuf,sizeof(sendbuf),stdin) != NULL)
    {
        int len = write(conn,sendbuf,strlen(sendbuf));
        len = read(conn,recvbuf,sizeof(recvbuf)); 
        if(len == 0)
        {
            cerr << "server closed" << endl;
            break;
        }
        else if(len == -1)
        {
            err_exit("read");
        }

        fputs(recvbuf,stdout);
        memset(sendbuf,0,sizeof(sendbuf));
        memset(recvbuf,0,sizeof(recvbuf));
    }
}
