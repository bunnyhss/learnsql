//
// Created by shanshan on 2022/5/21.
//
#include "http.h"

void addfd(int epollfd,int fd,bool one_shot)
{
    epoll_event event;
    event.data.fd=fd;
    event.events=EPOLLIN|EPOLLRDHUP;
    if(one_shot){
        event.events |=EPOLLONESHOT;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    
    setnonblocking(fd);//将添加的文件描述符设置为非阻塞
}
void removefd(int epollfd,int fd)
{
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
    close(fd);
}
void modfd(int epollfd,int fd,int ev)
{
    epoll_event event;
    event.data.fd=fd;
    event.events=ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);
}
//对静态变量赋值
int http::m_user_count=0;
int http::m_epollfd=-1;

bool http::read()
{
    if(m_read_idx>=READ_BUFFER_SIZE){
        return false;
    }
    int bytes_read=0;
    while(true){
        bytes_read=recv(m_sockfd,m_read_buf+m_read_idx,READ_BUFFER_SIZE-m_read_idx,0);
        if(bytes_read==-1){
            if(errno==EAGAIN || errno==EWOULDBLOCK){
                //没有数据
                break;
            }
            return false;
        }else if(bytes_read==0){
            //对方关闭连接
            return false;
        }
        m_read_idx+=bytes_read;
    }
    return true;
}
//解析一行，判断依据\r\n
LINE_STATUS http::parse_line()
{
    char temp;
    for(;m_checked_idx<m_read_idx;++m_checked_idx){
        temp=m_read_buf[m_checked_idx];
        if(temp=='\r'){
            if((m_checked_idx+1)==m_read_idx){
                return LINE_OPEN;//还没读完完整的一行
            }else if(m_read_buf[m_checked_idx+1]=='\n'){
                m_read_buf[m_checked_idx]='\0';
                m_read_buf[m_checked_idx+1]='\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }else if(temp=='\n'){
            if((m_checked_idx>1)&&(m_read_buf[m_checked_idx-1]=='\r')){
                m_read_buf[m_checked_idx-1]='\0';
                m_read_buf[m_checked_idx]='\0';
                m_checked_idx++;
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}
//解析http请求行，获得请求方法，目标URL，http协议版本号等
HTTP_CODE http::parse_request_line(char * text)
{
    // GET/index.html HTTP/1.1
    m_url = strpbrk(text,"\t");
    
}