## Math

Morn provides a series of basic mathematical functions, Usually these will sacrifice little accuracy for faster speed.

### API

#### Exponent and Power

```c
double mExp(double v);
double mPow(double a,double v);
```

It is similar with `exp` and `pow` in C  standard library.



#### Logarithm 

```c
double mLn(double v);
double mLg(double v);
double mLog2(double v);
double mLogX(double x,double v);
```

`mLn` is similar with `log` in C standard library, and `mLg` is similar with `log10`, `mLog2` is similar with `mLog2`.

`mLogX` is a general function, which the base of  logarithm `x` must be given by users.



#### Root-Extraction

```c
double mSqrt(double v);
double mCurt(double v);
```

`mSqrt` is used to extract square root, and `mCurt` is used to extract cube root.



#### Trigonometric

```c
double mSin(double degree);
double mCos(double degree);
double mTan(double degree);
double mCot(double degree);
```

These 4 functions indicate sine cosine tangent and cotangent.

node:

Here the angle input must be degree, with 360 degrees for a cicle, and not radians. this is different with C

standard library.



#### Binary Round

```c
int mBinaryCeil(int data);
int mBinaryFloor(int data);
int mBinaryRound(int data);
```

These 3 function is used to get a integer, which will be 2 of integer power. Thus:

```c
mBinaryCeil(5)==8
mBinaryFloor(7)==4
mBinaryRound(5)==4
mBinaryRound(7)==8
```



### Performance

Here is a performance test:

```c
#define N 10000000

int main()
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
    
    return 0;
}
```

We generate 10000000 random data, and calculate with C-standard-library and Morn separately. More Similar Test can seen at [test_math.c]().

 The results are as following:

[![L5EoTg.png](https://s1.ax1x.com/2022/04/24/L5EoTg.png)](https://imgtu.com/i/L5EoTg) 



