#include "../src/thread_pool.h"

void *task_callback(void *arg)
{
    int i;
    int sum = 0;

    for(i = 0; i < 100; i++)
    {
        sum += i;
    }
    printf("sum:%d\n", sum);
    return NULL;
}

int main(void)
{
    int i;
    thread_pool_t *p = NULL;

    p = thread_pool_create(2);
    printf("%p\n", p);
    thread_pool_init(p, 0, 0);
    for(i = 0; i < 20; i++)
    {
        task_t *t = task_create();
        task_init(t, task_callback, NULL);
        task_add(p, t);
    }
    sleep(60);
    return 0;
}
