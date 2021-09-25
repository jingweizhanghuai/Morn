## Morn：线程池

线程池是使用多线程开发应用的时候的常见用法，简单的讲就是集中创建和集中释放的多个线程，用户只需要将待执行的程序扔进线程池，由线程池调度其中的线程去执行函数。用户不需要关心自己的程序在哪个线程中被执行。

在Morn里，提供的线程池具有以下特征。

* 任务有优先级
* 可使用固定线程数量，也可以动态调整线程数量
* 接口简单（仅有一个函数接口）



### 接口

#### 使用线程池执行函数

```c
void mThreadPool(void (*func)(void *));
void mThreadPool(void (*func)(void *),void *para);
void mThreadPool(void (*func)(void *),void *para,int *flag);
void mThreadPool(void (*func)(void *),void *para,int *flag,float priority);
```

此函数用于将函数扔进线程池。

其中：

func是待处理的函数指针，para是函数参数。这里，func的参数是void*，如果需要传入多个参数，需要将参数封装成一个结构体，并将结构体指针传入。

flag是指向标志位的指针，函数在线程池中运行结束后，此标志位将被置为1，否则（尚未执行或正在执行中）标志位将被置为0，通过此标志位，可以获知函数的运行状态。如果不需获取运行状态，可传入NULL。

priority是函数的优先级（合法的优先级必须大于等于0），此值给线程池的调度提供参考，priority越小的函数越优先被执行，此值的默认值是0（优先级最高）。

线程池在调度时，优先级高的优先被执行，对于相同优先级的函数，按照扔进线程池的顺序执行，先进的先执行。已经在执行中的函数，不会被更高优先级的函数中断。

当线程池中所有线程都被占用的时候，后入的线程将按照优先级写入buffer，待有空闲线程时，再从buffer里根据优先级顺序依次执行。



#### 属性

Morn中使用函数`mPropertyWrite`函数设置线程池属性，详见：[Morn：对象属性]()

线程池模块名称为`"ThreadPool"`。其属性包括：



* **线程数量：thread_num：**

```c
//设置线程池中的线程数量为10
int n_thread=10;
mPropertyWrite("ThreadPool","thread_num",&n_thread,sizeof(int));
```

设置线程数量通常用于：

①在使用线程池之前设置。

②根据业务场景的改变，调整线程池中线程数量。

如果在使用线程池之前没有设置线程数量，则使用默认的线程数量（等于cpu核心数）。



* **线程动态调整：thread_adjust**
* **最大线程数量：thread_max**

```c
//设置允许动态调整线程数，且线程最多16个。
int adjust_valid = 1;
int max_thread = 16;
mPropertyWrite("ThreadPool","thread_adjust",&adjust_valid,sizeof(int));
mPropertyWrite("ThreadPool","thread_max",&max_thread,sizeof(int));
```

通常，在使用线程池之前设置是否允许线程数动态调整以及调整范围。

若没有设置，则默认动态调整处于禁能状态。

使能动态调整后，若未设置最大线程数量，则最大线程数量为当前线程数的2倍，即thread_max=2*thread_num。

线程数量的调整范围为：[2,thread_max)



* **关闭线程池：exit**

```c
mPropertyWrite("ThreadPool","exit“);
```

默认情况下，线程池将在进程结束前关闭。

如果需要提前关闭线程池，则需要设置exit。



### 示例

示例中，在线程池中执行如下函数（将一个字符串中的大写字母转成小写字母、小写字母转成大写字母）：

```c
void func(char *str)
{
    printf("Thread %d input : %s\n",mThreadID(),str);
    for(char *p=str;*p;p++)
    {
             if((*p>='a')&&(*p<='z')) *p+=('A'-'a');
        else if((*p>='A')&&(*p<='Z')) *p+=('a'-'A');
    }
    mSleep(mRand(0,100));
    printf("Thread %d output: %s\n",mThreadID(),str);
}
```

#### 示例一：线程池的最简单使用

```c
int main()
{
    char data[64];
    for(int i=0;i<100;i++)
    {
        mRandString(data,32,64);    //随机生成长度在32至64之间的字符串
        mThreadPool(func,data,DFLT);//扔进线程池
        mSleep(mRand(0,40));		//随机执行
    }
    printf("finish\n");
}
```

以上：随机生成了100个字符串（长度在32至64之间），并时间随机地将其扔进线程池执行大小写转换。此程序执行结果为：

```
Thread 1 input : F4SII41yLXswZJgcpvV5Z4fx5aOHVbaJGTbGndIwalIy
Thread 2 input : GUswOOtu5fFLJLIUm9BXVoxDJ4JBhTScqmHbt3IJayrqX0jN5rkc
Thread 1 output: f4sii41YlxSWzjGCPVv5z4FX5AohvBAjgtBgNDiWALiY
Thread 1 input : O8j2snOeGyVlSuAvfehMtBeJTi49nFBBcOIOXqcrOTE7k
Thread 2 output: guSWooTU5FfljliuM9bxvOXdj4jbHtsCQMhBT3ijAYRQx0Jn5RKC
Thread 3 input : 3yMFJtP7sAKvMqrPpCCf904CG2qyp1j1NY1DILcUSofBeiKE0imcivb1ezf8SCK
Thread 2 input : HmYUPUHqEgCWVkoAsCmVwgBCwaTDUyDfnBv0jchGJ7PJV28Y8j4IJlPyPpQeLC
Thread 4 input : wiA6sVNueo0nuifKiIbfZxWtSwxdLqAH0UwNE1BnewqFyCkE
Thread 3 output: 3YmfjTp7SakVmQRpPccF904cg2QYP1J1ny1dilCusOFbEIke0IMCIVB1EZF8sck
Thread 1 output: o8J2SNoEgYvLsUaVFEHmTbEjtI49NfbbCoioxQCRote7K
Thread 3 input : 2zKCrYqSZA4tErDbeAsYj2bjqiz3i9kS1G7O5TA6id4rodCb3t3lQVV
Thread 4 output: WIa6SvnUEO0NUIFkIiBFzXwTsWXDlQah0uWne1bNEWQfYcKe
...
Thread 2 input : J0F9euYmKG2UmswTsTUIHAQ5cVBDoolRO0Nl2cQrmOIihh
Thread 1 output: TWYsvYosDXcnD2iQlrcvB7EW4X1BImDUaLwedYaNXF7UJ
Thread 1 input : cI46qeRbI2rnPRKS4hJIBck26xM4sntqbpoyKoqSHYvLRybZwR1giQv9b
Thread 4 output: v1HvYRXW8160skf90G3dzvlFkZjsFODJgzayv6A1d7a4ooCwLPbmgWVBQZB06
finish
Thread 3 input : DS9waYspnJwNy8WSIhCageINcQdk2Q49ta6fRBPS2GhB
Thread 3 output: ds9WAySPNjWnY8wsiHcAGEinCqDK2q49TA6Frbps2gHb
Thread 2 output: j0f9EUyMkg2uMSWtStuihaq5CvbdOOLro0nL2CqRMoiIHH
Thread 1 output: Ci46QErBi2RNprks4HjibCK26Xm4SNTQBPOYkOQshyVlrYBzWr1GIqV9B
```

以上程序的线程池中，使用了默认的线程数量，且不允许线程数量动态调整。可以看到：程序执行中，各字符串的处理被分配在不同的线程中被并行执行。

#### 示例二：线程池的属性设置

```c
int main()
{
    char data[100][64];

    int thread_num =2;
    mPropertyWrite("ThreadPool","thread_num",&thread_num,sizeof(int));
    int thread_adjust=1;
    mPropertyWrite("ThreadPool","thread_adjust",&thread_adjust,sizeof(int));
    int thread_max = 8;
    mPropertyWrite("ThreadPool","thread_max",&thread_max,sizeof(int));

    mPropertyRead("ThreadPool","thread_num",&thread_num);
    printf("thread_num=%d\n",thread_num);
    
    for(int i=0;i<100;i++)
    {
        mRandString(&data[i][0],32,64);
        mThreadPool(func,&data[i][0]);
        mSleep(mRand(0,20));
    }

    mPropertyWrite("ThreadPool","exit");
    
    mPropertyRead("ThreadPool","thread_num",&thread_num);
    printf("thread_num=%d\n",thread_num);
    
    printf("finish\n");
    // mSleep(500);
}
```

以上：程序主体与示例一相同。设置线程池的初始线程数量为2，线程的最大数量为8，使能线程数自适应调整。此程序执行结果为：

```
thread_num=2
Thread 1 input : 80uB1yrBuLnFN3MR0h9ySs0AQody0fxRBqkaagtum1LPeeb
Thread 2 input : 4C0hQiwgeJbkqx8lu3TQGd31h9n5SapZJkREnYRFgHJ5j1GyEOoqgKzEBZa
Thread 1 output: 80Ub1YRbUlNfn3mr0H9YsS0aqODY0FXrbQKAAGTUM1lpEEB
Thread 1 input : UhY07j5tnc48WnmIaC6XzXEneOzDZdY4zYXewIvRUZ5Y1t9M
Thread 2 output: 4c0HqIWGEjBKQX8LU3tqgD31H9N5sAPzjKreNyrfGhj5J1gYeoOQGkZebzA
Thread 2 input : 05yzIbKvK5ALwSVwZTTaMFtd2zN8mtIblIdV6auiNg2dpgCUkVRu8LVJl
Thread 1 output: uHy07J5TNC48wNMiAc6xZxeNEoZdzDy4ZyxEWiVruz5y1T9m
Thread 1 input : DJ64NX9ipOa8ms1RdpEoyhXO8eSeVysHWsNXEdMk7DMkI7Qh5BlwOQCl
Thread 2 output: 05YZiBkVk5alWsvWzttAmfTD2Zn8MTiBLiDv6AUInG2DPGcuKvrU8lvjL
...
Thread 4 input : fniLHTWO4SvLxtuYb3icO13JXtNrwrZG4JtkEQmDYttos4M3zKokmuI8hk6rs
Thread 2 output: 9HDk178o3iicTYJ8BcQF5Oj5BZmeWK0mf9EgSl0T77O7qcEfEWVCwfeMOS
Thread 1 input : QXDRHvQ9NYB6juFEimjIVkGdiVsxe4LPJBA8qFEFWEG0ULV8aBDqJFRVcAj
Thread 3 input : LulSuJopDZlPod9hsxUiab2MQKqohzh4YA3SouzQm1
Thread 1 output: qxdrhVq9nyb6JUfeIMJivKgDIvSXE4lpjba8Qfefweg0ulv8AbdQjfrvCaJ
Thread 2 input : NtVVJ4ORIfurmsPYewRA6TBuNp3LnpchhmCGNAM3Dezre7YaZKyk
Thread 2 output: nTvvj4oriFURMSpyEWra6tbUnP3lNPCHHMcgnam3dEZRE7yAzkYK
thread_num=4
Thread 1 input : MsuP6AcH0duATFk5WI4NtOXLNJlwdTYKsDvdRskscemvgmA21MYMLqAr
Thread 4 output: FNIlhtwo4sVlXTUyB3ICo13jxTnRWRzg4jTKeqMdyTTOS4m3ZkOKMUi8HK6RS
Thread 3 output: lULsUjOPdzLpOD9HSXuIAB2mqkQOHZH4ya3sOUZqM1
Thread 1 output: mSUp6aCh0DUatfK5wi4nToxlnjLWDtykSdVDrSKSCEMVGMa21mymlQaR
finish
```

以上：可以看到，程序的初始线程数量为2，至程序结束时，线程数量被自适应调整为4。`printf("finish\n");`语句将在线程池退出后才被执行。

