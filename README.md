###introduction

The feature of this thread pool is as follows:

- each thread has one task queue
- dynamically add or remove threads from the thread pool according to the working states of threads
-  user interface is very simple

###usage

It's very simple to use this thread pool, see [thread\_pool\_test.c](test/thread_pool_test.c), just three steps

- use thread\_pool\_create to create a thread pool
- use thread\_pool\_init to init the thread pool
- use task\_add to add task into the thread pool

###documentation

Now, there is only chinese version.

- [CN DOC](doc/cn/README.md)

###todo

- support multiple task assignment strategies 
- etc..