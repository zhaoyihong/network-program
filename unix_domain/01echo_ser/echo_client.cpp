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
using namespace std;

void err_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

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
    char recv_buf[1024] = {
        0
    };
    char send_buf[1024] ={
        0
    };
    int ret;
    while(fgets(send_buf,sizeof(send_buf),stdin) != NULL)
    {
        ret = write(conn,send_buf,strlen(send_buf));
        if(-1 == ret)
        {
            err_exit("write");
        }


        ret = read(conn,recv_buf,sizeof(recv_buf));
        if(ret == -1)
        {
            if(EINTR == errno)
            {
                continue;
            }

            err_exit("read");
        }
        else if(0 == ret)
        {
            cout << "serer closed" << endl;
            close(conn);
            return;
        }

        puts(recv_buf);

        memset(recv_buf,0,sizeof(recv_buf));
        memset(send_buf,0,sizeof(send_buf));
    }

}
