## Thread Pool

A thread-pool is a collection of worker threads that efficiently execute asynchronous callbacks on behalf of the application.  The thread pool is primarily used to reduce the number of application threads and provide management of the worker threads.

The following applications can benefit from using a thread pool:

- An application that is highly parallel and can dispatch numbers of small work items asynchronously (such as distributed index search or network I/O).
- An application that creates and destroys numbers of threads that each run for a short time. Using the thread pool can reduce the complexity of thread management and the overhead involved in thread creation and destruction.
- An application that processes independent work items in the background and in parallel (such as loading multiple tabs).
- An application that must perform an exclusive wait on kernel objects or block on incoming events on an object. Using the thread pool can reduce the complexity of thread management and increase performance by reducing the number of context switches.
- An application that creates custom waiter threads to wait on events.

Morn provide a thread-pool. It has the features bellow:

* Task priority is supported.
* Both fixed thread number and adjustable thread number are available.
* simple API



### API

#### Thread Pool

```c
void mThreadPool(void (*func)(void *));
void mThreadPool(void (*func)(void *),void *para);
void mThreadPool(void (*func)(void *),void *para,int *flag);
void mThreadPool(void (*func)(void *),void *para,int *flag,float priority);
```

It is used to throw a function to the thread pool.

`func` is the function pointer. If the form of `func` is:

```c
void func(void *para);
```

The `para` must be provided.

And if：

```c
void func();
```

No `para` is needed.

note: If function need more than 1 parameters, you must make all the parameters as a struct, and set `para`  as the struct pointer.

When the function throwed to thread-pool has return, the`flag` will be set as 1, otherwise, `flag` will be set 0. If the state of function is careless, `flag` is not necessary, or can be set NULL.

`priority` is provided to thread pool scheduler, to decide which function run first. A smaller value indicates a higher priority. DFLT priority is 0 (the highest priority).

note: 

Functions have the same priority will run by order of `mThreadPool`. 

A higher priority means the function can jump to front of queue, but other running functions will not be affected.



#### Property

In Morn we use `mPropertyWrite` to set module property and read it using `mPropertyRead`.

Here,`ThreadPool` is the module name.

* **thread_num：**

```c
//设置线程池中的线程数量为10
int n_thread=10;
mPropertyWrite("ThreadPool","thread_num",&n_thread,sizeof(int));
```

`thread_num` is the number of worker threads, it can be set when:

1. before using thread pool.
2. adjust thread number when business scenarios changes.

If this property not set, the pool will use the default threads number, which is equal to CPU number.



* **thread_adjust**
* **thread_max**

```c
//设置允许动态调整线程数，且线程最多16个。
int adjust_valid = 1;
int max_thread = 16;
mPropertyWrite("ThreadPool","thread_adjust",&adjust_valid,sizeof(int));
mPropertyWrite("ThreadPool","thread_max",&max_thread,sizeof(int));
```

Usually, we should decide whether thread number is adjustable before using the pool. and set `thread_adjust` true if necessary. `thread_adjust` is disable, if no property set.

If `thread_adjust` is enable, `thread_max` will be needed. It represent the maximum worker threads number can be adjusted in the pool. In other word, pool can adjust from 2 to `thread_max` threads. if `thread_max` not set, the default value is `2*thread_num`.



* **exit**

```c
mPropertyWrite("ThreadPool","exit“);
```

Thread pool will close and be released before the end of program, and `exit` is not necessary.

Write `exit` when you want to brought an early end.

note:

Write `exit`  will block until all functions in pool return.



### example

As an example, we run the functions below in the thread pool. which is used to convert case of characters in string.

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



#### Getting Start

```c
#include "morn_ptc.h"
int main()
{
    char data[64];
    for(int i=0;i<100;i++)
    {
        mRandString(data,32,64);    //generate a string with size from 32 to 64
        mThreadPool(func,data);		//throw it to thread pool
        mSleep(mRand(0,40));		//sleep for a random time
    }
    printf("finish\n");
}
```

Here we generate a random string, and then convert character case in worker thread.

Output is:

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

In the program above, we used a default threads number, and disable thread-number adjust. It can be seen that:

1. For this computer, default thread-number is 4.
2. functions run in different worker thread independently.
3. Main function will return until all sub-thread returns.



#### Property Set

```c
int main()
{
    char data[100][64];

    int thread_num =2;
    mPropertyWrite("ThreadPool","thread_num",&thread_num,sizeof(int));
    int thread_adjust=1;
    mPropertyWrite("ThreadPool","thread_adjust",&thread_adjust,sizeof(int));
    int thread_max = 6;
    mPropertyWrite("ThreadPool","thread_max",&thread_max,sizeof(int));

    mPropertyRead("ThreadPool","thread_num",&thread_num);
    printf("thread_num=%d\n",thread_num);
    
    for(int i=0;i<100;i++)
    {
        mRandString(&data[i][0],32,64);
        mThreadPool(func,&data[i][0]);
        mSleep(mRand(0,20));
    }
    mPropertyRead("ThreadPool","thread_num",&thread_num);
    mPropertyWrite("ThreadPool","exit");
    
    printf("thread_num=%d\n",thread_num);
    printf("finish\n");
}
```

Here, the function do the same thing with example above. But we initiate the threads number as 2, enable thread adjust, and set the maximum threads number as 6.

Output is:

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
Thread 1 input : MsuP6AcH0duATFk5WI4NtOXLNJlwdTYKsDvdRskscemvgmA21MYMLqAr
Thread 4 output: FNIlhtwo4sVlXTUyB3ICo13jxTnRWRzg4jTKeqMdyTTOS4m3ZkOKMUi8HK6RS
Thread 3 output: lULsUjOPdzLpOD9HSXuIAB2mqkQOHZH4ya3sOUZqM1
Thread 1 output: mSUp6aCh0DUatfK5wi4nToxlnjLWDtykSdVDrSKSCEMVGMa21mymlQaR
thread_num=4
finish
```

It can be seen that:

1. thread-number of the pool adjusted from 2 to 4.
2. it blocks when write `exit` to the pool.



