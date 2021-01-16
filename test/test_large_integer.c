/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// 编译： gcc -O2 -fopenmp test_large_integer.c -I ..\include\ -I C:\ProgramFiles\CPackage\gmp\include -L ..\lib\x64_mingw\ -lmorn -L C:\ProgramFiles\CPackage\gmp\lib_x64_mingw -lgmp -o test_large_integer.exe

#include <stdio.h>
#include <stdlib.h>

#include "morn_math.h"
#include "gmp.h"

int main1()
{
    char str[200];
    MLInt a,b,c,d,e,f,g;

    for(int i=0;i<100;i++)
    {
        printf("i is %d\n",i);
        
        int a1 = mRand(0,0x7fffffff);
        int a2 = mRand(0,0x7fffffff);
        uint64_t a3 = (((uint64_t)a1)<<31)+(uint64_t)a2;
        mS64ToLInt(a3,&a);
        mLIntToString(&a,str);printf("a is %s\n",str);

        char b0[128];b0[0]=mRand('1','9');
        int j;for(j=1;j<mRand(64,128);j++) b0[j]=mRand('0','9');
        b0[j]=0;
        mStringToLInt(b0,&b);
        mLIntToString(&b,str);printf("b is %s\n",str);

        mLIntMulU32(&a,mRand(0,0x7fffffff),&c);
        mLIntToString(&c,str);printf("c is %s\n",str);

        mLIntMul(&b,&c,&d);          
        mLIntToString(&d,str);printf("d is %s\n",str);

        mLIntAdd(&d,&a,&e);          
        mLIntToString(&e,str);printf("e is %s\n",str);

        mLIntDiv(&e,&c,&f,&g);       
        mLIntToString(&f,str);printf("f is %s\n",str);
        mLIntToString(&g,str);printf("g is %s\n",str);

        int flag1=mLIntCompare(&f,&b);
        if(flag1!=0)printf("flag1 is %d\n",flag1);

        int flag2=mLIntCompare(&g,&a);
        if(flag2!=0)printf("flag2 is %d\n",flag2);
    }
    return 0;
}

int main()
{
    char str[128];
    mpz_t ga,gb,gc;
    mpz_init(ga);mpz_init(gb);mpz_init(gc);
    
    MLInt ma,mb,mc;

    strcpy(str,"-12345678900987654321");mpz_init_set_str(ga,str,10);mStringToLInt(str,&ma);
    strcpy(str,"98765432100123456789");mpz_init_set_str(gb,str,10);mStringToLInt(str,&mb);

    mTimerBegin();
    for(int i=0;i<1000000;i++)mpz_mul(gc, ga, gb);
    mTimerEnd();
    gmp_sprintf(str,"%Zd",gc);printf("c=%s\n",str);

    mTimerBegin();
    for(int i=0;i<1000000;i++)mLIntMul(&ma,&mb,&mc);
    mTimerEnd();
    mLIntToString(&mc,str);printf("c=%s\n",str);

    mpz_clear(ga);mpz_clear(gb);mpz_clear(gc);
    return 0;
}

// int main()
// {
//     char str[100];
//     MLInt a,b,c,d,e,f,g;
//     mS64ToLInt(12345678987654,&a);mLIntToString(&a,str);printf("a is %s\n",str);
//     char *num ="32101234567898765432101234567898765432101234567898765432101234567";
//     mStringToLInt(num,&b);       mLIntToString(&b,str);printf("b is %s\n",str);
//     mLIntMulU32(&a,8987654,&c);  mLIntToString(&c,str);printf("c is %s\n",str);
    
//     mLIntMul(&b,&c,&d);          mLIntToString(&d,str);printf("d is %s\n",str);
//     mLIntAdd(&d,&a,&e);          mLIntToString(&e,str);printf("e is %s\n",str);
    
//     mLIntDiv(&e,&c,&f,&g);       mLIntToString(&f,str);printf("f is %s\n",str);
//                                  mLIntToString(&g,str);printf("g is %s\n",str);
                                 
//     return 0;
// }
