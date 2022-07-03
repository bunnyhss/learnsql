//
// Created by shanshan on 2022/5/19.
//
#include <cstdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include "locker.h"
#include "threadpool.h"
#include <unistd.h>
#include <netinet.h>
#include <signal.h>
#include "http_conn.h"

using namespace std;

#define MAX_FD 65535
#define MAX_EVENT_NUMBER 10000
//添加信号捕捉
void addsig(int sig,void(handler)(int)){
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler=handler;
    sigfillset(&sa.sa_mask);
    sigaction(sig,&sa, nullptr);
}

//添加文件描述符到epoll的函数
extern void addfd(int epollfd,int fd,bool one_shot);
//从epoll中删除文件描述符的函数
extern void removefd(int epollfd,int fd);
//修改文件描述符
extern void modfd(int epollfd,int fd,int ev);
int main(int argc,char* argv[]){
    if(argc<=1){
        printf("按照如下格式运行：%s port_number\n",basename(argv[0]));
        exit(-1);
    }
    //获取端口号
    int port=atoi(argv[1]);
    //对SIGPIE信号进行处理
    addsig(SIGPIE,SIG_IGN);
    //创建线程池，初始化线程池
    threadpool<http_conn> *pool=nullptr;
    try{
        pool=new threadpool<http_conn>;
    }catch(...){
        exit(-1);
    }
    //创建一个数组用于保存所有的客户端信息
    http_conn * users=new http_conn[MAX_FD];
    
    int listenfd=socket(PF_INET,SOCK_STREAM,0);
    //设置端口复用(需要在绑定之前)
    int reuse=1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));
    //绑定
    struct sockaddr_in address;
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=INADDR_ANY;
    address.sin_port=htons(port);
    };
    bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    //监听
    listen(listenfd,5);
    //创建epoll对象，事件数组，添加监听的文件描述符
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd=epoll_create(5);
    //将监听的文件描述符添加到epoll对象中
    addfd(epollfd,listenfd,false);
    http_conn::m_epollfd=epollfd;
    while(true){
        int num = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if((num<0)&&(errno!=EINTR)){
            printf("epoll failure\n");
            break;
        }
        //循环遍历事件数组
        for(int i=0;i<num;i++){
            int sockfd=events[i].data.fd;
            if(sockfd==listenfd){
                //有客户端连接进来
                struct sockaddr_in client_address;
                sockaddr_t client_addrlen=sizeof(client_address);
                int connectfd=accept(listenfd,(struct sockaddr*)&client_address,&client_addrlen)
                if(http_conn::m_user_count>=MAX_FD){
                    //目前的连接数满了,提示客户端服务器内部忙碌
                    close(connectfd);
                    continue;
                }
                //将新的客户数据初始化放到用户信息数组中
                users[connectfd].init();
            }
    }
    
    
    
    return 0;
}