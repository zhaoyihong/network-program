/*************************************************************************
	> File Name: socketpair.cpp
	> Author:zhaoyihong 
	> Mail:zhaoyihong_at_126_dot_com 
	> Created Time: 2015年01月14日 星期三 19时49分47秒
    socketpair 可以用来创建一个全双工管道,用来进行进程间通信
************************************************************************/

#include  <iostream>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <errno.h>
#include  <stdlib.h>
#include  <unistd.h>
#include <stdio.h>

using namespace std;

void err_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void)
{
    int sockfds[2];
    if(socketpair(AF_UNIX,SOCK_STREAM,0,sockfds) < 0)
    {
        err_exit("socketpair");
    }

    pid_t pid;
    pid = fork();
    if(pid == -1)
    {
        err_exit("fork");
    }
    else if(pid == 0)
    {
        close(sockfds[0]);
        int var;
        while(1)
        {
            read(sockfds[1],&var,sizeof(var));
            ++var;
            write(sockfds[1],&var,sizeof(var));
        }
        
    }
    else if(pid>0)
    {
        close(sockfds[1]);
        int var = 1;
        while(1)
        {
            write(sockfds[0],&var,sizeof(var));
            printf("sending data %d\n",var);
            read(sockfds[0],&var,sizeof(var));
            printf("receving data %d\n",var);
            ++var;
            sleep(1);
        }
    }

}





