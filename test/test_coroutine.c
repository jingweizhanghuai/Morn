// gcc -O2 test_coroutine.c -o test_coroutine.exe -lmorn
#include "morn_ptc.h"

int main();
void test2(int *c);

void test1(int *c)
{
    for(int i=0;*c;i++)
    {
        printf("running in test1,i=%d\n",i);
        mCoroutine(test2,c);
    }
    mCoroutine(test2,c);
    printf("test1 end\n");
}

void test2(int *c)
{
    for(int i=0;*c;i++)
    {
        printf("running in test2,i=%d\n",i);
        mCoroutine(main);
    }
    printf("test2 end\n");
}

void test3(int *c)
{
    for(int i=0;*c;i++)
    {
        printf("running in test3,i=%d\n",i);
        mCoroutine(main);
    }
    printf("test3 end\n");
}

int main()
{
    int c=1;
    int i=0;for(;i<4;i++)
    {
        printf("running in main ,i=%d\n",i);
        mCoroutine(test1,&c);
        mCoroutine(test3,&c);
    }
    c=0;
    mCoroutine(test1,&c);
    mCoroutine(test3,&c);
    
    printf("main end\n");
    return 0;
}

//     void count()
//     {
//         /*声明一个静态局部变量*/
//         static int num = 0;
//         num++;
//         printf("%d\n",num);
//     }
//     int main(void)
//     {
//         int i=0;
//         for (i = 0;i <= 5;i++)
//         {
//                 count();
//         }
//         return 0;
//     }
    
