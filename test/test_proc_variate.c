// gcc -O2 -fopenmp test_proc_variate.c -o test_proc_variate.exe -lmorn
#include "morn_ptc.h"

void *mProcVariate(const char *name,int size);
void test1()
{
    float *p = mProcVariate("my_variate",sizeof(float));
    while(1)
    {
        printf("p=%p,*p=%f\n",p,*p);
        if((*p>100)||(*p<0)) break;
        mSleep(1);
    }
}

void test2()
{
    float *p = mProcVariate("my_variate",sizeof(float));
    for(int i=0;i<10000;i++)
    {
        mSleep(1);
        *p=i%100;
    }
}

int main(int argc,char *argv[])
{
         if(strcmp(argv[1],"test1")==0) test1();
    else if(strcmp(argv[1],"test2")==0) test2();

    return 0;
}
