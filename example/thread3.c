#include "morn_ptc.h"

void test1(char *data)
{
    for(int i=0;i<26;i++)
    {
        printf("running in test1, data=%c\n",data[i]);
        mSleep(1);
    }
}

void test2(char *data)
{
    for(int i=0;i<26;i++)
    {
        printf("running in test2, data=%c\n",data[i]);
        mSleep(1);
    }
}

int main()
{
    char *data1=mMalloc(26);
    char *data2=mMalloc(26);
    for(int i=0;i<26;i++)
    {
        data1[i]='A'+i;
        data2[i]='a'+i;
    }
    mThread(test1,data1);
    mThread(test2,data2);
    
    mThreadJoin();
    mFree(data1);
    mFree(data2);
    return 0;
}
    
