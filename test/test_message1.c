// gcc -O2 -fopenmp test_message1.c -o test_message1.exe -I ../include/ -L ../lib/x64_gnu/ -lmorn -lm
// gcc -O2 -fopenmp test_message1.c -o test_message1.exe -lmorn
// cl.exe -O2 -openmp -I ..\include\ ..\lib\x64_msvc\libmorn.lib .\test_message1.c
#include "morn_ptc.h"

void send_test()
{
    mTimerBegin();
    char data[4096];
    for(int i=0;i<10;i++)
    {
        mRandString(data,2048,4096);
        printf("i=%d,data=%s\n",i,data);
        mProcMessageWrite("msgtest",data);
        // mSleep(1);
    }
    mTimerEnd();
}

void recive_test()
{
    for(int i=0;;i++)
    {
        char *p=mProcMessageRead("msgtest");
        if(strcmp(p,"exit")==0) {printf("recive %d message\n");return;}
        printf("i=%d,data=%s\n",i,p);
    }
}

int main(int argc,char **argv)
{
         if(strcmp(argv[1],"send"  )==0)   send_test();
    else if(strcmp(argv[1],"recive")==0) recive_test();
    else mProcMessageWrite("msgtest",argv[1]);
    return 0;
}

