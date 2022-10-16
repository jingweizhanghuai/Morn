// build: gcc -O2 test_process_message.c -lmorn -lpthread -o test_process_message.exe

#include "morn_ptc.h"

void publisher()
{
    char data[64];
    for(int i=0;i<1000;i++)
    {
        mSleep(mRand(0,500));
        mRandString(data,16,64);
        printf("string[%03d]= %s\n",i,data);
        mProcMessageWrite("string",data);
    }
}

void publisher1()
{
    char data[64];
    for(int i=0;i<1000;i++)
    {
        mSleep(50);
        mRandString(data,16,64);
        printf("string[%03d]= %s\n",i,data);
        mProcMessageWrite("string",data);
    }
}

void publisher2()
{
    char data[64];
    for(int i=0;i<1000;i++)
    {
        mSleep(200);
        mRandString(data,16,64);
        printf("string[%03d]= %s\n",i,data);
        mProcMessageWrite("string",data);
    }
}

void subscriber()
{
    for(int i=0;;i++)
    {
        mSleep(100);
        char *p=mProcMessageRead("string");
        if(p!=NULL) 
        {
            if(p[0]==0) return;
            printf("string[%03d]= %s\n",i,p);
        }
    }
}

int main(int argc,char *argv[])
{
         if(strcmp(argv[1],"publisher" )==0) publisher();
    else if(strcmp(argv[1],"publisher1")==0) publisher1();
    else if(strcmp(argv[1],"publisher2")==0) publisher2();
    else if(strcmp(argv[1],"subscriber")==0) subscriber();
    else if(strcmp(argv[1],"exit"      )==0) {char data=0;mProcMessageWrite("string",&data,1);}
    else printf("run as: \"test_process_topic.exe publisher\", or \"test_process_topic.exe subscriber\"\n");
    return 0;
}
