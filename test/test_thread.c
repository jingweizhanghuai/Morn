
#include "morn_ptc.h"

int counter=0;

MThreadSignal sgn = MORN_THREAD_SIGNAL;
void mThreadExclusive0(void (*func)(void *),void *para)
{
    mThreadLockBegin(sgn);
    func(para);
    mThreadLockEnd(sgn);
}
int id=0;
void mThreadExclusive1(void (*func)(void *),void *para)
{
    while(&id,0,mThreadID())==0);
    func(para);
    id=0;
}

void add(void *para)
{
    mAtomicAdd(&counter,1);//++;
}

void thread_func(void *para)
{
    for(int i=0;i<1000000;i++) add(NULL);
        // mThreadExclusive1(add,NULL);
}

int main()
{
    mTimerBegin();
    mThread((thread_func,NULL),(thread_func,NULL),(thread_func,NULL),(thread_func,NULL),(thread_func,NULL),(thread_func,NULL));
    mTimerEnd();
    printf("counter=%d\n",counter);
}

