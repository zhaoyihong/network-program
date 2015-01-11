/*************************************************************************
	> File Name: echo_ser.cpp
	> Author:zhaoyihong 
	> Mail:zhaoyihong_at_126_dot_com 
	> Created Time: 2015年01月11日 星期日 14时17分59秒
 ************************************************************************/

#include<iostream>
#include<cstdio>
#include <errno.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include <unistd.h>
#include  <stdlib.h>
#include <string.h>
 #include <arpa/inet.h>

using namespace std;


void echo_ser(int fd);

void err_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main()
{
    int sock;
    sock = socket(AF_INET,SOCK_DGRAM,0); //IPV4 数据包协议只有UDP,第三个参数用0
    if(sock < 0)
    {
        err_exit("socket"); 
    }

    int on = 1;
    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0 )
    {
        err_exit("socket");
    }

    struct sockaddr_in ser_addr;
    memset(&ser_addr,0,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(5188);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock,(struct sockaddr *)&ser_addr,sizeof(ser_addr)) < 0)
    {
        err_exit("bind");
    }

    //udp 服务器不需要监听
    
    echo_ser(sock);

    close(sock);
    return 0;
}

void echo_ser(int fd)
{
    char recv_buf[1024] = {0};
    struct sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    int n ;
    while(1)
    {
        memset(&recv_buf,0,sizeof(recv_buf));
        n = recvfrom(fd,recv_buf,sizeof(recv_buf),0,(sockaddr *)&peer_addr,&peer_len);
        if(n == -1)
        {
            if(errno == EINTR)
            {
                continue;
            }

            err_exit("recvfrom");
        }
        else
        {
            cout << "recv:" << recv_buf ;
            sendto(fd,recv_buf,strlen(recv_buf),0,(sockaddr *)&peer_addr,peer_len);
        }
    
    }

}


