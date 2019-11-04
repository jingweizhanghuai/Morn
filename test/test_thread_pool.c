// gcc -O2 -fopenmp test_thread_pool.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_thread_pool.exe
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_util.h"

void mThreadPool(MList *pool,void (*func)(void *),void *para,int priority);

void func(void *para)
{
    pthread_t ID=pthread_self();
    int *t = para;
    printf("func run on thread %d,t is %d\n",ID,*t);
    mSleep(*t);
}

// int main1()
// {
//     int t[4];t[0]=mRand(10,100);t[1]=mRand(10,100);t[2]=mRand(10,100);t[3]=mRand(10,100);
//     mThread(4,func(t),func(t+1),func(t+2),func(t+3));
//     return 1;
// }


int main()
{
    MList *pool = mListCreate(4,NULL);
    
    int t[100];
    for(int i=0;i<100;i++)
    {
        t[i]=mRand(10,120);
        printf("t[%d] is %d\n",i,t[i]);
        mThreadPool(pool,func,t+i,0);
        mSleep(mRand(5,30));
    }

    mListRelease(pool);
}
    





    