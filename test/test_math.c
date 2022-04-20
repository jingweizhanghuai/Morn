// gcc -O2 test_math.c -o test_math.exe -lmorn
#include "morn_math.h"

int main()
{
    #define N 10000000
    double *r=malloc(N*sizeof(double));
    float *a =mMalloc(N*sizeof(float));
    double *rst1=malloc(N*sizeof(double));
    float *rst2 =mMalloc(N*sizeof(float));
    float err_max;
    
    for(int i=0;i<N;i++)
    {
        a[i]=(float)mRand(-1000000000,1000000000)/1234567.0;
        r[i]=a[i]*MORN_PI/180.0;
    }

    mTimerBegin("sin");
    for(int i=0;i<N;i++)
        rst1[i]=sin(r[i]);
    mTimerEnd("sin");

    mTimerBegin("mSin");
    for(int i=0;i<N;i++)
        rst2[i]=mSin(a[i]);
    mTimerEnd("mSin");

    err_max=0;
    for(int i=0;i<N;i++)
    {
        float err=ABS((float)rst1[i]-rst2[i]);
        err_max=MAX(err,err_max);
    }
    printf("sin err_max=%.10f\n",err_max);

    mTimerBegin("cos");
    for(int i=0;i<N;i++)
        rst1[i]=cos(r[i]);
    mTimerEnd("cos");

    mTimerBegin("mCos");
    for(int i=0;i<N;i++)
        rst2[i]=mCos(a[i]);
    mTimerEnd("mCos");

    err_max=0;
    for(int i=0;i<N;i++)
    {
        float err=ABS((float)rst1[i]-rst2[i]);
        err_max=MAX(err,err_max);
    }
    printf("cos err_max=%.10f\n",err_max);
    
    free(r);
    mFree(a);
    free(rst1);
    mFree(rst2);
    
    return 0;
}