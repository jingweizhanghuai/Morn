## Morn：内存池

在Morn的源码里，所有的内存申请和释放都是对Morn内存池的操作。

什么是内存池就不多废话了，百度一下全都知道。这里说一下为什么在Morn里要使用内存池（其实百度一下也都知道，不展开说了）。两个目的，首先是**避免内存碎片化**，这个是主要原因，其次是加快内存申请释放的速度，这个是打草搂兔子，这个速度不是很重要。

#### 接口

Morn的内存池接口函数就两个：

```C
void *mMalloc(int size);
void mFree(void *ptr);
```

`mMalloc`用来在内存池里面申请空间，其作用和C语言里面的`malloc`一样，使用方法也一样。

`mFree`用来把从内存池里申请的空间释放掉，其作用和C语言里面的`free`一样，使用方法也一样。

这里强调一点：所有使用`mMalloc`申请的内存空间，都必须而且只能用`mFree`来释放。所有`mFree`所释放的内存空间都必须而且只能是由`mMalloc`申请得到的。



#### 性能

比较一下`mMalloc`、`mFree`和原生的`malloc`、`free`的速度，写了下面的测试程序（Morn/test/test_memory.c文件）：

```c
#define TEST_NUM 10000000
int main()
{
    char *data[1024];
    memset(data,0,1024*sizeof(int *));

    mTimerBegin("Morn");
    for(int i=0;i<TEST_NUM;i++)
    {
        int idx = mRand(0,1024);
        if(data[idx]==NULL)
        {
            int size = mRand(0,16384);
            data[idx] = mMalloc(size);
        }
        else
        {
            mFree(data[idx]);
            data[idx] = NULL;
        }
    }
    for(int i=0;i<1024;i++)
        if(data[i]!=NULL)
            mFree(data[i]);
    mTimerEnd("Morn");
    return 0;
}
```

上面的代码也就是循环申请和释放随机大小的内存空间（在1字节至16K字节之间），得到的结果如下：

![test_memory](D:\Morn\doc\test_memory.png)

通过约一千万次的申请和释放，可以看到，在Morn的内存池上申请和释放内存大概比直接用libc中`malloc`、`free`快2倍左右。与当前流行的tcmalloc、jemalloc、mimalloc相比，其效率落后于tcmalloc，快于mimalloc和jemalloc。

