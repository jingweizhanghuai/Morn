#include "morn_ptc.h"

void test1(int *c)
{
    char *data=mMalloc(26);
    for(int i=0;i<26;i++) data[i]='A'+i;
    for(int i=0;*c;i++)
    {
        printf("running in test1, data=%c\n",data[i]);
        mCoroutine();
    }
    mFree(data);
    printf("\ntest1 finished!\n");
}

void test2(int *c)
{
    char *data=mMalloc(26);
    for(int i=0;i<26;i++) data[i]='a'+i;
    for(int i=0;*c;i++)
    {
        printf("running in test2, data=%c\n",data[i]);
        mCoroutine();
    }
    mFree(data);
    printf("\ntest2 finished!\n");
}

int main()
{
    int cycle=1;
    for(int i=0;i<8;i++)
    {
        printf("\nrunning in main,i=%d\n",i);
        mCoroutine(test1,&cycle);
        mCoroutine(test2,&cycle);
    }
    cycle=0;
    mCoroutine(test1,&cycle);
    mCoroutine(test2,&cycle);
        
    return 0;
}
    
