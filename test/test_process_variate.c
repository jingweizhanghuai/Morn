#include "morn_ptc.h"

void slave()
{
    int *p = mProcSlaveVariate("test",sizeof(int));
    while(1)
    {
        mSleep(100);
        if(*p<0) break;
        printf("data=%d\n",*p);
    }
}

void master()
{
    int *p = mProcMasterVariate("test",sizeof(int));
    for(int i=0;i<100;i++)
    {
        *p=i;
        printf("data=%d\n",*p);
        mSleep(1000);
    }
    *p=-1;
}

int main(int argc,char *argv[])
{
    if(strcmp(argv[1],"slave")==0) slave();
    else                          master();
    return 0;
}

