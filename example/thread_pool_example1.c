#include "morn_ptc.h"

void func(char *str)
{
    printf("Thread %d input : %s\n",mThreadID(),str);
    mSleep(mRand(0,100));
    for(int i=0;str[i];i++)
    {
             if((str[i]>='a')&&(str[i]<='z')) str[i]+=('A'-'a');
        else if((str[i]>='A')&&(str[i]<='Z')) str[i]+=('a'-'A');
    }
    printf("Thread %d output: %s\n",mThreadID(),str);
}

int main()
{
    char str[16][64];
    for(int i=0;i<16;i++)
    {
        mSleep(mRand(0,30));
        mRandString(&str[i][0],32,64);
        mThreadPool(func,&str[i][0]);
    }
    mSleep(100);
    return 0;
}
