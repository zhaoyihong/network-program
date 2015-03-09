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

    struct sockaddr_in ser_addr;
    memset(&ser_addr,0,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(5188);
    ser_addr.sin_addr.s_addr = inet_addr("120.24.169.130");
    socklen_t ser_len = sizeof(ser_addr);

    //只有connect之后才能返回异步错误
    //UDP connect没有三次握手,仅仅只是维护了一个信息
    //connect 只能发送给对等方
    if(connect(sock,(struct sockaddr *)&ser_addr,sizeof(ser_addr)) < 0)
    {
        err_exit("connect");
    }

    char send_buf[1024] = {0};
    char recv_buf[1024] = {0};

    int n;
    while(fgets(send_buf,sizeof(send_buf),stdin))
    {
        
        //n = sendto(sock,send_buf,strlen(send_buf),0,(sockaddr *)&ser_addr,ser_len);
        n = sendto(sock,send_buf,strlen(send_buf),0,NULL,0);
        
        if(-1 == n)
        {
            err_exit("sendto");
        }

        //n = recvfrom(sock,recv_buf,sizeof(recv_buf),0,(sockaddr *)&ser_addr,&ser_len);
        n = recvfrom(sock,recv_buf,sizeof(recv_buf),0,NULL,NULL);
       

        if(-1 == n)
        {
            if(errno == EINTR)
            {
                continue;
            }
            err_exit("recvfrom");
        }
        else if(0 == n)
        {
            cout << "server closed" << endl;
            break;
        }
        else
        {
            cout << recv_buf;
        }

        memset(send_buf,0,sizeof(send_buf));
        memset(recv_buf,0,sizeof(recv_buf));
    }

    close(sock);
    return 0;
}

