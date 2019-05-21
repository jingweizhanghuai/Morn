## Morn：MList栈和队列

Morn的作者认为：栈（stack）和队列（queue）都是有特殊读写规则的MList。对于栈，它遵循后入先出的规则。对于队列，它遵循先入先出的规则。

#### 栈

栈的类型是MList，所以在使用前需要先使用`mListCreate`创建栈，创建的时候，需要指定栈的容量。使用结束后需要使用`mListRelease`释放栈。

对于栈，Morn提供了三个接口：



##### 向栈中写入数据

```c
void *mStackWrite(MList *stack,void *data,int size);
```

可以看到，它和`mListWrite`的区别在于，`mStackWrite`不能控制写入的位置，这个位置（就是栈顶）是由Morn内部控制的。除此以外，它的用法与`mListWrite`没啥区别。

值得注意的是，当栈满时，将不能继续写入，如果写入必不成功，此时返回值将是NULL，通过此返回值可以判断是否栈满。



##### 从栈中读出数据

```c
void *mStackRead(MList *stack,void *data,int size);
```

同理，此函数用法与`mListRead`相似，但是不能控制读出的位置，只能从栈顶读出。

值得注意的是，当栈空时，将不能继续读出，如果读则必不成功，此时返回值将是NULL，通过此返回值可以判断是否栈空。



##### 获知栈中元素的个数

```c
int mStackSize(MList *stack);
```

返回值即为栈中元素的个数。注意，这个函数返回的是当前栈中有多少个元素，而不是栈的容量（栈的容量是stack->num）。当栈满时返回stack->num，当栈空时返回0。其它时候返回值在两者之间。



#### 队列

队列的类型是MList，所以在使用前需要先使用`mListCreate`创建队列，创建的时候，需要指定队列的容量。使用结束后需要使用`mListRelease`释放队列。

对于队列，Morn也是提供了三个相似的接口：



##### 向队列中写入元素

```c
void *mQueueWrite(MList *queue,void *data,int size);
```

用法同`mStackWrite`。写入的位置为队尾。队满时，写入将失败，返回NULL。



##### 从队列中读出元素

```c
void *mQueueRead(MList *queue,void *data,int size);
```

用法同`mStackRead`。读出的位置为队头。队空时，读出将失败，返回NULL。



##### 获知队列中元素的个数

```c
int mQueueSize(MList *stack);
```

返回值即为队列中元素的个数。当队列满时返回stack->num，当队列空时返回0。其它时候返回值在两者之间。



#### 示例

栈和队列常被用在多线程程序里，以下是一个队列的示例：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
MList *my_queue;
void write(float *data)
{
    int n=0;
    while(n<100)
    {
        pthread_mutex_lock(&mutex);
        void *p = mQueueWrite(my_queue,data+n,sizeof(float));
        pthread_mutex_unlock(&mutex);
        if(p==NULL) continue;
        n=n+1;
        Sleep(mRand(0,100));
    }
}
void read()
{
    int n=0;
    while(n<100)
    {
        pthread_mutex_lock(&mutex);
        float *data = mQueueRead(my_queue,NULL,DFLT);
        pthread_mutex_unlock(&mutex);
        if(data==NULL) continue;
        n=n+1;
        int size = mQueueSize(my_queue);printf("size is %d\t",size);
        printf("data is %f\n",*data);
        Sleep(mRand(0,100));
    }
}
int main()
{
    float in[100];
    for(int i=0;i<100;i++) in[i]=(float)i;
    
    if(my_queue ==NULL) my_queue = mListCreate(10,NULL);
    
    mThread(2,write(in),read());
    
    mListRelease(my_queue);
}
```

此例中，除了主进程外，有两个进程，写进程不断的往Queue里写入数据，读进程不断的从Queue里读出数据。