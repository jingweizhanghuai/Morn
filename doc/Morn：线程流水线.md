## Morn：线程流水线

通常，当我们需要程序并行的时候，我们会遇见这样的需求，一个流程，可以被分成若干个工序，这里每一个工序由不同的线程来完成，多个线程并行执行，共同完成一个任务。简单说就是流水线。

一个流水线可以简单描述成：当工人1在操作工件A的时候，工人2同时在操作工件B，工人3同时在操作工件C，每个工人完成一道工序，下一个周期，工人1拿到了一个新的工件X，同时把工件A交给工人2处理，工人2把工件B交给工人3，然后处理工件A。工人3把工件C交付，然后处理工件B，同理，每个周期都依此进行下去。

可以看到，流水线的速度取决于最慢的那个工序。

再简单的说：流水线就是一个带同步功能的缓存器。

同步的意思是：每个工人都同步的移交工件，不能存在两个工人同时处理一个工件，也不能出现工件在线上丢失，当某一道工序提前完成的时候，线程将进入等待，直到所有的工人都完成操作后，才能获得下一工件。

缓存器的意思是：在流水线上时刻同时存在一定数量的工件，且工件的数量与工人的数量（线程数）相等（显然更大的缓存器也没有啥用）。

在Morn里面，流水线就是一个容器（MList），容器中的元素就是工件。



### 接口

显然，在使用流水线之前你需要先创建流水线`mListCreate`,在程序结束之前你需要释放流水线`mListRelease`。另外指定工件的个数（同时也是线程的个数）和工件的内存大小，这里使用`mListPlace`函数。这些函数，具体可见文档[Morn：容器](Morn：容器.md)。

#### 流水线

```c
void *mPipeline(MList *pipe,int order);
```

这个函数功能就是从流水线上获取数据。

pipe即流水线（容器）。

order即工序，每个线程对应流水线上唯一的工序，且：①order值越低，其工序的顺序越靠前（工序1在工序2的前面）。②所有工序必须是连续的（不能缺少某道工序）。③order的取值范围是0到pipe->num（左闭右开，工序总数与容器容量相同）。

函数的返回值是流水线中的元素的指针。流水线按照工序分配工件，每次执行时，按照顺序获得不同的元素。

对于每个线程，函数`mPipeline`是同步返回的。

如果流水线已经终止，获得的返回值将是NULL。所以对于“消费者”，需要检查返回值是否有效。



#### 流水线终止

```c
void mPipelineComplete(MList *list,int thread)；
```

这个函数通常由“生产者”执行，用以表示流水线至此结束。（这里只是为了说明，借用“生产者”和“消费者”的概念，这里的生产者通常是工序0，但是没有强制要求，任意工序都可以执行此函数）。

值得一提的是：在`mPipelineComplete`后，流水线并不会全部终止，例如工序2发出`mPipelineComplete`要求后，工序0和工序1都将返回NULL而停止，但工序3还将继续执行一个周期，工序4还将继续执行两个周期。

函数的参数与`mPipeline`相同，不再赘述。



### 示例

一个简单的示例

```c
struct PipeData
{
    int data0;
    int data1;
    int data2;
    float data3;
    float data4;
};
MList *pipe;
void step0() 
{
    for(int i=0;i<10;i++) 
    {
        struct PipeData *p=mPipeline(pipe,0);
        p->data0=mRand(0,100);
    } 	
    mPipelineComplete(pipe,0);
}
void step1() 
{
    while(1) 
    {
        struct PipeData *p=mPipeline(pipe,1);
        if(p==NULL)return;
        p->data1=p->data0+5;
    }
}
void step2() 
{
    while(1) 
    {
        struct PipeData *p=mPipeline(pipe,2);
        if(p==NULL)return;
        p->data2=p->data1*2;    
    }
}
void step3() 
{
    while(1) 
    {
        struct PipeData *p=mPipeline(pipe,3);
        if(p==NULL)return;
        p->data3=sqrt(p->data2);
    }
}
void step4() 
{
    while(1) 
    {
        struct PipeData *p=mPipeline(pipe,4);
        if(p==NULL)return;
        p->data4=p->data3/3.0;  
    }
}
void step5() 
{
    while(1) 
    {struct PipeData *p=mPipeline(pipe,5);
     if(p==NULL)return;
     printf("%d,%d,%d,%f,%f\n",p->data0,p->data1,p->data2,p->data3,p->data4);
    }
}

int main()
{
    pipe = mListCreate(DFLT,NULL);
    mListPlace(pipe,6,sizeof(struct PipeData));

    mThread(6,step0(),step1(),step2(),step3(),step4(),step5());

    mListRelease(pipe);
}
```

这里共有6个工序，其中工序0是“生产者”，它依次产生了10个随机数，工序1对工序0产生的数据加5，工序2在工序1的基础上乘2，工序3在工序2的基础上开平方，工序4在工序3的基础上除3，工序5把前面各个工序的值打印出来。

执行之，得到的结果如下：

```
83,88,176,13.266500,4.422166
86,91,182,13.490738,4.496912
77,82,164,12.806249,4.268750
15,20,40,6.324555,2.108185
93,98,196,14.000000,4.666667
35,40,80,8.944272,2.981424
86,91,182,13.490738,4.496912
92,97,194,13.928389,4.642796
49,54,108,10.392304,3.464102
21,26,52,7.211102,2.403701
```

这个示例只是为了说明Morn流水线的用法，实际上这么简单的操作根本就用不到多线程，也不应该使用流水线。



### 关于异常

这里关于异常，多说一点儿。

假设有工序0到工序5，共6道工序，如果这其中工序3异常而终止执行（没有执行`mPipelineComplete`函数），那么整个流水线都将停止，并且进入死锁状态。这显然是我们不想看到的。

处理的策略有二：

其一，即使遇到异常，线程也不退出，这你可以参考[Morn：异常处理](Morn：异常处理.md)的相关内容。

其二，一旦遇到异常，整个流水线所有线程都终止。例如上例中工序3，如果开方的数是负数，那么就认为是出现异常，整个流水线都退出，此时这段程序应改为：

```c
void step3() 
{
    while(1) 
    {
        struct PipeData *p=mPipeline(pipe,3);
        if(p==NULL)return;
        if(p->data2<0) break;
        p->data3=sqrt(p->data2);
    }
    mPipelineComplete(pipe,3);
}
```



