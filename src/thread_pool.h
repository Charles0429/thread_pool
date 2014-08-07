#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "list.h"

#define THREAD_POOL_MAX_THREADS 10
#define THREAD_POOL_MIN_THREADS 1

#define THREAD_STATE_BUSY 0X1
#define THREAD_STATE_IDLE 0x2

#define THREAD_STOPPING 0x1
#define THREAD_RUNNING 0x2

#define THREAD_IDLE_LEVEL 0.2
#define THREAD_BUSY_LEVEL 0.8

#define MASTER_INTERVAL 30

typedef struct task_t 
{
    void *arg; /*the arg of task callback function*/
    void *(*task_callback)(void *args); /*task callback function*/
    struct list_head entry; /*a node in the task queue*/
}task_t;

typedef struct thread_t
{
    pthread_t tid;
    pthread_mutex_t mutex; /*mutex lock for task queue*/
    pthread_cond_t cond;
    int state; /*THREAD_STATE_BUSY or THREAD_STATE_IDLE*/
    int stop;  /*mark if the thread is going to quit*/
    int queue_size; /*the number of tasks in the task_queue*/
    struct list_head task_queue; /*the head of task queue*/
    struct list_head worker_entry; /*a node in the thread pool's worker queue*/
    struct list_head idle_entry; /*a node in the thread pool's idle queue*/
    void *tp;
}thread_t;

typedef struct thread_pool_t
{
    struct list_head worker_queue; /*the head of worker queue*/
    struct list_head idle_queue; /*the head of idle queue*/
    thread_t *master_thread; /*the struct of master thread*/
    pthread_mutex_t global; /*the global lock for thread pool*/
    thread_t *task_next; /*a task is going to be which thread*/
    int size; /*the number of threads*/
    int max_size; /*maxium number of threads*/
    int min_size; /*minium numbers of threads*/
    double low_level; /*the maxium proportion threshold of idle threads*/
    double high_level; /*the minium proportion threshold of idle threads*/
    int master_interval; /*the working interval of master thread*/
}thread_pool_t;

/*create a thread pool*/
thread_pool_t *thread_pool_create(int size);

/*init the thread pool*/
int thread_pool_init(thread_pool_t *);

/*init the thread pool internal*/
int thread_pool_init_internal(thread_pool_t *, double low_level, double high_level, int master_interval);

/*the function which master thread executes*/
void *master_callback(void *arg);

/*the function which a worker thread executes*/
void *worker_callback(void *arg);

/*create a thread*/
thread_t *thread_create(void);

/*add a thread to thread pool*/
int thread_add(thread_pool_t *, thread_t *);

/*delete a thread in a thread pool*/
void thread_del_internal(thread_pool_t *, thread_t *);

/*the interface of deleting a thread*/
void thread_del(thread_t *t);

/*create a task*/
task_t *task_create(void);

/*init a task*/
void task_init(task_t *, void* (*)(void *), void *);

/*add a task into a thread pool*/
void task_add(thread_pool_t *, task_t *);

#endif
