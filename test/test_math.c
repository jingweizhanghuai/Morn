// gcc -O2 test_math.c -o test_math.exe -lmorn
#include "morn_math.h"

#define N 10000000

float error_calculate(double *rst1,double *rst2,int n)
{
    double err_max=0.0;
    for(int i=0;i<n;i++)
    {
        double err=ABS(rst1[i]-rst2[i])/MAX(ABS(rst1[i]),1.0);
        err_max=MAX(err,err_max);
    }
    return err_max;
}

void test_sqrt()
{
    double *a=mMalloc(N*sizeof(double));
    double *rst1=mMalloc(N*sizeof(double));
    double *rst2=mMalloc(N*sizeof(double));
    
    for(int i=0;i<N;i++)
        a[i]=(double)mRand(1,10000000000)/1234567.0;
    
    mTimerBegin( "sqrt"); for(int i=0;i<N;i++) rst1[i]= sqrt(a[i]); mTimerEnd( "sqrt");
    mTimerBegin("mSqrt"); for(int i=0;i<N;i++) rst2[i]=mSqrt(a[i]); mTimerEnd("mSqrt");
    printf("sqrt err_max=%e\n\n",error_calculate(rst1,rst2,N));
    
    mFree(a);
    mFree(rst1);
    mFree(rst2);
}

void test_exp()
{
    double *a=mMalloc(N*sizeof(double));
    double *rst1=mMalloc(N*sizeof(double));
    double *rst2=mMalloc(N*sizeof(double));
    
    for(int i=0;i<N;i++)
        a[i]=(double)mRand(-100000000,100000000)/1234567.0;
    
    mTimerBegin( "exp"); for(int i=0;i<N;i++) rst1[i]= exp(a[i]); mTimerEnd( "exp");
    mTimerBegin("mExp"); for(int i=0;i<N;i++) rst2[i]=mExp(a[i]); mTimerEnd("mExp");
    printf("exp err_max=%e\n\n",error_calculate(rst1,rst2,N));

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
    
    for(int i=0;i<N;i++)
    {
        a[i]=(double)mRand(-100000000,100000000)/1234567.0;
        b[i]=(double)mRand(1,100)/12.34567;
    }
    
    mTimerBegin("pow" ); for(int i=0;i<N;i++) rst1[i]= pow(b[i],a[i]); mTimerEnd("pow" );
    mTimerBegin("mPow"); for(int i=0;i<N;i++) rst2[i]=mPow(b[i],a[i]); mTimerEnd("mPow");
    printf("pow err_max=%e\n\n",error_calculate(rst1,rst2,N));

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
    
    for(int i=0;i<N;i++)
        a[i]=(double)mRand(1,100000000)/1234567.0;
    
    mTimerBegin("log"); for(int i=0;i<N;i++) rst1[i]=log(a[i]); mTimerEnd("log");
    mTimerBegin("mLn"); for(int i=0;i<N;i++) rst2[i]=mLn(a[i]); mTimerEnd("mLn");
    printf("ln err_max=%e\n\n",error_calculate(rst1,rst2,N));

    mTimerBegin("log10"); for(int i=0;i<N;i++) rst1[i]=log10(a[i]); mTimerEnd("log10");
    mTimerBegin("mLg"  ); for(int i=0;i<N;i++) rst2[i]= mLg( a[i]); mTimerEnd("mLg"  );
    printf("lg err_max=%e\n\n",error_calculate(rst1,rst2,N));

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
    
    for(int i=0;i<N;i++)
    {
        a[i]=(double)mRand(-1000000000,1000000000)/1234567.0;
        r[i]=a[i]*MORN_PI/180.0;
    }

    mTimerBegin("sin" ); for(int i=0;i<N;i++) rst1[i]= sin(r[i]); mTimerEnd("sin" );
    mTimerBegin("mSin"); for(int i=0;i<N;i++) rst2[i]=mSin(a[i]); mTimerEnd("mSin");
    printf("sin err_max=%e\n\n",error_calculate(rst1,rst2,N));

    mTimerBegin("cos" ); for(int i=0;i<N;i++) rst1[i]= cos(r[i]); mTimerEnd("cos" );
    mTimerBegin("mCos"); for(int i=0;i<N;i++) rst2[i]=mCos(a[i]); mTimerEnd("mCos");
    printf("cos err_max=%e\n\n",error_calculate(rst1,rst2,N));

    mTimerBegin("tan" ); for(int i=0;i<N;i++) rst1[i]= tan(r[i]); mTimerEnd("tan" );
    mTimerBegin("mTan"); for(int i=0;i<N;i++) rst2[i]=mTan(a[i]); mTimerEnd("mTan");
    printf("tan err_max=%e\n\n",error_calculate(rst1,rst2,N));
    
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
