// gcc -O2 test_coroutine.c -o test_coroutine.exe -lmorn
#include "morn_ptc.h"

void test1(int *c)
{
    printf("test1 begin\n");
    for(int i=0;*c;i++)
    {
        printf("running in test1,i=%d\n",i);
        mCoroutine();
    }
    printf("test1 end\n");
}

void test2_1(int *c)
{
    printf("test2_1 begin\n");
    for(int i=0;*c;i++)
    {
        printf("running in test2_1,i=%d\n",i);
        mCoroutine();
    }
    printf("test2_1 end\n");
}

void test2(int *c)
{
    printf("test2 begin\n");
    for(int i=0;*c;i++)
    {
        printf("running in test2,i=%d\n",i);
        mCoroutine(test2_1,c);
    }
    mCoroutine(test2_1,c);
    printf("test2 end\n");
}

void test3(int *c)
{
    printf("test3 begin\n");
    test1(c);
    printf("test3 end\n");
}

int main()
{
    printf("main begin\n");
    char *states[2]={"running...","done!"};
    int flag1,flag2,flag3;
    int c=1;
    int i=0;for(;i<4;i++)
    {
        printf("running in main,i=%d\n",i);
        
        mCoroutine(test1,&c,&flag1);
        printf("test1 is %s\n",states[flag1]);
        
        mCoroutine(test2,&c,&flag2);
        printf("test2 is %s\n",states[flag2]);
        
        mCoroutine(test3,&c,&flag3);
        printf("test3 is %s\n",states[flag3]);
        printf("\n");
    }
    
    c=0;
    mCoroutine(test1,&c,&flag1);
    printf("test1 is %s\n",states[flag1]);
    
    mCoroutine(test2,&c,&flag2);
    printf("test2 is %s\n",states[flag2]);
    
    mCoroutine(test3,&c,&flag3);
    printf("test3 is %s\n",states[flag3]);
    
    printf("main end\n");
    return 0;
}
    
