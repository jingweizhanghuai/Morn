#include "morn_ptc.h"

void test1()
{
    for(int i=0;i<10;i++)
    {
        printf("running in test1,i=%d\n",i);
        mSleep(1);
    }
}

void test2()
{
    for(int i=0;i<10;i++)
    {
        printf("running in test2,i=%d\n",i);
        mSleep(1);
    }
}

int main()
{
    mThread(test1);
    mThread(test2);
    return 0;
}
    
