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
using namespace std;

USER_LIST client_list;

void err_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void do_login(MESSAGE msg,int sock,sockaddr_in *client_addr);

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

    if(bind(sock,(sockaddr *)&server_addr,sizeof(server_addr))< 0)
    {
        err_exit("bind");
    }


    chat_ser(sock);

    close(sock);
    return 0;
}

void chat_ser(int sock)
{
    struct sockaddr_in client_addr;
    socklen_t client_len ;
    int ret;
    MESSAGE msg;

    while(1)
    {
        memset(&msg,0,sizeof(msg));
        client_len = sizeof(client_addr);    
        
        ret = recvfrom(sock,&msg,sizeof(msg),0,(struct sockaddr *)&client_addr,&client_len);
        if(ret < 0)
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
            case C2S_LOGIN:
                do_login(msg,sock,&client_addr);
                break;
            case C2S_LOGOUT:
                do_logout(msg,sock,&clent_addr);
                break;
            case C2S_ONLINE_USER:
                do_send_list(sock,&client_addr);
                break;
            default:
                break;
        }
    }
}


void do_login(MESSAGE msg,int sock,sockaddr_in *client_addr)
{
    USER_INFO user_info;
    memset(user_info.username,0,sizeof(user_info.username));
    strcpy(user_info.username,msg.body);
    user_info.ip = client_addr.sin_addr.s_addr;
    user_info.port = client_addr.sin_port;

    USER_LIST::iterator it;
    for(it = client_list.begin(); it != client_list.end(); ++it)
    {
        if(strcmp(it->username,msg.body) == 0)
        {
            break;
        }
    }


    //没在已登录列表中找到用户
    if(it == client_list.end())
    {
        printf("has a user login:%s <--> %s:%d\n",msg.body,inet_ntoa(user_info.ip),ntohs(user_info.port));
        client_list.push_back(user_info);
        
        //发送登录成功应答
        MESSAGE reply_msg;
        reply_msg.cmd = htonl(S2C_LOGIN_OK);
        sendto(sock,&reply_msg,sizeif(reply_msg),0,(sockaddr *)&client_addr,sizeof(client_addr));

        //发送在线人数信息
        //1 发送人数
        int count = htonl(client_list.size());
        sendto(sock,&count,sizeif(count),0,(sockaddr *)&client_addr,sizeof(client_addr));
        
        //2 发送在线用户信息
        for(it = client_list.begin(); it != client_list.end(); ++it)
        {
            sendto(sock,&(*it),sizeof(USER_INFO),0,(sockaddr *)&client_addr,sizeof(client_addr));
        }

        //3 向其他用户通知有用户登录
        msg.cmd = htonl(S2C_SOMEONE_LOGIN);
        for(it = client_list.begin(); it != client_list.end(); ++it)
        {
            if(strcmp(it->username,user_info.username) == 0)
            {
                continue;
            }
            sockaddr_in peer_addr;
            memset(peer_addr,0,sizeof(peer_addr));
            peer_addr.sin_family = AF_INET;
            peer_addr.sin_port = it->port;
            peer_addr.sin_addr.s_addr = it->ip;

            if(sendto(sock,&msg,sizeof(msg),0,(sockaddr *)&peer_addr,sizeof(peer_addr))<0)
            {
                err_exit("sendto");
            }
        }
    }
    else //在已登录列表中找到了用户
    {
        MESSAGE reply_msg;
        reply_msg.cmd = htonl(S2C_ALREADY_LOGINED);
        sendto(sock,&reply_msg,sizeif(reply_msg),0,(sockaddr *)&client_addr,sizeof(client_addr));
    }
}
