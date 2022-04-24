// gcc -O2 test_math.c -o test_math.exe -lmorn
#include "morn_math.h"

// int main()
// {
//     double a=12.3456;
//     int64_t *b=(int64_t *)&a;
//     int c=(*b)>>52;
//     *b = ((*b)&0x000fffffffffffff)|0x3ff0000000000000;

//     printf("a=%f,c=%d\n",a,c);
// }

#define N 10000000

void test_sqrt()
{
    double *a=mMalloc(N*sizeof(double));
    double *rst1=mMalloc(N*sizeof(double));
    double *rst2=mMalloc(N*sizeof(double));
    double err_max;
    
    for(int i=0;i<N;i++)
        a[i]=(double)mRand(1,10000000000)/1234567.0;
    
    mTimerBegin("sqrt");
    for(int i=0;i<N;i++)
        rst1[i]=sqrt(a[i]);
    mTimerEnd("sqrt");

    mTimerBegin("mSqrt");
    for(int i=0;i<N;i++)
        rst2[i]=mSqrt(a[i]);
    mTimerEnd("mSqrt");

    err_max=0.0;
    for(int i=0;i<N;i++)
    {
        double err=ABS(rst1[i]-rst2[i]);
        err_max=MAX(err,err_max);
    }
    printf("sqrt err_max=%e\n\n",err_max);
    
    mFree(a);
    mFree(rst1);
    mFree(rst2);
}

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
    double *a=mMalloc(N*sizeof(double));
    double *b=mMalloc(N*sizeof(double));
    double *rst1=mMalloc(N*sizeof(double));
    double *rst2=mMalloc(N*sizeof(double));
    double err_max;
    
    for(int i=0;i<N;i++)
    {
        a[i]=(double)mRand(-100000000,100000000)/1234567.0;
        b[i]=(double)mRand(1,100)/12.34567;
    }
    
    mTimerBegin("pow");
    for(int i=0;i<N;i++)
        rst1[i]=pow(b[i],a[i]);
    mTimerEnd("pow");

    mTimerBegin("mPow");
    for(int i=0;i<N;i++)
        rst2[i]=mPow(b[i],a[i]);
    mTimerEnd("mPow");

    err_max=0.0;
    for(int i=0;i<N;i++)
    {
        double err=ABS(rst1[i]-rst2[i])/rst1[i];
        err_max=MAX(err,err_max);
    }
    printf("pow err_max=%e\n\n",err_max);

    mFree(a);
    mFree(b);
    mFree(rst1);
    mFree(rst2);
}

void test_log()
{
    double *a=mMalloc(N*sizeof(double));
    double *rst1=mMalloc(N*sizeof(double));
    double *rst2=mMalloc(N*sizeof(double));
    double err_max;
    
    for(int i=0;i<N;i++)
        a[i]=(double)mRand(1,100000000)/1234567.0;
    
    mTimerBegin("log");
    for(int i=0;i<N;i++)
        rst1[i]=log(a[i]);
    mTimerEnd("log");

    mTimerBegin("mLn");
    for(int i=0;i<N;i++)
        rst2[i]=mLn(a[i]);
    mTimerEnd("mLn");

    err_max=0.0;
    for(int i=0;i<N;i++)
    {
        double err=ABS(rst1[i]-rst2[i]);
        err_max=MAX(err,err_max);
    }
    printf("ln err_max=%e\n\n",err_max);

    mTimerBegin("log10");
    for(int i=0;i<N;i++)
        rst1[i]=log10(a[i]);
    mTimerEnd("log10");

    mTimerBegin("mLg");
    for(int i=0;i<N;i++)
        rst2[i]=mLg(a[i]);
    mTimerEnd("mLg");

    err_max=0.0;
    for(int i=0;i<N;i++)
    {
        double err=ABS(rst1[i]-rst2[i]);
        err_max=MAX(err,err_max);
    }
    printf("lg err_max=%e\n\n",err_max);

    mFree(a);
    mFree(rst1);
    mFree(rst2);
}

void test_sin_cos()
{
    double *r=mMalloc(N*sizeof(double));
    double *a=mMalloc(N*sizeof(double));
    double *rst1=mMalloc(N*sizeof(double));
    double *rst2=mMalloc(N*sizeof(double));
    double err_max;
    
    for(int i=0;i<N;i++)
    {
        a[i]=(double)mRand(-1000000000,1000000000)/1234567.0;
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

    mTimerBegin("tan");
    for(int i=0;i<N;i++)
        rst1[i]=tan(r[i]);
    mTimerEnd("tan");

    mTimerBegin("mTan");
    for(int i=0;i<N;i++)
        rst2[i]=mTan(a[i]);
    mTimerEnd("mTan");

    err_max=0;
    for(int i=0;i<N;i++)
    {
        if(rst2[i]==0.0) continue;
        double err=ABS((rst1[i]-rst2[i])/rst2[i]);
        // if(err>err_max) {err_max=err;printf("a[i]=%f,r[i]=%f,rst1[i]=%.10f,rst2[i]=%.10f,errr=%.10f\n",a[i],r[i],rst1[i],rst2[i],err);}
        err_max=MAX(err,err_max);
    }
    printf("tan err_max=%e\n\n",err_max);
    
    mFree(r);
    mFree(a);
    mFree(rst1);
    mFree(rst2);
}

int main()
{
    test_sqrt();
    test_exp();
    test_pow();
    test_log();
    test_sin_cos();
    return 0;
}