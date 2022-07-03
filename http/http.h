//
// Created by shanshan on 2022/5/21.
//

#ifndef WEBSERVER_HTTP_H
#define WEBSERVER_HTTP_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"
#include <sys/uio.h>

class http
{
public:
    http(){}
    ~http(){}
    
public:
    void init(int sockfd, const sockaddr_in& addr); // 初始化新接受的连接
    void close_conn();  // 关闭连接
    void process(); // 处理客户端请求
    bool read();// 非阻塞读
    bool write();// 非阻塞写

public:
    static const int FILENAME_LEN = 200;        // 文件名的最大长度
    static const int READ_BUFFER_SIZE = 2048;   // 读缓冲区的大小
    static const int WRITE_BUFFER_SIZE = 1024;  // 写缓冲区的大小

public:
    //HTTP请求方法，这里只支持GET
    enum METHOD{GET=0,POST,HEAD,PUT,DELETE,TRACE,OPTIONS,CONNECT};
    /*
        服务器处理HTTP请求的可能结果，报文解析的结果
        NO_REQUEST          :   请求不完整，需要继续读取客户数据
        GET_REQUEST         :   表示获得了一个完成的客户请求
        BAD_REQUEST         :   表示客户请求语法错误
        NO_RESOURCE         :   表示服务器没有资源
        FORBIDDEN_REQUEST   :   表示客户对资源没有足够的访问权限
        FILE_REQUEST        :   文件请求,获取文件成功
        INTERNAL_ERROR      :   表示服务器内部错误
        CLOSED_CONNECTION   :   表示客户端已经关闭连接了
    */
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    
    /*
     * 主状态机的状态：
     * CHECK_STATUS_REQUESTLINE:当前正在分析请求行
     * CHECK_STATUS_HEADER:当前正在分析请求头
     * CHECK_STATUS_CONTENT:当前正在分析请求体
     */
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
    
    // 从状态机的三种可能状态，即行的读取状态，分别表示
    // 1.读取到一个完整的行 2.行出错 3.行数据尚且不完整
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };
    
    //主从状态机对应的函数
    HTTP_CODE process_read();//解析http请求
    HTTP_CODE parse_request_line(char* text);//解析http请求首行
    HTTP_CODE parse_headers(char* text);//解析http请求头
    HTTP_CODE parse_content(char* text);//解析http请求体
    
    LINE_STATUS parse_line();

private:
    void init();    // 初始化连接
    HTTP_CODE process_read();    // 解析HTTP请求
    bool process_write( HTTP_CODE ret );    // 填充HTTP应答
    
    // 下面这一组函数被process_read调用以分析HTTP请求
    HTTP_CODE parse_request_line( char* text );
    HTTP_CODE parse_headers( char* text );
    HTTP_CODE parse_content( char* text );
    HTTP_CODE do_request();
    char* get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();
    
    // 这一组函数被process_write调用以填充HTTP应答
    void unmap();
    bool add_response( const char* format, ... );
    bool add_content( const char* content );
    bool add_content_type();
    bool add_status_line( int status, const char* title );
    bool add_headers( int content_length );
    bool add_content_length( int content_length );
    bool add_linger();
    bool add_blank_line();
    
public:
    static int m_epollfd;//所有socket上的事件都被注册到同一个epoll对象→用静态
    static int m_user_count;//统计用户的数量
private:
    int m_sockfd;//该http连接的socket
    sockaddr_int m_address;//对方的socket地址
    
    char m_read_buf[READ_BUFFER_SIZE];//读缓冲区
    int m_read_idx;//标识读缓冲区中已经读入的客户端数据的最后一个字节的下一个位置
    int m_checked_idx;//当前正在分析的字符在读缓冲区中的位置
    int m_start_line;//当前正在解析的行的起始位置
    
    CHECK_STATE m_check_state;
    METHOD m_method;
    
    
};

#endif //WEBSERVER_HTTP_H
