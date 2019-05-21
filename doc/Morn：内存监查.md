## Morn：内存监查

可能很多C语言的初学者都遇到过一个头疼的问题——内存泄漏。在这个文档里，Morn的作者也反复的啰里啰嗦的强调过：所有的Create都需要Release，所有的Malloc都需要Free，因为如果没有这么做，就一定会有内存泄漏。

比遇见内存泄漏更让人头疼的问题是：程序员自己也不知道，到底是哪个Create了没有Release，或是Malloc了没有Free。

因此，Morn提供了一个工具，去做内存的监视和检查。

Morn的内存检查**只对Morn提供的mMalloc和mFree有效**。

Morn的内存监查会且只会在Debug模式下生效，而且是**自动生效，不需要其它额外代码**。如果你在编译Morn源代码的时候也使用了Debug模式，那么也可以同时监视Morn里面的内存使用情况（因此也就可以根据监视的结果来判断Create和Release的使用情况）。

在Debug模式下（开启内存监查），会额外占用部分内存，并且会吃掉一些速度。所以只适合去做内存检查，不适合用于发布版本。

Morn的内存监查可以用于检测以下三种异常：

* 申请了内存，但是没有释放。
* 没有申请内存，但是试图释放。
* 内存越界使用，即使用的内存比申请的内存多。

Debug模式下，查看内存使用情况，可以使用：

```c
void MemoryListPrint(int state);
```

其中输入参数state用于设置”你想查看哪种内存“，`MemoryListPrint(1)`表示查看申请后尚未释放的内存。`MemoryListPrint(1)`表示查看已被正常释放的内存。`MemoryListPrint(DFLT)`表示查看所有使用过的内存。

#### 示例

```c
#define DEBUG             //启用Debug模式，监视内存
#include "morn_util.h"

void test1()
{
    int *data1 = mMalloc(10*sizeof(int));
    int *data2 = mMalloc( 8*sizeof(int));
    mFree(data1);
    int *data3 = mMalloc(16*sizeof(int));
    mFree(data3);
    MemoryListPrint(1);
}

int test2()
{
    int *data1 = mMalloc(10*sizeof(int));
    int *data2 = mMalloc( 8*sizeof(int));
    mFree(data1);
    int *data3 = data2;
    mFree(data2);
    mFree(data1);
}

void test3()
{
    int *data = mMalloc(10*sizeof(int));
    for(int i=0;i<12;i++)
        data[i]=i;
    mFree(data);
}
```

test1中，有一处内存申请后没有释放，其执行结果为：

```
[test_Memory2.c:11 test1] memory 1, ptr is 0x00000000001A6C84,size is 32,state is 1
memory not free 1
memory list over
```

test2中，有一处内存并不需要释放，但是使用了mFree，其执行结果为：

```
[test_Memory2.c,line 25]Error: in function test2: free invalid memory
```

test3中，我们申请了10个int的空间，但是，使用了12个int，所以越界了，其执行结果为：

```
[test_Memory2.c,line 33]Error: in function test3: memory over used
```

可见，对于test2和test3的情况，在执行的时候就会报异常。

但是对于test1的情况，需要使用MemoryListPrint来打印内存情况，从而发现有内存异常。

所以一个建议是：如果你想监视内存，那么就在你的主函数最后一行（主函数返回前）加一句：`MemoryListPrint(1)`。



