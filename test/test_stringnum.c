// gcc -O2 test_stringnum.c -o test_stringnum.exe -lmorn
#include "morn_util.h"

#define N 10000
#define T 1000
    
void test_atoi()
{
    char str[N][16];
    for(int i=0;i<N;i++)
    {
        int v=mRand();
        sprintf(&str[i][0],"%d",v);
    }
    int sum=0;

    mTimerBegin("atoi");
    for(int t=0;t<T;t++)
    {
        sum=0;
        for(int i=0;i<N;i++)
            sum += atoi(&str[i][0]);
    }
    mTimerEnd("atoi");
    printf("sum=%d\n",sum);
    
    mTimerBegin("mAtoi");
    for(int t=0;t<T;t++)
    {
        sum=0;
        for(int i=0;i<N;i++)
            sum += mAtoi(&str[i][0]);
    }
    mTimerEnd("mAtoi");
    printf("sum=%d\n",sum);
}

void test_atof1()
{
    char str[N][16];
    for(int i=0;i<N;i++)
    {
        double v=(double)mRand()/(double)mRand();
        sprintf(&str[i][0],"%.16f",v);
        str[i][15]=0;
    }
    double sum=0;

    mTimerBegin("atof");
    for(int t=0;t<T;t++)
    {
        sum=0;
        for(int i=0;i<N;i++)
            sum += atof(&str[i][0]);
    }
    mTimerEnd("atof");
    printf("sum=%f\n",sum);
    
    mTimerBegin("mAtof");
    for(int t=0;t<T;t++)
    {
        sum=0;
        for(int i=0;i<N;i++)
            sum+= mAtof(&str[i][0]);
    }
    mTimerEnd("mAtof");
    printf("sum=%f\n",sum);
}

void test_atof2()
{
    char str[N][16];
    for(int i=0;i<N;i++)
    {
        double v=(double)mRand()*(double)mRand()/(double)mRand()/(double)mRand();
        sprintf(&str[i][0],"%.8e",v);
        str[i][15]=0;
    }
    double sum=0;

    mTimerBegin("atof");
    for(int t=0;t<T;t++)
    {
        sum=0;
        for(int i=0;i<N;i++)
            sum += atof(&str[i][0]);
    }
    mTimerEnd("atof");
    printf("sum=%f\n",sum);
    
    mTimerBegin("mAtof");
    for(int t=0;t<T;t++)
    {
        sum=0;
        for(int i=0;i<N;i++)
            sum+= mAtof(&str[i][0]);
    }
    mTimerEnd("mAtof");
    printf("sum=%f\n",sum);
}

int main()
{
    test_atoi();
    test_atof1();
    test_atof2();
    return 0;
}


