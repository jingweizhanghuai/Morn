## Morn：线程池

线程池是使用多线程开发应用的时候的常见用法，简单的讲就是集中创建和集中释放的多个线程，用户只需要将待执行的程序扔进线程池，由线程池调度其中的线程去执行函数。用户不需要关心自己的程序在哪个线程中被执行。

在Morn里，提供了一个带有优先级的线程池。

### 接口

在Morn里，线程池本质上就是一个容器（MList），这个容器里装着的是若干线程。

因此，使用线程池的时候，需要首先使用`mListCreate`来创建线程池，在程序结束时，需要使用`mListRelease`函数来释放线程池。创建线程池的时候需要指定线程的数量，此线程的数量需要用户根据业务量的大小和程序的处理速度来确定。

关于容器（MList）以及此容器的创建和释放，请参考文档[Morn：容器](./Morn：容器.md)。

```c
void mThreadPool(MList *pool,void (*func)(void *),void *para,int *flag,float priority);
```

此函数用于将函数扔进线程池。

其中pool就是用于存储线程的容器（即线程池）。

func是待处理的函数指针，para是函数参数。这里，func的参数是void*，如果需要传入多个参数，需要将参数封装成一个结构体，并将结构体指针传入。

flag是指向标志位的指针，函数在线程池中运行结束后，此标志位将被置为1，否则（尚未执行或正在执行中）标志位将被置为0，通过此标志位，可以获知函数的运行状态。如果不需获取运行状态，可传入NULL。

priority是函数的优先级（合法的优先级必须大于等于0），此值给线程池的调度提供参考，priority越小的函数越优先被执行，此值的默认值是0，若被设置为DFLT，表示优先级最高。

线程池在调度时，优先级高的优先被执行，对于相同优先级的函数，按照扔进线程池的顺序执行，先进的先执行。已经在执行中的函数，不会被更高优先级的函数中断。

当线程池中所有线程都被占用的时候，后入的线程将按照优先级写入buffer，待有空闲线程时，再从buffer里获取函数信息执行。

### 示例

```c
void func(void *para)
{
    int ID;PthreadOrder(ID);
    int *t = para;
    printf("func run on thread %d,sleep %dms\n",ID,*t);
    mSleep(*t);
}

int main()
{
    //创建一个有四个线程的线程池
    MList *pool = mListCreate(4,NULL);
    
    int t[100];
    for(int i=0;i<100;i++)
    {
        t[i]=mRand(10,120);
        // 把函数扔进线程池
        mThreadPool(pool,func,t+i,NULL,0);
        mSleep(mRand(5,30));
    }

    // 释放线程池
    mListRelease(pool);
}
```

此示例中，待执行的函数func，用于打印线程信息并sleep若干毫秒，传入的参数是sleep的毫秒数。

主函数中先创建了一个包含4个线程的线程池，然后分100次随机时间的将函数func扔进线程池，结束后释放了内存池。

函数的运行结果如下：

```
func run on thread 0,sleep 51ms
func run on thread 1,sleep 74ms
func run on thread 2,sleep 39ms
func run on thread 0,sleep 48ms
func run on thread 2,sleep 22ms
func run on thread 3,sleep 105ms
func run on thread 0,sleep 81ms
func run on thread 1,sleep 71ms
func run on thread 2,sleep 35ms
func run on thread 2,sleep 107ms
func run on thread 3,sleep 61ms
func run on thread 1,sleep 62ms
……
```



