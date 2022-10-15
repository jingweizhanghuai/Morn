#include "morn_ptc.h"

void master()
{
    int *p1 = mProcMasterVariate("data1",sizeof(int));
    int *p2 = mProcMasterVariate("data2",sizeof(int));
    for(int i=0;i<100;i++)
    {
        *p1=i+i;
        *p2=i+i+1;
        printf("data1=%d,data2=%d\n",*p1,*p2);
        mSleep(1000);
    }
    *p1=-1;
    *p2=-1;
}

void slave1()
{
    int *p = mProcSlaveVariate("data1",sizeof(int));
    while(1)
    {
        mSleep(100);
        if(*p<0) break;
        printf("data=%d\n",*p);
    }
}

void slave2()
{
    int *p = mProcSlaveVariate("data2",sizeof(int));
    while(1)
    {
        mSleep(100);
        if(*p<0) break;
        printf("data=%d\n",*p);
    }
}

int main(int argc,char *argv[])
{
         if(strcmp(argv[1],"slave1")==0) slave1();
    else if(strcmp(argv[1],"slave2")==0) slave2();
    else                                 master();
    return 0;
}

