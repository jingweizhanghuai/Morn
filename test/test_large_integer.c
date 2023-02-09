// build: gcc -O2 -fopenmp test_large_integer.c  -lmorn -lcrypto -lgmp -o test_large_integer.exe

#include "morn_math.h"
#include "gmp.h"
#include "openssl/bn.h"

int test_GMP(int k,char *rst)
{
    mpz_t a;
    mpz_init(a);
    
    mTimerBegin("GMP");
    for(int n=0;n<1000000/k;n++)
    {
        mpz_init_set_ui(a,1);
        for(int i=1;i<=k;i++)
            mpz_mul_ui(a,a,i);
    }
    mTimerEnd("GMP");
    
    gmp_sprintf(rst,"%Zd",a);
    mpz_clear(a);
}

int test_OpenSSL(int k,char *rst)
{
    BIGNUM *a=BN_new();
    
    mTimerBegin("OpenSSL");
    for(int n=0;n<1000000/k;n++)
    {
        BN_set_word(a,1);
        for(int i=1;i<=k;i++)
            BN_mul_word(a,i);
    }
    mTimerEnd("OpenSSL");
    
    strcpy(rst,BN_bn2dec(a));
    BN_free(a);
}

void test_Morn(int k,char *rst)
{
    MArray *a=mArrayCreate();
    
    mTimerBegin("Morn");
    for(int n=0;n<1000000/k;n++)
    {
        mIntToLInt(a,1);
        for(int i=1;i<=k;i++)
            mLIntMul(a,i);
    }
    mTimerEnd("Morn");
    
    mLIntToString(a,rst);
    mArrayRelease(a);
}

void test_Morn2(int k,char *rst)
{
    MArray *a=mArrayCreate();
    
    mTimerBegin("Morn");
    for(int n=0;n<1000000/k;n++)
        mFactorial(a,k);
    mTimerEnd("Morn");
    
    mLIntToString(a,rst);
    mArrayRelease(a);
}

int main()
{
    char *rst1=malloc(500000);
    char *rst2=malloc(500000);
    char *rst3=malloc(500000);
    char *rst4=malloc(500000);
    
    printf("\n(100!) for 10000 times:\n");
    test_GMP(100,rst1);
    test_OpenSSL(100,rst2);
    test_Morn(100,rst3);
    test_Morn2(100,rst4);
    mException((strcmp(rst1,rst3)!=0)||(strcmp(rst2,rst4)!=0),EXIT,"result error");
    
    printf("\n(1000!) for 1000 times:\n");
    test_GMP(1000,rst1);
    test_OpenSSL(1000,rst2);
    test_Morn(1000,rst3);
    test_Morn2(1000,rst4);
    mException((strcmp(rst1,rst3)!=0)||(strcmp(rst2,rst4)!=0),EXIT,"result error");
    
    printf("\n(10000!) for 100 times:\n");
    test_GMP(10000,rst1);
    test_OpenSSL(10000,rst2);
    test_Morn(10000,rst3);
    test_Morn2(10000,rst4);
    mException((strcmp(rst1,rst3)!=0)||(strcmp(rst2,rst4)!=0),EXIT,"result error");
    
    printf("\n(100000!) for 10 times:\n");
    test_GMP(100000,rst1);
    test_OpenSSL(100000,rst2);
    test_Morn(100000,rst3);
    test_Morn2(100000,rst4);
    mException((strcmp(rst1,rst3)!=0)||(strcmp(rst2,rst4)!=0),EXIT,"result error");
    
    free(rst1);
    free(rst2);
    free(rst3);
    free(rst4);
    return 0;
}

int test2_GMP(int k,char *rst)
{
    mpz_t a,b;
    mpz_init(a);mpz_init(b);
    
    mpz_init_set_ui(a,1);
    for(int i=1;i<=k;i++)
        mpz_mul_ui(a,a,i);
    
    mTimerBegin("GMP");
    for(int t=0;t<1000;t++)
        mpz_mul(b,a,a);
    mTimerEnd("GMP");
    
    gmp_sprintf(rst,"%Zd",b);
    mpz_clear(a);
    mpz_clear(b);
}

void test2_Morn(int k,char *rst)
{
    MArray *a=mArrayCreate();
    MArray *b=mArrayCreate();

    mIntToLInt(a,1);
    for(int i=1;i<=k;i++)
        mLIntMul(a,i);
    
    mTimerBegin("Morn");
    for(int t=0;t<1000;t++)
        mLIntMul(a,a,b);
    mTimerEnd("Morn");
    
    mLIntToString(b,rst);
    mArrayRelease(a);
    mArrayRelease(b);
}


int main2()
{
    char rst1[500000];
    char rst2[500000];
    
    test2_GMP(10000,rst1);
    test2_Morn(10000,rst2);
    
//     printf("rst1=%s\n",rst1);
//     printf("rst2=%s\n",rst2);
}

int main3()
{
    MArray *a=mArrayCreate();
    mTimerBegin("Morn1");
    mIntToLInt(a,1);
    for(int i=1;i<=100000;i=i+1)
        mLIntMul(a,i);
    mTimerEnd("Morn1");
    mArrayRelease(a);
    
    MArray *b=mArrayCreate();
    mTimerBegin("Morn2");
    mFactorial(b,100000);
    mTimerEnd("Morn2");
    mArrayRelease(b);
}



