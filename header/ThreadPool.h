#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <queue>
#include <pthread.h>
#include <iostream>
#include <unistd.h>

using namespace std;
// 函数类
class Task_t {
public:
    void* args; // 参数
    void (*function)(void*);// 函数指针
};

class ThreadPool {
private:
    // 任务队列
    Task_t* task_queue;
    int queue_front; // 队头指针
    int queue_rear; // 队尾指针
    int queue_size; // 长度
    int queue_capacity; // 容量

    // 线程池
    int min_num; // 线程池中最少的线程个数
    int max_num; // 最多线程个数
    int busy_num; // 忙线程个数
    int live_num; // 线程池中所有的线程个数
    int exit_num; // 线程数多了之后，需要删除的线程个数

    // 管理者线程与工作者线程
    pthread_t manager_id;
    pthread_t* worker_id;

    // 同步与互斥
    pthread_mutex_t mutex_pool; // 线程池的锁
    pthread_mutex_t mutex_busy; //忙线程个数的互斥锁
    pthread_cond_t cond_no_empty; // 条件变量，生产者唤醒消费者，阻塞消费者
    pthread_cond_t cond_no_full; // 条件变量，消费者唤醒生产者，阻塞生产者

    // 线程池关闭状态
    int pool_close;
public:
    ThreadPool(int min_num, int max_num, int capacity);
    void addTask(void (*function)(void*), void* arg);
    void destoryPool(void);
    friend void* manager(void* arg);
    friend void* worker(void* arg);
    friend void exit_thread(ThreadPool* pool);


};
void* manager(void* arg);
void* worker(void* arg);
void exit_thread(ThreadPool* pool);

#endif // !_THREAD_POOL_H
