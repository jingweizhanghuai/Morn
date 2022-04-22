// gcc -O2 test_math.c -o test_math.exe -lmorn
#include "morn_math.h"

#define N 10000000

void test_exp()
{
    double *a=mMalloc(N*sizeof(double));
    double *rst1=mMalloc(N*sizeof(double));
    double *rst2=mMalloc(N*sizeof(double));
    double err_max;
    
    for(int i=0;i<N;i++)
        a[i]=(double)mRand(-100000000,100000000)/1234567.0;
    
    mTimerBegin("exp");
    for(int i=0;i<N;i++)
        rst1[i]=exp(a[i]);
    mTimerEnd("exp");

    mTimerBegin("mExp");
    for(int i=0;i<N;i++)
        rst2[i]=mExp(a[i]);
    mTimerEnd("mExp");

    err_max=0.0;
    for(int i=0;i<N;i++)
    {
        double err=ABS(rst1[i]-rst2[i])/rst1[i];
        err_max=MAX(err,err_max);
    }
    printf("exp err_max=%e\n\n",err_max);

    mFree(a);
    mFree(rst1);
    mFree(rst2);
}

void test_pow()
{
    double *a1=mMalloc(N*sizeof(double));
    float  *a2=mMalloc(N*sizeof(float));
    double *b1=mMalloc(N*sizeof(double));
    float  *b2=mMalloc(N*sizeof(float));
    double *rst1=mMalloc(N*sizeof(double));
    double *rst2=mMalloc(N*sizeof(double));
    double err_max;
    
    for(int i=0;i<N;i++)
    {
        a2[i]=(float)mRand(-100000000,100000000)/1234567.0;a1[i]=a2[i];
        b2[i]=(float)mRand(1,100)/12.34567;                b1[i]=b2[i];
    }
    
    mTimerBegin("pow");
    for(int i=0;i<N;i++)
        rst1[i]=pow(b1[i],a1[i]);
    mTimerEnd("pow");

    mTimerBegin("mPow");
    for(int i=0;i<N;i++)
        rst2[i]=mPow(b2[i],a2[i]);
    mTimerEnd("mPow");

    err_max=0.0;
    for(int i=0;i<N;i++)
    {
        double err=ABS(rst1[i]-rst2[i])/rst1[i];
        err_max=MAX(err,err_max);
    }
    printf("pow err_max=%e\n\n",err_max);

    mFree(a1);
    mFree(a2);
    mFree(b1);
    mFree(b2);
    mFree(rst1);
    mFree(rst2);
}

void test_log()
{
    double *a1=mMalloc(N*sizeof(double));
    float  *a2=mMalloc(N*sizeof(float));
    double *rst1=mMalloc(N*sizeof(double));
    double *rst2=mMalloc(N*sizeof(double));
    double err_max;
    
    for(int i=0;i<N;i++)
    {
        a2[i]=(float)mRand(1,100000000)/1234567.0;
        a1[i]=a2[i];
    }
    
    mTimerBegin("log");
    for(int i=0;i<N;i++)
        rst1[i]=log(a1[i]);
    mTimerEnd("log");

    mTimerBegin("mLn");
    for(int i=0;i<N;i++)
        rst2[i]=mLn(a1[i]);
    mTimerEnd("mLn");

    err_max=0.0;
    for(int i=0;i<N;i++)
    {
        double err=ABS(rst1[i]-rst2[i]);
        err_max=MAX(err,err_max);
        // if(err>err_max) {err_max=err;printf("a=%f,rst1[i]=%f,rst2[i]=%f,err_max=%f\n",a1[i],rst1[i],rst2[i],err_max);}
    }
    printf("ln err_max=%e\n\n",err_max);

    mTimerBegin("log10");
    for(int i=0;i<N;i++)
        rst1[i]=log10(a1[i]);
    mTimerEnd("log10");

    mTimerBegin("mLg");
    for(int i=0;i<N;i++)
        rst2[i]=mLg(a1[i]);
    mTimerEnd("mLg");

    err_max=0.0;
    for(int i=0;i<N;i++)
    {
        double err=ABS(rst1[i]-rst2[i]);
        err_max=MAX(err,err_max);
        // if(err>err_max) {err_max=err;printf("a=%f,rst1[i]=%f,rst2[i]=%f,err_max=%f\n",a1[i],rst1[i],rst2[i],err_max);}
    }
    printf("lg err_max=%e\n\n",err_max);

    mFree(a1);
    mFree(a2);
    mFree(rst1);
    mFree(rst2);
}

void test_sin_cos()
{
    double *r=mMalloc(N*sizeof(double));
    float  *a=mMalloc(N*sizeof(float));
    double *rst1=mMalloc(N*sizeof(double));
    double *rst2=mMalloc(N*sizeof(double));
    double err_max;
    
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
        double err=ABS(rst1[i]-rst2[i]);
        // if(err>err_max) {err_max=err;printf("a[i]=%.10f,r[i]=%.10f,rst1[i]=%.10f,rst2[i]=%.10f,errr=%.10f\n",a[i],r[i],rst1[i],rst2[i],err);}
        err_max=MAX(err,err_max);
    }
    printf("sin err_max=%e\n\n",err_max);

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
        double err=ABS(rst1[i]-rst2[i]);
        // if(err>err_max) {err_max=err;printf("a[i]=%f,r[i]=%f,rst1[i]=%.10f,rst2[i]=%.10f,errr=%.10f\n",a[i],r[i],rst1[i],rst2[i],err);}
        err_max=MAX(err,err_max);
    }
    printf("cos err_max=%e\n\n",err_max);
    
    mFree(r);
    mFree(a);
    mFree(rst1);
    mFree(rst2);
}

int main()
{
    test_exp();
    test_pow();
    test_log();
    test_sin_cos();
    return 0;
}