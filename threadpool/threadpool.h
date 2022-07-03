//
// Created by shanshan on 2022/5/19.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H
#include <pthread.h>
#include "locker.h"
#include <list.h>
#include <cstdio.h>

//线程池类，对于不同的任务是不同的实现
template<typename T>
class threadpool{
public:
    threadpool(int thread_number=8,int max_request=10000);
    ~threadpool();
    bool append(T* request);
private:
    static void* worker(void* arg);
    void run();
private:
    //线程数量
    int m_thread_number;
    //线程的容器
    //用数组，大小即为线程数量
    pthread_t* m_threads;
    //请求队列中最多允许的等待处理的请求
    int m_max_request;
    //请求队列
    std::list<T*> m_workqueue;
    //互斥锁
    locker m_queuelock;
    //信号量，用来判断是否有任务需要处理
    sem m_queuesem;
    //是否结束线程
    bool m_stop;
};
template<typename T>
threadpool<T>::threadpool(int thread_number,int max_request):
m_thread_number(thread_number),
m_max_request(max_request),m_stop(false),m_thread(nullptr){
    if((thread_number<=0)||(max_request<=0)){
        throw std::exception();
    }
    m_threads=new pthread_t[m_thread_number];
    if(!m_threads){
        throw std::exception();
    }
    //创建thread_number个线程并设置为线程脱离(用完会自己进行销毁)
    for(int i=0;i<thread_number;i++){
        printf("create the %dth thread",i);
        if(pthread_create(m_threads+i,nullptr,worker, this)!=0){
            delete[] m_threads;
            throw std::exception();
        }
        if(pthread_detach(m_thread[i])){
            delete[] m_threads;
            throw std::exception();
        }
    }
}
template<typename T>
threadpool<T>::~threadpool() {
    delete[] m_threads;
    m_stop=true;
}
template<typename T>
bool threadpool<T>::append(T* request){
    m_queuelock.lock();
    if(m_workqueue>m_max_request){
        m_queuelock.unlock();
        return false;
    }
    m_workqueue.push_back(request);
    m_queuelock.unlock();
    m_queuesem.post();
    return true;
}
template <typename T>
void* threadpool<T>::worker(void *arg) {
    threadpool * pool=(threadpool *) arg;
    pool->run();//线程池需要从工作队列中取任务执行
    return pool;
}
template<typename t>
void threadpool<T>::run(){
    while(m_stop){
        m_queuesem.wait();
        m_queuelocker.lock();
        if(m_workqueue.empty()){
            m_queuelocker.unlock();
            continue;
        }
        T* request=m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        if(!request){
            continue;
        }
        request->process();
    }
}
#endif //WEBSERVER_THREADPOOL_H
