#include    "sysutils.h"

//错误退出
void err_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}


/**                                                                                                 *  tcp_server - 启动tcp服务器
 *  @host:服务器ip地址或者主机名
 *  @port:服务器端口号
 *  成功返回监听套接字
 */
int tcp_server(const char *host,unsigned int port)
{
    int listenfd = socket(PF_INET,SOCK_STREAM,0);
    if(listenfd == -1)
    {
        err_exit("socket");
    }

    struct sockaddr_in seraddr;
    memset(&seraddr,0,sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = ntohs(port);

    //根据输入的host不同,绑定的ip地址也不同. 如果host为NULL,则绑定所有IP地址
    if(host != NULL) //host不为NULL,绑定指定的host
    {
        //host可能是ip地址,也可能是主机名. 当两者都不是时,会在gethostbyname中报错

        struct in_addr tmp_addr;
        if( inet_aton(host,&tmp_addr) != 0)  //host是ip地址
        {
            seraddr.sin_addr = tmp_addr;
        }
        else //host是主机名
        {
            hostent *hp = gethostbyname(host);
            if(NULL == hp)
            {
                err_exit("gethostbyname");
            }

            seraddr.sin_addr = *(struct in_addr*)hp->h_addr_list;
        }
    }
    else
    {
        seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    //端口重用
    reuse_addr(listenfd);

    if(bind(listenfd,(sockaddr *)&seraddr,(socklen_t)sizeof(seraddr)) < 0)
    {
        err_exit("bind");
    }

    if(listen(listenfd,SOMAXCONN) < 0)
    {
        err_exit("listen");
    }

    return listenfd;     
} 
 

/**
 *tcp_client - 生成socket号,连接tcp服务器
 *@host tcp服务器ip或者服务器名字
 *@port tcp服务器端口号
 *返回值 socket号
 */
int tcp_client(const char *host,unsigned int port)
{
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(-1 == sockfd)
    {
        err_exit("socket");
    }

    sockaddr_in seraddr;
    bzero(&seraddr,sizeof(seraddr));
    seraddr.sin_family=AF_INET;
    seraddr.sin_port=htons(port);
    
    if(host != NULL)
    {
        struct in_addr tmp_addr;
        if(inet_aton(host,&tmp_addr) != 0)  //host是ip地址
        {
            seraddr.sin_addr = tmp_addr;
        }
        else //host是主机名
        {
            hostent *hp = gethostbyname(host);
            if(NULL == hp)
            {
                err_exit("cannot parse the server name");
            }

            seraddr.sin_addr = *(struct in_addr*)hp->h_addr_list;
        }
    }
    else
    {
        //host为NULL时,连接本地服务器
        seraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    }

    socklen_t seraddr_len = sizeof(seraddr);
    if(connect(sockfd,(sockaddr *)&seraddr,seraddr_len)< 0) 
    {
        err_exit("connect");
    }

    return sockfd;
}


/**
 * getlocalip - 返回本机ip地址,不需要socket号
 * @ip:输出ip地址
 * 成功返回0,失败返回-1
 */
int getlocalip(char *ip)
{
    //获取主机名
    const int len = 100;
    char name[len] = {0};
    if(gethostname(name,len) < 0)
    {
        return -1;
    }

    //获取主机名下所有的ip地址
    struct hostent* hp = gethostbyname(name);
    if(NULL == hp)
    {
        return -1;
    }

    //一般来说hostent->h_addr_list的第一个ip是本地默认ip
    if(hp->h_addr_list[0] != NULL)
    {
        //strcpy(ip,inet_ntoa(*(struct in_addr*)(hp->h_addr_list[0])));
        strcpy(ip,inet_ntoa(*(struct in_addr*)(hp->h_addr))); //h_addr 等价于 h_addr_list[0]
    }
    else
    {
        return -1;
    }
  
    /*
    int i = 0;
    while(hp->h_addr_list[i] != NULL)
    {
        puts(inet_ntoa(*(in_addr *)(hp->h_addr_list[i])));
        ++i;
    }
    */

    return 0;
}


/**
 *getsockaddr - 获取本地端口和地址,适用于没有绑定就connet之后对使用的端口的获取
 *@sockfd 已连接或者已绑定的套接口
 *@port socket号对应的本地端口
 *@ip socket号对应的本地ip
 *返回值 0表示成功获取,-1表示出错
 */
int getsockaddr(int sockfd,int *port,char *ip)
{
    sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    int ret = getsockname(sockfd,(sockaddr *)&localaddr,&addrlen);
    if(-1 == ret)
    {
        return ret;
    }

    *port = ntohs(localaddr.sin_port);
    strcpy(ip,inet_ntoa(localaddr.sin_addr));

    return 0;
}

/**
 *active_nonblock - 将文件描述符设置为非阻塞
 *@fd:文件描述符
 */
void active_nonblock(int fd)
{

    int flag = fcntl(fd,F_GETFL,NULL);
    if(-1 == flag)
    {
        err_exit("fcntl");
    }

    if(fcntl(fd,F_SETFL,flag|O_NONBLOCK) < 0)
    {
        err_exit("fcntl");
    }
}

/**
 *deactive_nonblock - 将文件描述符设置为阻塞
 *@fd 文件描述符
 */
void deactive_nonblock(int fd)
{

    int flag = fcntl(fd,F_GETFL,NULL);
    if(-1 == flag)
    {
        err_exit("fcntl");
    }

    if(fcntl(fd,F_SETFL,flag&(~O_NONBLOCK)) < 0)
    {
        err_exit("fcntl");
    }
}


/**
 *readn - 读取n个字符,如果不足则阻塞
 *@fd socket号
 *@buf 读取缓冲区
 *@count 读取字节数
 *返回值 读取到的字符数,出错为-1
 */
ssize_t readn(int fd,void *buf,size_t count)
{
    ssize_t nread;
    ssize_t left = count;
    char *pbuf = (char *)buf;

    while(left > 0)
    {
        nread = read(fd,buf,left);
        if(nread < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }
            
            return -1;
        }
   
        //读取到数据末尾
        if(nread == 0)
        {
            break;
        }

        left -= nread;
        pbuf += nread;    
    }

    return  count - left;
}

/**
 *writen - 写入n个字符,如果不足则阻塞
 *@fd socket号
 *@buf 输入缓冲区
 *@count 写入字节数
 *返回值 写入socket到的字符数,出错为-1
 */
ssize_t writen(int fd,void *buf,size_t count)
{
    ssize_t left = count;
    ssize_t nwriten = 0;
    char *pbuf = (char*)buf;
    
    while(left > 0)
    {
        nwriten = write(fd,buf,left);
        if(nwriten < 0)
        {
            if(errno == EINTR)
            {
                continue;
            }

            return -1;
        }
        else if(nwriten == 0)
        {
            continue;
        }
        
        left -= nwriten;
        pbuf += nwriten;
    }

    return count - left;
}



/**
 *recv_peek 读取socket的缓冲区,但是数据不从缓冲区清除
 *@fd 读取的socket号
 *@buf 存放数据的缓冲区
 *@len 读取的长度
 *返回值 实际读取到的长度,如果为-1表示出错
 */
ssize_t recv_peek(int fd,void *buf,size_t len)
{
    ssize_t nrecv;
    
    while(1)
    {
        nrecv = recv(fd,buf,len,MSG_PEEK);
        if(nrecv == -1 && errno== EINTR)
        {
            continue;
        }
        break;
    }


    return nrecv;
}

/**
 *readline - 读取一行
 *@fd 读取的socket号
 *@buf 读取数据存放位置
 *@maxline 一行最大的长度
 *返回值 读取到的长度,如果为0表示,-1表示出错
 */
ssize_t readline(int fd,void *buf,size_t maxline)
{
    ssize_t nleft = maxline;
    char *pbuf = (char *)buf;
    ssize_t nread;

    while(nleft > 0)
    {
        
        nread = recv_peek(fd,pbuf,nleft);
        
        if(nread == -1)
        {
            if(errno == EINTR)
            {
                continue;
            }

            return nread;
        }
        else if(nread == 0)
        {
            return  nread;
        }

        //如果读取到的数据中有\n,则从将\n及其前面的读取出来
        for(int i=0;i<nread;++i)
        {
            if(pbuf[i] == '\n')
            {
                //memset(pbuf,0,nleft);
                nread = readn(fd,pbuf,i+1);
                if(nread != i+1)
                {
                    exit(EXIT_FAILURE);
                }
        
                return maxline-nleft+nread;
            }
        }


        //如果读取到的数据中没有\n,则全部读取出来
        if(nread > nleft)
        {
            exit(EXIT_FAILURE);
        }
        
        if(readn(fd,pbuf,nread) != nread)
        {
            exit(EXIT_FAILURE);
        }
        
        nleft -= nread;
        pbuf += nread;

    }

    //读取完maxline长度也没有遇到\n
    return -1;
}

/**
 *send_fd - 发送文件描述符
 *@sockfd socket号
 *@send_fd 发送的文件描述符
 */
void send_fd(int sockfd,int send_fd)
{
    struct msghdr msg;
    struct cmsghdr *p_cmsg; //指向辅助数据
    struct iovec vec;                        //缓冲区
    char cmsgbuf[CMSG_SPACE(sizeof(send_fd))]; //辅助信息缓冲区
    int *p_fds;
    
    char sendchar = 0; //我们不需发送正常数据,所以只定义一个字节的缓冲区
    
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_flags = 0;

    //填充缓冲区
    vec.iov_base = &sendchar;
    vec.iov_len = sizeof(sendchar);

    //填充辅助信息
    p_cmsg = CMSG_FIRSTHDR(&msg); //获取第一个辅助数据
    p_cmsg->cmsg_len = CMSG_LEN(sizeof(send_fd));
    p_cmsg->cmsg_level = SOL_SOCKET;
    p_cmsg->cmsg_type = SCM_RIGHTS;
    p_fds = (int *)CMSG_DATA(p_cmsg);
    *p_fds = send_fd;

    int ret;
    while((ret=sendmsg(sockfd,&msg,0))==-1 && errno==EINTR);
    if(-1 == ret)
    {
        err_exit("sendmsg");
    }
}



/**
 *recv_fd - 接收文件描述符
 *@sockfd:从socket号接收
 *返回接收的文件描述符
 */
int recv_fd(int sockfd)
{
    struct msghdr msg;
    char recvchar;
    struct iovec vec;
    int recv_fd;
    char cmsgbuf[CMSG_SPACE(sizeof(recv_fd))];
    struct cmsghdr *p_cmsg;
    int *p_fd;

    vec.iov_base = &recvchar;
    vec.iov_len = sizeof(recvchar);

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = &vec;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);
    msg.msg_flags = 0;

    p_fd = (int *)CMSG_DATA(CMSG_FIRSTHDR(&msg));
    *p_fd = -1;

    int ret;
    while((ret=recvmsg(sockfd,&msg,0)) == -1 && errno == EINTR) ;
   
    //发送的数据只有1字节 iov中
    if( 1 != ret)
    {
        err_exit("recvmsg");
    }


    p_cmsg = CMSG_FIRSTHDR(&msg);
    if(NULL == p_cmsg)
    {
        err_exit("no passed fd");
    }

    p_fd = (int *)CMSG_DATA(p_cmsg);
    recv_fd = *p_fd;

    if(-1 == recv_fd)
    {
        err_exit("no passed fd");
    }

    return recv_fd;
}



/**
 *reuse_addr - 设置监听端口重用
 *@fd 监听端口号
 */
void reuse_addr(int fd)
{
    int on = 1;
    if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) == -1)
    {
        err_exit("setsockopt");
    }
}


/**
 *write_pack 往sock中写入Pack_t结构体
 *@fd 写入的socket号
 *@buf 写入缓冲区
 *返回值 Pack_t结构体中的数据大小,-1表示失败
 */
int write_pack(int fd,Pack_t *buf)
{
    int len = ntohl(buf->len);
    if(writen(fd,buf,len+sizeof(buf->len)) != len)
    {
        return -1;
    }
    return len;
}

/**
 *read_pack - 读取Pack_t结构体 
 *@fd 数据源socket号
 *@buf 读取数据存放位置
 *返回值 读取到的Pack_t结构体中的buf缓冲大小
 */
int read_pack(int fd,Pack_t *buf)
{
    int len = 0;
    int nread = readn(fd,&len,sizeof(len));
    if(nread != sizeof(len))
    {
        return -1;
    }

    len = ntohl(len);
    
    nread = readn(fd,buf->buf,len);
    if(nread != len)
    {
        return -1;
    }

    return len;
}


/**
 *process_sigchld - SIGCHLD信号处理
 *@isIgnore 是否忽略
 */
void process_sigchld(bool isIgnore)
{
    if(isIgnore)
    {
        //忽略SIGCHLD
        signal(SIGCHLD,SIG_IGN);
    }
    else
    {
        //使用waitpid进行轮询
        signal(SIGCHLD,__handle_sigchld);
    }
}


/**
 *__handle_sigchld  - 子进程处理函数
 */
void __handle_sigchld(int sig)
{
    int status;
    while(waitpid(-1,&status ,WNOHANG) > 0)
    {

    }
}

/**
 *read_timeout - 读超时检测函数,不包含读操作
 *@fd:文件描述符
 *@wait_seconds:等待时间描述,如果<0表示不检测
 *成功返回0,失败返回-1,超时返回-1且errno=ETIMEDOUT
 */
int read_timeout(int fd,unsigned int wait_seconds)
{
    int ret = 0;
    if(wait_seconds > 0)
    {
        fd_set read_fdset;
        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        FD_ZERO(&read_fdset);
        FD_SET(fd,&read_fdset);

        do
        {
            ret = select(fd+1,&read_fdset,NULL,NULL,&timeout);
        }while(ret < 0 && errno == EINTR);

        if(ret == 0) //超时返回-1,errno=ETIMEDOUT
        {
            errno = ETIMEDOUT;
            ret = -1;
        }
        else if ( ret > 0) //成功返回0
        {
            ret = 0;
        }
    }

    return ret ;
}

/**
 *write_timeout - 写超时检测函数,不包含写操作
 *@fd:文件描述符
 *@wait_seconds:等待时间描述,如果<0表示不检测
 *成功返回0,失败返回-1,超时返回-1且errno=ETIMEDOUT
 */
int write_timeout(int fd,unsigned int wait_seconds)
{
    int ret = 0;
    if(wait_seconds > 0)
    {
        fd_set write_fdset;
        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        FD_ZERO(&write_fdset);
        FD_SET(fd,&write_fdset);

        do
        {
            ret = select(fd+1,NULL,&write_fdset,NULL,&timeout);
        }while(ret < 0 && errno == EINTR);

        if(ret == 0) //超时返回-1,errno=ETIMEDOUT
        {
            errno = ETIMEDOUT;
            ret = -1;
        }
        else if ( ret > 0) //成功返回0
        {
            ret = 0;
        }
    }

    return ret ;
}



/**
 *accept_timeout - accept超时检测函数,包含accept操作
 *@fd:文件描述符
 *@addr:accept返回的对方地址
 *@wait_seconds:等待时间描述,如果<0表示不检测
 *成功返回已连接的套接字,失败返回-1,超时返回-1且errno=ETIMEOUT
 */
int accept_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds)
{
    int ret = 0;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    
    if(wait_seconds > 0)
    {
        fd_set read_fdset;
        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;
        FD_ZERO(&read_fdset);
        FD_SET(fd,&read_fdset);

        do
        {
            ret = select(fd+1,&read_fdset,NULL,NULL,&timeout);
        }while(ret < 0 && errno == EINTR);

        if(ret == -1)
        {
            return -1;
        }
        else if(ret == 0) //超时返回-1,errno=ETIMEDOUT
        {
            errno = ETIMEDOUT;
            return -1;
        }
    }

    if(addr != NULL)
    {
        ret = accept(fd,(sockaddr *)addr,&addrlen);
        
    }
    else
    {
        ret = accept(fd,NULL,NULL);
    }

    if(ret < 0)
    {
        err_exit("accept");
    }
    
    return ret ;
}



/**
 *connect_timeout - connect超时检测函数,包含connect操作
 *@fd:文件描述符
 *@addr:accept返回的对方地址
 *@wait_seconds:等待时间描述,如果<0表示使用系统默认超时时间75秒
 *成功返回已连接的套接字,失败返回-1,超时返回-1且errno=ETIMEDOUT
 */
int connect_timeout(int fd,struct sockaddr_in *addr,unsigned int wait_seconds)
{
    int ret = 0;
    socklen_t addrlen = sizeof(struct sockaddr_in);
   
    //设置为非阻塞模式
    if(wait_seconds > 0)
    {
        active_nonblock(fd);    
    }

    //connect在非阻塞模式下会立即返回,在阻塞模式下会的等待默认1.5倍RTT75s
    ret = connect(fd,(sockaddr *)addr,addrlen);
    
    if(-1 == ret && errno == EINPROGRESS) //非阻塞模式时 可能无法返回成功
    {

        //使用select来检测fd是否可写
        fd_set connect_fdset;
        FD_ZERO(&connect_fdset);
        FD_SET(fd,&connect_fdset);
        struct timeval timeout;
        timeout.tv_sec = wait_seconds;
        timeout.tv_usec = 0;

        do
        {
            //一旦建立连接,套接字就可写
            ret = select(fd+1,NULL,&connect_fdset,NULL,&timeout);
        }while(ret < 0 && errno == EINTR);

        //超时
        if(ret == 0)
        {
            ret = -1;
            errno = ETIMEDOUT;
        }
        else if(ret == 1)
        {
            //ret = 1 有两种情况:1 连接建立成功 2套接字产生错误
            //套接字错误不会保存在errno中,需要使用getsockopt来获取
            int err;
            socklen_t socklen = sizeof(err);
            int sockopterr = getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,&socklen);
            if(-1 == sockopterr)
            {
                return -1;
            }

            if(0 == err) //没有错误
            {
                ret = 0; //连接建立成功
            }
            else
            {
                errno = err;
                ret = -1;
            }
        }
    }// end if ret == -1

    //文件描述符重新置为阻塞模式
    if(wait_seconds > 0)
    {
        deactive_nonblock(fd);
    }

    return ret;
}

