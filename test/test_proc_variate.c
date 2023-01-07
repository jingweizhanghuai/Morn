// gcc -O2 -fopenmp test_proc_variate.c -o test_proc_variate.exe -lmorn
#include "morn_ptc.h"

void master()
{
    uint64_t *p = mProcMasterVariate("my_variate",sizeof(uint64_t));
    for(int i=0;;i++)
    {
        if(i&0x01) *p=0xffffffffffffffff;
        else       *p=0;
        mSleep(1);
    }
}

void slave()
{    
    uint64_t *p = mProcSlaveVariate("my_variate",sizeof(uint64_t));
    while(1)
    {
        mSleep(1);
//         printf("p[0]=%lx\n",p[0]);
        mException((p[0]>0)&&(p[0]<0xffffffffffffffff),EXIT,"2222222222222"); 
    }
}

int main(int argc,char *argv[])
{
         if(strcmp(argv[1],"master")==0) master();
    else if(strcmp(argv[1],"slave" )==0)  slave();
    return 0;
}
