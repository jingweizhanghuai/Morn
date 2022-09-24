## Chrono

If you want to do something some time later or in a periodic fashion, you may need chrono.

Chrono is a function which will block until reaching the specified time.

The  precision of Morn chrono is millisecond.



### API

#### Create Chrono Task

```c
int mChronoTask(int t);
int mChronoTask(int t,int n);
```

This is used to create a task, which will run every `t`ms for a total of `n` times. Endless loop if `n` is not been set or been set DFLT.

It returns a task ID, which is unique to  tell the difference between Tasks. The return ID is greater than or equal to 0. It will be used in `mChrono` function.

If no `mChronoTask` in program, a internal chrono task will be created, and the ID is DFLT.



#### Chrono

```c
int mChrono();
```

This function will blocks until reaching the time set by function `mChronoTask`.

It returns the task ID, tells which task-time reached.



#### Delete Chrono Task

```c
void mChronoDelete(int ID);
```

Chrono task will be automatically delete, when the task finished, and no `mChronoDelete` needed. 

But if you want to brought an early end, `mChronoDelete` can help. It is used in thus situation: when creating a task, you do not know how many runs is needed, and the task must be removed  when the conditions are ripe.

The input ID is the return of `mChronoTask`, which must greater than or equal to 0.



### Example

 Complete example file is [test_chrono.c](https://github.com/jingweizhanghuai/Morn/blob/master/test/test_chrono.c) .

#### Getting Start

```c
int main()
{
    mChronoTask(500);
    while(1)
    {
        mChrono();
        printf("time=%fms\n",mTime());
        ...
    }
    return 0;
}
```

Output is:

```
time=18446744073101352.000000ms
time=18446744073101852.000000ms
time=18446744073102352.000000ms
time=18446744073102852.000000ms
time=18446744073103352.000000ms
time=18446744073103852.000000ms
...
```



#### Multiple Task Chrono

```c
int main()
{
    int ID0=mChronoTask(200,4);int i0=0;
    int ID1=mChronoTask(300,3);int i1=0;
    int ID2=mChronoTask(500,2);int i2=0;
    printf("begin: time=%fms\n",mTime());
    while(1)
    {
        int ID=mChrono();
        if(ID<0) break;
        if(ID==ID0) printf("loop %d in task %d,time=%fms\n",ID,i0++,mTime());
        if(ID==ID1) printf("loop %d in task %d,time=%fms\n",ID,i1++,mTime());
        if(ID==ID2) printf("loop %d in task %d,time=%fms\n",ID,i2++,mTime());
        ...
    }
    return 0;
}
```

Here we create 3 tasks: task 1 runs 4 times for every 200ms, task 2 runs 3 times for every 300ms, task 3 runs 2 times for every 500ms.

Output is:

```
begin: time=18446744073007528.000000ms
loop 0 in task 0,time=18446744073007728.000000ms
loop 1 in task 0,time=18446744073007828.000000ms
loop 0 in task 1,time=18446744073007928.000000ms
loop 2 in task 0,time=18446744073008028.000000ms
loop 1 in task 1,time=18446744073008128.000000ms
loop 0 in task 2,time=18446744073008128.000000ms
loop 0 in task 3,time=18446744073008328.000000ms
loop 1 in task 2,time=18446744073008428.000000ms
loop 2 in task 1,time=18446744073008528.000000ms
```



#### Complex Task Chrono

```c
int main()
{
    int ID0 = mChronoTask(1000,10);
    int t=100;
    int ID1 = mChronoTask(t);
    printf("begin: time=%fms\n",mTime());
    while(1)
    {
        int ID = mChrono();
        if(ID==ID0)
        {
            mChronoDelete(ID1);
            t=t+100;if(t>500) break;
            ID1 = mChronoTask(t);
            printf("change cycle time: %dms\n",t);
        }
        else printf("time=%fms\n",mTime());
    }
    return 0;
}
```

Here we shows how to make a chrono with decreasing frequency. (The period increases by 100ms for every second.)

```
begin: time=18446744073238156.000000ms
time=18446744073238256.000000ms
time=18446744073238356.000000ms
time=18446744073238456.000000ms
time=18446744073238556.000000ms
time=18446744073238656.000000ms
time=18446744073238756.000000ms
time=18446744073238856.000000ms
time=18446744073238956.000000ms
time=18446744073239056.000000ms
time=18446744073239156.000000ms
change cycle time: 200ms
time=18446744073239356.000000ms
time=18446744073239556.000000ms
time=18446744073239756.000000ms
time=18446744073239956.000000ms
time=18446744073240156.000000ms
change cycle time: 300ms
time=18446744073240456.000000ms
time=18446744073240756.000000ms
time=18446744073241056.000000ms
change cycle time: 400ms
time=18446744073241556.000000ms
time=18446744073241956.000000ms
change cycle time: 500ms
time=18446744073242656.000000ms
time=18446744073243156.000000ms
```

