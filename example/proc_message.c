#include "morn_ptc.h"

void publisher()
{
    char data[64];
    for(int i=0;i<1000;i++)
    {
        mSleep(mRand(0,500));
        mRandString(data,16,64);
        printf("string[%04d]= %s\n",i,data);
        mProcMessageWrite("string",data);
    }
}

void subscriber()
{
    for(int i=0;;i++)
    {
        mSleep(100);
        char *p=mProcMessageRead("string");
        if(p[0]==0) return;
        printf("string[%04d]= %s\n",i,p);
    }
}

int main(int argc,char *argv[])
{
         if(strcmp(argv[1],"publisher" )==0) publisher();
    else if(strcmp(argv[1],"subscriber")==0) subscriber();
    else if(strcmp(argv[1],"exit"      )==0) mProcMessageWrite("string","\0");
    else printf("run as: \"test_process_topic.exe publisher\", or \"test_process_topic.exe subscriber\"\n");
    return 0;
}
