#include "morn_ptc.h"

void test1(int *c)
{
    for(int i=0;;i++)
    {
        if(*c==0) return;
        printf("running in test1,i=%d\n",i);
        mSleep(1);
    }
}

void test2(int *c)
{
    for(int i=0;;i++)
    {
        if(*c==0) return;
        printf("running in test2,i=%d\n",i);
        mSleep(1);
    }
}

int main()
{
    int cycle=1;
    mThread(test1,&cycle);
    mThread(test2,&cycle);
    mSleep(10);
    cycle=0;
    return 0;
}
    
