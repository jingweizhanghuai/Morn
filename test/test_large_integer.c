/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// build: gcc -O2 -fopenmp test_large_integer.c  -lmorn -lcrypto -lgmp -o test_large_integer.exe

#include "morn_math.h"

#include "gmp.h"

#include "openssl/bn.h"

// int main()
// {
//     MArray *a=mArrayCreate();
//     mIntToLInt(a,1);
//     for(int i=1;i<=100;i++)
//         mLIntMul(a,i);
//     
//     char rst[4000];
//     mLIntToString(a,rst); 
//     printf("rst=%s\n",rst);
//     mArrayRelease(a);
// }

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

int main()
{
    char rst1[40000];
    char rst2[40000];
    char rst3[40000];
    
    printf("\n");
    test_GMP(100,rst1);
    test_OpenSSL(100,rst2);
    test_Morn(100,rst3);
    mException((strcmp(rst1,rst2)!=0)||(strcmp(rst2,rst3)!=0),EXIT,"result error");
    
    printf("\n");
    test_GMP(1000,rst1);
    test_OpenSSL(1000,rst2);
    test_Morn(1000,rst3);
    mException((strcmp(rst1,rst2)!=0)||(strcmp(rst2,rst3)!=0),EXIT,"result error");
    
    printf("\n");
    test_GMP(10000,rst1);
    test_OpenSSL(10000,rst2);
    test_Morn(10000,rst3);
    mException((strcmp(rst1,rst2)!=0)||(strcmp(rst2,rst3)!=0),EXIT,"result error");
}
    
