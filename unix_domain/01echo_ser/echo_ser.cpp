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
    int ret;
    while(1)
    {
        memset(recv_buf,0,sizeof(recv_buf));
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
            cout << "client closed" << endl;
            close(conn);
            return;
        }
        else 
        {
            puts(recv_buf);
            ret = write(conn,recv_buf,strlen(recv_buf));
            if(-1 == ret)
            {
                err_exit("write");
            }
        }

    }

}
