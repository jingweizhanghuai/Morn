// build: gcc -O2 -fopenmp test_large_integer.c  -lmorn -lcrypto -lgmp -o test_large_integer.exe

#include "morn_math.h"
#include "gmp.h"
#include "openssl/bn.h"

int test_GMP(int k,char *rst)
{
    mpz_t a;
    mpz_init(a);
    
    mTimerBegin("GMP");
//     for(int n=0;n<1000000/k;n++)
    for(int n=0;n<100000;n++)
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
//     for(int n=0;n<1000000/k;n++)
    for(int n=0;n<100000;n++)
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
//     for(int n=0;n<1000000/k;n++)
    for(int n=0;n<100000;n++)
    {
        mIntToLInt(a,1);
        for(int i=1;i<=k;i++)
            mLIntMul(a,i);
    }
    mTimerEnd("Morn");
    
    mLIntToString(a,rst);    
    mArrayRelease(a);
}
/*
struct LIntInfo
{
    uint8_t sign;
    uint8_t type;
    __int128_t data:112;
};*/

int main()
{
//     struct LIntInfo info;
//     info.data=-5;info.sign=0;info.type=3;
//     printf("sizeof(LIntInfo)=%ld\n",sizeof(struct LIntInfo));
//     printf("info.data=%ld\n",info.data);
//     return 0;
    
    
    char rst1[500000];
    char rst2[500000];
    char rst3[500000];
    
    printf("\n(100!) for 10000 times:\n");
    test_GMP(100,rst1);
    test_OpenSSL(100,rst2);
    test_Morn(100,rst3);
//     printf("rst1=%s\n",rst1);
    mException((strcmp(rst1,rst2)!=0)||(strcmp(rst2,rst3)!=0),EXIT,"result error");
    return 0;
    
    printf("\n(1000!) for 1000 times:\n");
    test_GMP(1000,rst1);
    test_OpenSSL(1000,rst2);
    test_Morn(1000,rst3);
    mException((strcmp(rst1,rst2)!=0)||(strcmp(rst2,rst3)!=0),EXIT,"result error");
    
    
    printf("\n(10000!) for 100 times:\n");
    test_GMP(10000,rst1);
    test_OpenSSL(10000,rst2);
    test_Morn(10000,rst3);
    mException((strcmp(rst1,rst2)!=0)||(strcmp(rst2,rst3)!=0),EXIT,"result error");
    
    
    printf("\n(100000!) for 10 times:\n");
    test_GMP(100000,rst1);
    test_OpenSSL(100000,rst2);
    printf("aaaaa\n");
    test_Morn(100000,rst3);
    mException((strcmp(rst1,rst2)!=0)||(strcmp(rst2,rst3)!=0),EXIT,"result error");
    return 0;
}
    
