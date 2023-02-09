#include "morn_ptc.h"

void test1()
{
    for(int i=0;;i++)
    {
        printf("running in test1,i=%d\n",i);
        mCoroutine();
    }
}

void test2()
{
    for(int i=0;;i++)
    {
        printf("running in test2,i=%d\n",i);
        mCoroutine();
    }
}

int main()
{
    for(int i=0;i<8;i++)
    {
        printf("\nrunning in main,i=%d\n",i);
        mCoroutine(test1);
        mCoroutine(test2);
    }
    return 0;
}
    
