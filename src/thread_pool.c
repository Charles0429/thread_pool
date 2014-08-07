#include "thread_pool.h"

thread_pool_t *thread_pool_create(int size)
{
    thread_pool_t *p;
    thread_t *master_thread;

    p = (thread_pool_t*)malloc(sizeof(thread_pool_t));
    if(p == NULL)
    {
        //LOG("malloc");
        return NULL;
    }

    master_thread = (thread_t *)malloc(sizeof(thread_t));
    if(master_thread == NULL)
    {
        //LOG("malloc");
        free(p);
        return NULL;
    }

    p->master_thread = master_thread;
    p->size = size;
    return p;
}

int thread_pool_init(thread_pool_t *p)
{
    return thread_pool_init_internal(p, THREAD_IDLE_LEVEL, 
                                     THREAD_BUSY_LEVEL, MASTER_INTERVAL);
}

int thread_pool_init_internal(thread_pool_t *p, double low_level, 
                              double high_level, int master_interval)
{
    int i;
    int size;
    int ret;
    thread_t *t = NULL;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(ret != 0)
    {
        //LOG
        return -1;
    }   

    pthread_mutex_init(&p->global, NULL);
    p->max_size = THREAD_POOL_MAX_THREADS;
    p->min_size = THREAD_POOL_MIN_THREADS;
    p->low_level = low_level;
    p->high_level = high_level;
    p->master_interval = master_interval;
    INIT_LIST_HEAD(&p->worker_queue);
    INIT_LIST_HEAD(&p->idle_queue);

    size = p->size;
    p->size = 0;
    for(i = 0; i < size; i++)
    {

        t = thread_create();
        if(t == NULL)
        {
            return -1;
        }     
        thread_add(p, t);

    }
    pthread_create(&p->master_thread->tid, &attr,
                   master_callback, (void *)p);
    pthread_attr_destroy(&attr);
    p->task_next = list_first_entry((&p->worker_queue), thread_t, worker_entry);
    return 0;
}

thread_t *thread_create(void)
{
    thread_t *t;

    t = (thread_t *)malloc(sizeof(thread_t));
    if(t == NULL)
    {
        //LOG
        return NULL;
    }
    return t;
}

int thread_add(thread_pool_t *p, thread_t *t)
{
    int ret;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(ret != 0)
    {
        //LOG
        pthread_attr_destroy(&attr);
        return -1;
    }

    INIT_LIST_HEAD(&t->task_queue);
    pthread_mutex_lock(&p->global);
    list_add_tail(&(t->worker_entry), &(p->worker_queue));
    list_add_tail(&(t->idle_entry), &(p->idle_queue));
    p->size++;
    pthread_mutex_unlock(&p->global);
    t->state = THREAD_STATE_IDLE;
    t->state = THREAD_RUNNING;
    t->queue_size = 0;
    t->tp = (void *)p;
    pthread_mutex_init(&t->mutex, NULL);
    pthread_cond_init(&t->cond, NULL);

    pthread_create(&t->tid, &attr,
                   worker_callback, (void *)t);
    pthread_attr_destroy(&attr);
   
    return 0;
}

void thread_del(thread_t *t)
{
    pthread_mutex_lock(&t->mutex);
    t->stop = THREAD_STOPPING;
    pthread_cond_signal(&t->cond);
    pthread_mutex_unlock(&t->mutex);
}

void thread_del_internal(thread_pool_t *p, thread_t *t)
{
    pthread_mutex_lock(&p->global);
    list_del(&t->worker_entry);
    list_del(&t->idle_entry);
    p->size--;
    pthread_mutex_unlock(&p->global);
    free(t);
}
   
void *worker_callback(void *arg)
{
    thread_t *curr = (thread_t *)arg;
    thread_pool_t *tp = (thread_pool_t *)(curr->tp);
    task_t *t;
    
    while(1)
    {
        pthread_mutex_lock(&curr->mutex);
        while(curr->queue_size == 0 && curr->stop != THREAD_STOPPING)
        {
            pthread_cond_wait(&curr->cond, &curr->mutex);
        }
        if(curr->stop == THREAD_STOPPING)
        {
            if(curr->queue_size == 0)
            {
                pthread_mutex_unlock(&curr->mutex);
                thread_del_internal(tp, curr);
                break;
            }
            else
            {
                pthread_mutex_unlock(&curr->mutex);
                continue;
            }
        }
        else
        {
            t = list_entry(curr->task_queue.next, task_t, entry);
            if(t)
            {
                t->task_callback(t->arg);
            }
            list_del(&t->entry);
            free(t);
            curr->queue_size--;
        }
        if(curr->queue_size == 0)
        {
            pthread_mutex_lock(&tp->global);
            list_add_tail(&(curr->idle_entry), &(tp->idle_queue));
            pthread_mutex_unlock(&tp->global);
            curr->state = THREAD_STATE_IDLE;
        }
        pthread_mutex_unlock(&curr->mutex);
    }
    return NULL;
}

void *master_callback(void *arg)
{
    thread_pool_t *p = (thread_pool_t *)arg;
    thread_t *t;

    sleep(p->master_interval);
    while(1)
    {
        int busy = 0;
        int idle = 0;
        
        pthread_mutex_lock(&p->global);
        list_for_each_entry(t, (&(p->worker_queue)), worker_entry)
        {
             if(t->state == THREAD_STATE_IDLE)
             {
                 idle++;
             }
             else if(t->state == THREAD_STATE_BUSY)
             {
                 busy++;
             }
        }
    
        double threshold = busy / p->size;
        printf("threshold:%.2lf low_level:%.2lf\n", threshold, p->low_level);
        if(threshold < p->low_level)
        {
             struct list_head *next;
             int delete_num = p->size * (p->low_level - threshold);
             while(delete_num--)
             {
                 next = p->idle_queue.next;
                 thread_t *t = list_entry(next, thread_t, idle_entry);
                 thread_del(t);
                 next = next->next;
                 printf("deleting a thread\n");     
             }
        }
        if(threshold > p->high_level)
        {
             int add_num = p->size * (p->high_level - threshold);
             while(add_num--)
             {
                 thread_t *t = thread_create();
                 if(t == NULL)
                 {
                     //log
                     break;
                 }
                 else
                 {
                     thread_add(p, t);
                 }
             }            
        }
            
        pthread_mutex_unlock(&p->global);
        printf("busy:%d, idle:%d\n", busy, idle);
        sleep(p->master_interval);
    }
}

task_t *task_create(void)
{
    task_t *t;

    t = (task_t *)malloc(sizeof(task_t));
    if(t == NULL)
    {
        //log
        return NULL;
    }
    return t;
}

void task_init(task_t *t, void* (*task_callback)(void *), void *arg)
{
    t->task_callback = task_callback;
    t->arg = arg;
}

void task_add(thread_pool_t *p, task_t *t)
{
    thread_t *th = NULL;
    thread_t *last = NULL;

    pthread_mutex_lock(&p->global);
    th = p->task_next;
    last = list_entry(p->worker_queue.prev, thread_t, worker_entry);
    if(th == last)
    {
        p->task_next = list_first_entry((&p->worker_queue), thread_t, worker_entry);
    }
    else
    {
        p->task_next = list_next_entry(th, worker_entry);
    }
    
    pthread_mutex_unlock(&p->global);    

    pthread_mutex_lock(&th->mutex);
    list_add_tail(&(t->entry), &(th->task_queue));
    th->queue_size++;
    if(th->queue_size == 1)
    {
        th->state = THREAD_STATE_BUSY;
        pthread_mutex_lock(&p->global);
        list_del(&(th->idle_entry));
        pthread_mutex_unlock(&p->global);
    }

    pthread_cond_signal(&th->cond);
    pthread_mutex_unlock(&th->mutex);
}
