/*************************************************************************
	> File Name: echo_ser.cpp
	> Author:zhaoyihong 
	> Mail:zhaoyihong_at_126_dot_com 
	> Created Time: 2015年01月14日 星期三 16时08分43秒
 ************************************************************************/

#include<iostream>
#include  <cstdio>
#include  <sys/socket.h>
#include  <cerrno>
#include  <cstdlib>
#include  <unistd.h>
#include  <sys/types.h>
#include  <sys/un.h>
#include  <sys/signal.h>
#include    "sysutils.h"

using namespace std;

void echo_client(int conn);

int main(void)
{
    signal(SIGPIPE,SIG_IGN);

    int sockfd;
    if((sockfd=socket(AF_UNIX,SOCK_STREAM,0)) < 0 )
    {
        err_exit("socket");
    }

    struct sockaddr_un server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path,"/tmp/testsocket");
    
    if(connect(sockfd,(sockaddr *)&server_addr,sizeof(server_addr)) < 0)
    {
        err_exit("connect");
    }

    echo_client(sockfd);

    return 0;
}


void echo_client(int conn)
{
    int fd = open("text",O_CREAT|O_WRONLY|O_APPEND,0644);
    if(-1 == fd)
    {
        err_exit("open");
    }

    send_fd(conn,fd);
    close(fd);

    puts("done");
}
