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

void echo_ser(int);


int main(void)
{
    signal(SIGCHLD,SIG_IGN);
    signal(SIGPIPE,SIG_IGN);

    const char *path = "/tmp/testsocket"; 
    unlink(path);

    int listen_fd;
    if((listen_fd=socket(AF_UNIX,SOCK_STREAM,0)) < 0 )
    {
        err_exit("socket");
    }

    struct sockaddr_un server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path,path);

    if(bind(listen_fd,(sockaddr *)&server_addr,sizeof(server_addr)) < 0)
    {
        err_exit("bind");
    }

    if(listen(listen_fd,SOMAXCONN) < 0)
    {
        err_exit("listen");
    }

    int conn;
    pid_t pid;
    while(1)
    {
        conn = accept(listen_fd,NULL,NULL);
        if(conn < 0)
        {
            if(EINTR == errno)
            {
                continue;
            }
            err_exit("accept");
        }

        pid = fork();

        if(pid == -1)
        {
            err_exit("fork");
        }

        if(pid == 0)
        {
            close(listen_fd);
            echo_ser(conn);
            exit(EXIT_SUCCESS);
        }
        
        close(conn);
    }


    return 0;
}


void echo_ser(int conn)
{
    char recv_buf[1024];
    memset(recv_buf,0,sizeof(recv_buf));
    int fd = recv_fd(conn);
    char msg[] = "hello client!\n";
   
    int ret;
    while( (ret=write(fd,(void *)msg,(ssize_t)strlen(msg))) == -1  && errno == EINTR)
    {
    }

    if(-1 == ret)
    {
        err_exit("write");
    }
    

    close(fd);
}
