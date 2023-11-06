#include "morn_ptc.h"

void test1()
{
    for(int i=0;i<5;i++)
    {
        printf("running in test1,i=%d\n",i);
        mCoroutine();
    }
    printf("test1 finished!\n");
}

void test2()
{
    for(int i=0;i<7;i++)
    {
        printf("running in test2,i=%d\n",i);
        mCoroutine();
    }
    printf("test2 finished!\n");
}

int main()
{
    int flag1,flag2;
    for(int i=0;;i++)
    {
        printf("\nrunning in main,i=%d\n",i);
        mCoroutine(test1,NULL,&flag1);
        mCoroutine(test2,NULL,&flag2);
        if(flag1&&flag2) break;
    }
    return 0;
}
