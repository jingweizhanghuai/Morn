#include "morn_ptc.h"

void test2();
void test3();
void test1()
{
    for(int i=0;;i++)
    {
        printf("running in test1,i=%d, goto test2...\n",i);
        mCoroutine(test2);
        printf("running in test1,i=%d, goto test3...\n",i);
        mCoroutine(test3);
    }
}

void test2()
{
    for(int i=0;;i++)
    {
        printf("running in test2,i=%d, goto test1...\n",i);
        mCoroutine(test1);
    }
}

void test3()
{
    for(int i=0;;i++)
    {
        printf("running in test3,i=%d, goto main ...\n",i);
        mCoroutine();
    }
}

int main()
{
    for(int i=0;i<8;i++)
    {
        printf("\nrunning in main ,i=%d, goto test1...\n",i);
        mCoroutine(test1);
    }
    return 0;
}
    
