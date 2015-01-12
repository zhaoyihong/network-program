/*************************************************************************
	> File Name: chatserv.cpp
	> Author:zhaoyihong 
	> Mail:zhaoyihong_at_126_dot_com 
	> Created Time: 2015年01月11日 星期日 16时52分59秒
 ************************************************************************/

#include <stdio.h>
#include<iostream>
#include  <sys/socket.h>
#include  <sys/types.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <arpa/inet.h>
#include    "pub.h"
#include  <sys/epoll.h>
#include <string.h>
#include  <errno.h>
#include  <vector>
#include  <signal.h>
using namespace std;

USER_LIST client_list;
char username[16];


void err_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void chat_client(int sock,sockaddr_in* pserver_addr);
void do_someone_login(MESSAGE &);
void do_someone_logout(MESSAGE &);
void do_getlist(int);
void do_chat(MESSAGE &msg);   
void do_public_chat(MESSAGE &msg);
void print_usage();

void parse_cmd(char *,int ,sockaddr_in *);
bool sendmsgto(int sock,char *peername,char *msg_line);

int main(void)
{
    signal(SIGINT,SIG_IGN);
    int sock;
    if((sock=socket(AF_INET,SOCK_DGRAM,0)) < 0 )
    {
       err_exit("socket"); 
    }

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(5188);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

   // if(connect(sock,(sockaddr *)&server_addr,sizeof(server_addr)) < 0)
   // {
   //     err_exit("connect");
   // }

    chat_client(sock,&server_addr);

    close(sock);
    return 0;
}

void chat_client(int sock,sockaddr_in* pserver_addr)
{
    struct sockaddr_in client_addr;
    socklen_t client_len ;
    int ret;
    MESSAGE msg;
    USER_INFO user_info;
    while(1)
    {
        memset(&msg,0,sizeof(msg));
        memset(username,0,sizeof(username));

        printf("please input your name:");        
        fflush(stdout); //输出上面的语句
        scanf("%s",username);
       // fflush(stdin); //处理scanf后面的回车符
        getchar();
        strcpy(msg.body,username);
        msg.cmd = htonl(C2S_LOGIN);
        ret = sendto(sock,&msg,sizeof(msg),0,(struct sockaddr *)pserver_addr,sizeof(*pserver_addr));
        if(ret == -1)
        {
            if(errno == EINTR)
            {
                continue;
            }
            err_exit("sendto");
        }

        memset(&msg,0,sizeof(msg));
        ret = recvfrom(sock,&msg,sizeof(msg),0,NULL,NULL);
        if(ret == -1)
        {
             if(errno == EINTR)
            {
                continue;
            }
            err_exit("recvfrom");
        }
    

        int cmd = ntohl(msg.cmd);
        switch(cmd)
        {
            case S2C_LOGIN_OK:
                //登录成功 break循环 
                printf("login success!\n");
                break;
            case S2C_ALREADY_LOGINED:
                //已经登录了,换个名字
                continue;
                break;
            default:
                break;
        }
        break;
    }

    //接收在线人数信息
    int count;
    ret = recvfrom(sock,&count,sizeof(count),0,NULL,NULL);
    printf("has %d users logined server!\n",ntohl(count));
   
    int ncount = ntohl(count);
    for(int i =0 ;i < ncount ; ++i)
    {
        ret = recvfrom(sock,&user_info,sizeof(user_info),0,NULL,NULL);
        if(ret == -1)
        {
            if(errno == EINTR)
            {
                continue;
            }
            err_exit("recvfrom");
        }
       
        in_addr tmp;
        tmp.s_addr = user_info.ip;
        printf("%d,%s<-->%s:%d\n",\
               i,user_info.username,inet_ntoa(tmp),ntohs(user_info.port));
        client_list.push_back(user_info);
    }

   printf("commands are:\n");
   printf("send <username> <msg>\n");
   printf("sendall <msg>\n");
   printf("list\n");
   printf("exit\n");
   printf("\n");


   //IO复用

   epoll_event ev;
   int epfd = epoll_create1(EPOLL_CLOEXEC);
   if(epfd == -1)
   {
       err_exit("epoll_create1");
   }

   ev.data.fd = STDIN_FILENO;
   ev.events = EPOLLIN | EPOLLET;
   if(epoll_ctl(epfd,EPOLL_CTL_ADD,ev.data.fd,&ev) < 0)
   {
       err_exit("epoll_ctl");
   }

   ev.data.fd = sock;
   if(epoll_ctl(epfd,EPOLL_CTL_ADD,ev.data.fd,&ev) < 0)
   {
       err_exit("epoll_ctl");
   }

   vector<epoll_event> events(16);
   while(1)
   {
       int nready = epoll_wait(epfd,&*events.begin(),(int)events.size(),-1);
       if(nready == -1)
       {
           if(errno == EINTR)
           {
               continue;
           }
           err_exit("epoll_wait");
       }
      
       if(nready == events.size())
       {
           events.resize(2*nready);
       }

       for(int i = 0; i< nready; ++i)
       {

           if(events[i].events & EPOLLIN)
           {
               //如果是由网络接收的
               if(events[i].data.fd == sock)
               {
                   sockaddr_in peer_addr;
                   socklen_t peer_len = sizeof(peer_addr);
                   recvfrom(sock,&msg,sizeof(msg),0,(sockaddr *)&peer_addr,&peer_len);
                   int cmd = ntohl(msg.cmd);
                   switch ( cmd )
                   {
                       case S2C_SOMEONE_LOGIN :
                            do_someone_login(msg);
                             break;
                       case S2C_SOMEONE_LOGOUT :
                           do_someone_logout(msg);
                           break;   
                       case S2C_ONLIE_USER :
                           do_getlist(sock);
                           break;   
                       case C2C_CHAT :
                           do_chat(msg);
                           break;
                       case S2C_PUBLIC_CHAT:
                            do_public_chat(msg);
                       default :
                           break;
                   }
               }

               //如果是由设备输入的
               if(events[i].data.fd == STDIN_FILENO)
               {
                    char cmd_line[100] = {0};
                    if(fgets(cmd_line,sizeof(cmd_line),stdin) == NULL)
                    {
                        //终止输入 退出客户端
                        break;
                    } 
                  
                   if(cmd_line[0] == '\n')
                   {
                        continue;
                   }

                   cmd_line[strlen(cmd_line)-1] = '\0';
                   
                   parse_cmd(cmd_line,sock,pserver_addr);
               }
           }
       }
   }
    
  
    //登出
    memset(&msg,0,sizeof(msg)); 
    msg.cmd = htonl(C2S_LOGOUT);
    strcpy(msg.body,username);  
    sendto(sock,&msg,sizeof(msg),0,(sockaddr *)pserver_addr,sizeof(*pserver_addr));
    printf("user %s has logout server!\n",username);
}


void do_someone_login(MESSAGE &msg)
{
    USER_INFO *user_info = (USER_INFO *)msg.body; 
    in_addr tmp;
    tmp.s_addr = user_info->ip;
    printf("%s<-->%s:%d has logined server!\n",user_info->username,inet_ntoa(tmp),ntohs(user_info->port));
    client_list.push_back(*user_info);
}

void do_someone_logout(MESSAGE &msg)
{
    USER_LIST::iterator it;
    for(it = client_list.begin();it != client_list.end(); ++it)
    {
        if(strcmp(it->username , msg.body) == 0)
        {
            break;
        }
    }

    if(it != client_list.end())
    {
        client_list.erase(it);
    }

    printf("user %s has logined out!\n",msg.body);

}

void do_getlist(int sock)
{
    int count;
    recvfrom(sock,&count,sizeof(count),0,NULL,NULL);
    printf("has %d users logined server!\n",ntohl(count));

    client_list.clear();

    int n = ntohl(count);
    for(int i = 0; i < n ;++i)
    {
        USER_INFO user_info;
        recvfrom(sock,&user_info,sizeof(user_info),0,NULL,NULL);
        client_list.push_back(user_info);
        in_addr tmp;
        tmp.s_addr =  user_info.ip;
        printf("%s<-->%s:%d\n",user_info.username,inet_ntoa(tmp),ntohs(user_info.port));
    }

}

void do_chat(MESSAGE &msg)
{
    CHAT_MSG *p_chat = (CHAT_MSG *)msg.body;
    printf("user %s sending to you:%s\n",p_chat->username,p_chat->msg);
}


void do_public_chat(MESSAGE &msg)
{
    CHAT_MSG *p_chat = (CHAT_MSG *)msg.body;
    printf("user %s sending to all:%s\n",p_chat->username,p_chat->msg);
}

void parse_cmd(char *cmd_line,int sock,sockaddr_in *pserver_addr)
{
    char cmd[10] = {0};
    char *p = strchr(cmd_line,' ');
    bool single_comand = true;
    if(p != NULL)
    {
        *p = '\0';
        single_comand = false;
    }
   
    strcpy(cmd,cmd_line);
    if(strcmp(cmd,"exit") == 0)
    {
        //登出
        MESSAGE msg;
        memset(&msg,0,sizeof(msg));
        msg.cmd = htonl(C2S_LOGOUT);
        strcpy(msg.body,username);
        sendto(sock,&msg,sizeof(msg),0,(sockaddr *)pserver_addr,sizeof(*pserver_addr));
        printf("user %s has logout server!\n",username);
        exit(EXIT_FAILURE);

    }
    else if(strcmp(cmd,"send") == 0)
    {
        if(single_comand)
        {
            printf("bad command\n");
            print_usage();
            return;    
        }

        char peername[16] = {0};
        char msg_line[MSG_LEN] = {0};

        while(*++p == ' '){} //去除空格
        char *p2 = strchr(p,' ');
        
        if(p2 == NULL)
        {
            printf("bad command\n");
            print_usage();
            return;
        }

        *p2 = '\0';
        strcpy(peername,p);
        
        while(*++p2 == ' '){}
        if(*p2 == '\0')
        {
            printf("cannot send null message\n");
            print_usage();
            return;
        }
        strcpy(msg_line,p2);
        sendmsgto(sock,peername,msg_line);
    }
    else if(strcmp(cmd,"list") == 0)
    {
        MESSAGE msg;
        memset(&msg,0,sizeof(msg));
        msg.cmd = htonl(C2S_ONLINE_USER);
        if(sendto(sock,&msg,sizeof(msg),0,(sockaddr *)pserver_addr,sizeof(*pserver_addr)) < 0 )
        {
            err_exit("sendto");
        }
    }
    else if(strcmp(cmd,"sendall") == 0)
    {
        if(single_comand)
        {
            printf("bad command\n");
            print_usage();
            return;    
        }

        while(*++p == ' '){}
        if(*p == '\0')
        {
            printf("cannot send null message\n");
            print_usage();
            return;
        }

        MESSAGE msg;
        memset(&msg,0,sizeof(msg));
        msg.cmd = htonl(C2S_PUBLIC_CHAT);
        
        CHAT_MSG chat_msg;
        memset(&chat_msg,0,sizeof(chat_msg));
        strcpy(chat_msg.username,username);
        strcpy(chat_msg.msg,p);
        memcpy(msg.body,&chat_msg,sizeof(chat_msg));
        if(sendto(sock,&msg,sizeof(msg),0,(sockaddr *)pserver_addr,sizeof(*pserver_addr)) < 0 )
        {
            err_exit("sendto");
        }

        printf("you sending to all %s\n:",chat_msg.msg);
    }
    else
    {
            printf("bad command\n");
            printf("commands are:\n");
            printf("send <username> <msg>\n");
            printf("sendall <msg>\n");
            printf("list\n");
            printf("exit\n");
            printf("\n");
    }
    
}


bool sendmsgto(int sock,char *peername,char *msg_line)
{
    if(strcmp(peername,username)==0)
    {
        printf("cannot sendto myself!\n");
        return false;
    }

    USER_LIST::iterator it ;
    for(it = client_list.begin(); it != client_list.end(); ++it)
    {
        if(strcmp(it->username,peername)==0)
        {
            break;
        }
    }

    if(it == client_list.end())
    {
        printf("user %s has not logined server !\n",peername);
        return false;
    }

    sockaddr_in peer_addr ;
    memset(&peer_addr,0,sizeof(peer_addr));
    peer_addr.sin_family  = AF_INET;
    peer_addr.sin_port = it->port;
    peer_addr.sin_addr.s_addr = it->ip;

    MESSAGE msg;
    msg.cmd = htonl(C2C_CHAT);
    CHAT_MSG chat_msg;
    memset(&chat_msg,0,sizeof(chat_msg));
    strcpy(chat_msg.username,username);
    strcpy(chat_msg.msg,msg_line);

    memcpy(msg.body,&chat_msg,sizeof(chat_msg));
    
    printf("sending message %s to user %s\n",msg_line,peername);

    sendto(sock,&msg,sizeof(msg),0,(struct sockaddr *)&peer_addr,sizeof(peer_addr));
}


void print_usage()
{
    printf("commands are:\n");
    printf("send <username> <msg>\n");
    printf("list\n");
    printf("exit\n");
    printf("\n");
     
}
