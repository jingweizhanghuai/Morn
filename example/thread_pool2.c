#include "morn_ptc.h"

void func(char *str)
{
    mSleep(mRand(0,100));
//     printf("Thread %d input : %s\n",mThreadID(),str);
    for(int i=0;str[i];i++)
    {
             if((str[i]>='a')&&(str[i]<='z')) str[i]+=('A'-'a');
        else if((str[i]>='A')&&(str[i]<='Z')) str[i]+=('a'-'A');
    }
//     printf("Thread %d output: %s\n",mThreadID(),str);
}

int main()
{
    char str[1000][64];

    int thread_num =2;
    mPropertyWrite("ThreadPool","thread_num",&thread_num,sizeof(int));
    int thread_adjust=1;
    mPropertyWrite("ThreadPool","thread_adjust",&thread_adjust,sizeof(int));
    int thread_max = 8;
    mPropertyWrite("ThreadPool","thread_max",&thread_max,sizeof(int));

    for(int i=0;i<1000;i++)
    {
        mSleep(mRand(0,30));
        mRandString(&(str[i][0]),32,63);
        mThreadPool(func,&(str[i][0]));
        if(i%100==0)
        {
            mPropertyRead("ThreadPool","thread_num",&thread_num);
            printf("thread_num=%d\n",thread_num);
        }
    }
    mPropertyWrite("ThreadPool","exit");
    return 0;
}
