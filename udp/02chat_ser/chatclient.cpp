/*************************************************************************
	> File Name: chatserv.cpp
	> Author:zhaoyihong 
	> Mail:zhaoyihong_at_126_dot_com 
	> Created Time: 2015年01月11日 星期日 16时52分59秒
 ************************************************************************/

#include<iostream>
#include  <sys/socket.h>
#include  <sys/types.h>
#include  <unistd.h>
#include  <stdlib.h>
#include  <arpa/inet.h>
#include    "pub.h"
#include  <sys/epoll>
#include <string.h>


using namespace std;

USER_LIST client_list;
char username[16];


void err_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void do_someone_login();
void do_someone_logout();
void do_getlist();
void do_chat();
void parse_cmd(char *,int ,sockaddr_in *);
void sendmsgto(int sock,char *peername,char *msg_line);


int main(void)
{
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

    chat_client(sock,&pserver_addr);

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
        fflush(stdout);
        scanf("%s",username);

        strcpy(msg.body,username);
        msg.cmd = htonl(C2S_LOGIN);
        ret = sendto(sock,&msg,sizeof(msg),0,(struct sockaddr *)pserver_addr,sizeof(*pserver_addr));
        if(ret == -1)
        {
            if(erron == EINTR)
            {
                continue;
            }
            err_exit("sendto");
        }

        memset(&msg,0,sizeof(msg));
        ret = recvfrom(sock,&msg,sizeof(msg),0,NULL,NULL);
        if(ret == -1)
        {
             if(erron == EINTR)
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
                break;
            case S2C_ALREADY_LOGINED:
                //已经登录了,换个名字
                continue;
                break;
            default:
                break;
        }
    }

    //接收在线人数信息
    int count;
    ret = recvfrom(sock,&count,sizeof(count),NULL,NULL);
    printf("has %d users logined server!\n",ntohl(count));
    
    for(int i =0 ;i < count ; ++i)
    {
        ret = recvfrom(sock,&user_info,sizeof(user_info),0,NULL,NULL);
        if(ret == -1)
        {
            if(errno == EINTR)
            {
                coutinue;
            }
            err_exit("recvfrom");
        }
        
        printf("%d,%s <--> %s:%d"\n,\
                i,user_info.username,inet_ntoa(user_info.ip),ntos(user_info.port));
        client_list.push_back(user_info);
    }

   printf("commands are:\n");
   printf("send <username> <msg>\n");
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
   ev.events = EPOLLN | EPOLLET;
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
                        do_someone_login();
                         break;
                   case S2C_SOMEONE_LOGOUT :
                       do_someone_logout();
                       break;   
                   case S2C_ONLIE_USER :
                       do_getlist();
                       break;   
                   case C2C_CHAT :
                       do_chat();
                       break;   
                   default :
                       break;
               }
           }

           //如果是由设备输入的
           if(events[i].data.fd == STDIN_FILENO)
           {
                char cmd_line[100] = {0};
                if(fgets(cmd_line,sizeof(cmd),stdin) == NULL)
                {
                    //终止输入 退出客户端
                    break;
                }
            
               if(cmd_line[0] == '\n')
               {
                    continue;
               }

               cmd[strlen(cmd_line)-1] = '\0';
               parse_cmd(cmd_line,sock,pserver_addr);
           }
       }
   }
    
   
   //登出
   

}


void do_someone_login();

void do_someone_logout();

void do_getlist();

void do_chat();

void parse_cmd(char *cmd_line,int sock,sockaddr_in *pserver_addr)
{
    char cmd[10] = {0};
    char *p = strchr(cmdline,' ');
    if(p != NULL)
    {
        *p = '\0';
    }
    
    strcpy(cmd,cmdline);
    if(strcmp(cmd,"exit") == 0)
    {
        //登出
        MESSAGE msg;
        msg.cmd = htonl(C2S_LOGOUT);
        
        sendto(sock,&msg,sizeof(msg),0,(sockaddr *)pserver_addr,sizeof(*pserver_addr));
        exit(EXIT_FAILURE);
        printf("user %s has logout server!\n",username);

    }
    else if(strcmp(cmd,"send") == 0)
    {
        char peername[16] = {0};
        char msg_line[MSG_LEN] = {0};

        while(*p++ == ' '){}
        char *p2 = strchr(p,' ');
        
        if(p2 == NULL)
        {
            printf("bad command\n");
            printf("commands are:\n");
            printf("send <username> <msg>\n");
            printf("list\n");
            printf("exit\n");
            printf("\n");
            return;
        }

        *p2 = '\0'
        strcpy(peername,p);
        
        while(*p2++ == ' ') {}
        strcpy(msg_line,p2);
        sendmsgto(sock,peername,msg_line);
    }
    else if(strcmp(cmd,"list") == 0)
    {
        MESSAGE msg;
        memset(&msg,0,sizeof(msg));
        msg.cmd = htonl(CS2_ONLINE_USER);
        if(sendto(sock,&msg,sizeof(msg),0,(sockaddr *)pserver_addr,sizeof(*pserver_addr)) < 0 )
        {
            err_exit("sendto");
        }
    }
    else
    {
            printf("bad command\n");
            printf("commands are:\n");
            printf("send <username> <msg>\n");
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
    msg.cmd = C2C_CHAT;
    CHAT_MSG chat_msg;
    memset(&chat_msg,0,sizeof(chat_msg));
    strcpy(chat_msg.username,username);
    strcpy(chat_msg.msg,msg_line);

    memcpy(msg.body,&chat_msg,sizeof(chat_msg));
    
    struct in_addr tmp;
    tmp.s_addr = it->ip;
    printf("sending message %s to user %s\n",msg_line,inet_ntoa(tmp));

    sendto(sock,&msg,szieof(msg),0,(struct sockaddr *)&peer_addr,sizeof(peer_addr));
}
