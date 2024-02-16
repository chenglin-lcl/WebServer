#include "ThreadPool.h"

ThreadPool::ThreadPool (int min_num, int max_num, int capacity) {
    // 初始化队列
    
    try{
        this->task_queue = new Task_t[capacity];
    } catch(std::bad_alloc& e){
        std::cout << "new task_queue failed!!!";
        return;
    }
    try {
        this->worker_id = new pthread_t[max_num](); // 按最大的来申请内存
    } catch (std::bad_alloc& e) {
        std::cout << "new worker_id failed!!!";
        delete[] this->task_queue;
        return;
    }
    // 初始化线程池 
    this->queue_capacity = capacity;
    this->min_num = min_num; // 线程池中最少的线程个数
    this->max_num = max_num; // 最多线程个数
    this->busy_num = 0; // 忙线程个数，在工作者线程取到任务之后才加1，工作完后-1
    this->live_num = min_num; // 线程池中所有的线程个数，在管理者线程添加线程之后+1，线程销毁后-1
    this->exit_num = 0; // 要关闭的线程个数
    // 初始化线程池的状态
    this->pool_close = 0;

    this->queue_size = 0;
    this->queue_rear = 0;
    this->queue_front = 0;

    // 初始化互斥锁
    pthread_mutex_init(&(this->mutex_pool), NULL);
    pthread_mutex_init(&(this->mutex_busy), NULL);

    // 初始化条件变量
    pthread_cond_init(&(this->cond_no_empty), NULL);
    pthread_cond_init(&(this->cond_no_full), NULL);

    // 创建管理者线程
    pthread_create(&(this->manager_id), NULL, manager, this);
    // pthread_detach(this->manager_id);
    // 创建工作者线程
    for (int i = 0; i < this->min_num; i++) {
        // std::cout << this->worker_id[i] << std::endl;
        pthread_create(&(this->worker_id[i]), NULL, worker, this); // 按最小的进行初始化
       // pthread_detach(this->manager_id);
    }
    std::cout << "ThreadPool created!" << std::endl;
    
}

void ThreadPool::addTask(void (*function)(void*), void* args) {
    // 向任务队列中插入函数
    pthread_mutex_lock(&(this->mutex_pool));
    //std::cout << "正在添加任务1" << std::endl;
    while (this->queue_size == this->queue_capacity && !this->pool_close) {
        //std::cout << "正在添加任务2" << std::endl;
        pthread_cond_wait(&(this->cond_no_full), &(this->mutex_pool));
       // std::cout << "正在添加任务3" << std::endl;
    }
    if (this->pool_close) {
        pthread_mutex_unlock(&(this->mutex_pool));
        return;
    }
    //std::cout << "正在添加任务4" << std::endl;
    this->task_queue[this->queue_rear].function = function;
    this->task_queue[this->queue_rear].args = args;
    this->queue_rear = (this->queue_rear+1) % this->queue_capacity;
    this->queue_size++;
    pthread_cond_signal(&(this->cond_no_empty));
    std::cout << "add task finished!" << std::endl;
    // std::cout << this->queue_size << std::endl;
    pthread_mutex_unlock(&(this->mutex_pool));
    
}

void ThreadPool::destoryPool(void) {
    // 回收线程资源
    this->pool_close = 1;
    pthread_join(this->manager_id, NULL);
    for (int i = 0; i < this->max_num; i++) {
        pthread_cond_signal(&(this->cond_no_empty));
    }
    // 回收内存资源
    pthread_mutex_destroy(&(this->mutex_pool));
    pthread_mutex_destroy(&(this->mutex_busy));
    pthread_cond_destroy(&(this->cond_no_empty));
    pthread_cond_destroy(&(this->cond_no_full));
    delete[] this->task_queue;
    delete[] this->worker_id;
     
}
void* manager(void* arg) {
    // 管理者线程就是要取检查工作线程数量，或者的线程数量
    // 根据这个数量，添加或者删除工作的线程数
    ThreadPool* pool = (ThreadPool*)arg;
    // 没有关闭的情况下进行
    while (!pool->pool_close) {
        sleep(3); // 过2秒之后开始工作


        // 先取个数，然后再运算
        pthread_mutex_lock(&(pool->mutex_pool));
        int live_num = pool->live_num;
        int queue_size = pool->queue_size;
        pthread_mutex_unlock(&(pool->mutex_pool));

        // 取值
        pthread_mutex_lock(&(pool->mutex_busy));
        int busy_num = pool->busy_num;
        pthread_mutex_unlock(&(pool->mutex_busy));

        // 当活着的线程数小于任务数的时候，应该增加线程
        if (live_num < queue_size && live_num < pool->max_num) {
            pthread_mutex_lock(&(pool->mutex_pool));
            // 每次添加两个线程
            for (int i = 0, count = 0; i < pool->max_num && live_num < pool->max_num && count < 2; i++) {
                if (pool->worker_id[i] == 0) { // 代表可用
                    pthread_create(&(pool->worker_id[i]), NULL, worker, pool); // 按最小的进行初始化
                    pool->live_num ++;
                    count++; // 不能放在for里面
                }
            }
            pthread_mutex_unlock(&(pool->mutex_pool));
        } 
        
        // 忙碌的线程个数远小于活着的线程个数时，应该减少线程
        if (busy_num * 2< live_num && live_num > pool->min_num) {
            // 销毁线程
            pthread_mutex_lock(&(pool->mutex_pool));
            pool->exit_num = 2;
            pthread_mutex_unlock(&(pool->mutex_pool));
            for (int i = 0; i < 2; i ++) {
                pthread_cond_signal(&(pool->cond_no_empty));
            }  
        }   
    }
    return NULL;
}

void* worker(void* arg) {
    // 工作者任务函数，其主要的工作就是进任务队列取任务
    // 工作者先要判断队列里面是不是有任务，或者线程池有没有关
    // 如果有任务就取任务，执行任务，如果没有任务就要自己阻塞，等待生产者将其唤醒
    // 难点：其中有个比较麻烦的就是关于线程池的关闭与否
    ThreadPool* pool = (ThreadPool*)arg;
    while (1) {
        // 任务队列的相关属性可能要被生产者访问，因此这里我们加一个锁
        pthread_mutex_lock(&(pool->mutex_pool));
        while (pool->queue_size == 0 && !pool->pool_close) {
            // 任务队列里面没有任务的时候，就要阻塞
            pthread_cond_wait(&(pool->cond_no_empty), &(pool->mutex_pool));
            if (pool->exit_num > 0) {
                pool->exit_num--;
                
                if (pool->live_num > pool->min_num) {
                    pool->live_num--;
                    pthread_mutex_unlock(&(pool->mutex_pool));
                    exit_thread(pool);
                }
                
                // return NULL;
            }
        }
        if (pool->pool_close == 1) { // 被关闭了后
            pthread_mutex_unlock(&(pool->mutex_pool)); // 解锁，避免死锁
            exit_thread(pool); // 并销毁线程，里面会有活着的线程数-1
            // return NULL;
        }
        
        // 开始取取任务，并且运行
        // 取出队头的任务
        Task_t task = pool->task_queue[pool->queue_front];
        pool->queue_front = (pool->queue_front+1) % (pool->queue_capacity); 
        pool->queue_size --;    
        // 唤醒生产者
        pthread_cond_signal(&(pool->cond_no_full));
        pthread_mutex_unlock(&(pool->mutex_pool));
        

        pthread_mutex_lock(&(pool->mutex_busy));
        // 忙的线程数就加1
        pool->busy_num ++; // 忙线程个数
        pthread_mutex_unlock(&(pool->mutex_busy));

        // 取出任务之后就运行任务
        task.function(task.args);
        
        pthread_mutex_lock(&(pool->mutex_busy));
        // 忙的线程数就减1
        pool->busy_num --; // 忙线程个数
        pthread_mutex_unlock(&(pool->mutex_busy));
        std::cout << "task completed!" << std::endl;
        free(task.args);
    }
    
    return NULL;

}

void exit_thread(ThreadPool* pool) {
    // 销毁线程
    pthread_t id = pthread_self();
    // 线程id清0
    for (int i = 0; i < pool->max_num; i ++) {
        if (id == pool->worker_id[i]) {
            pool->worker_id[i] = 0;
            cout << "thread " << id << " exit!" << endl;
            break;
        }
    }
    pthread_exit(NULL);

}