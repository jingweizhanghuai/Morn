
#include "morn_ptc.h"

// int counter=0;
// 
// MThreadSignal sgn = MORN_THREAD_SIGNAL;
// void mThreadExclusive0(void (*func)(void *),void *para)
// {
//     mThreadLockBegin(sgn);
//     func(para);
//     mThreadLockEnd(sgn);
// }
// int id=0;
// void mThreadExclusive1(void (*func)(void *),void *para)
// {
//     while(&id,0,mThreadID())==0);
//     func(para);
//     id=0;
// }
// 
// void add(void *para)
// {
//     mAtomicAdd(&counter,1);//++;
// }
// 
// void thread_func(void *para)
// {
//     for(int i=0;i<1000000;i++) add(NULL);
//         mThreadExclusive1(add,NULL);
// }

// int main()
// {
//     mTimerBegin();
//     mThread((thread_func,NULL),(thread_func,NULL),(thread_func,NULL),(thread_func,NULL),(thread_func,NULL),(thread_func,NULL));
//     mTimerEnd();
//     printf("counter=%d\n",counter);
// }

void func()
{
    for(int i=0;i<10;i++)
    {
        printf("running in thread %d,i=%d\n",mThreadID(),i);
        mSleep(100);
    }
    printf("thread %d end\n",mThreadID());
}

int main()
{
    int flag1=0,flag2=0;
    mThread(func,NULL,&flag1);
    mThread(func,NULL,&flag2);
    while(1)
    {
        mSleep(100);
        printf("flag1=%d,flag2=%d\n",flag1,flag2);
        if(flag1&&flag2) break;
    }
    return 0;
}

