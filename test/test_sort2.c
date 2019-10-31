/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 test_sort2.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_sort2.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_math.h"

void test1_S32(int n)
{
    int *data1=mMalloc(n*sizeof(int));
    int *data2=mMalloc(n*sizeof(int));
    
    for(int i=0;i<n;i++) {data1[i] = mRand(0-n,n);data2[i]=data1[i];}
    
    printf("S32 qsort(num %d):\n",n);
    int compare(const void *v1, const void *v2) {return ((*((int *)v1))-(*((int *)v2)));}
    mTimerBegin();
    qsort(data1,n,sizeof(int),compare);
    mTimerEnd();
    
    printf("S32 Morn sort(num %d):\n",n);
    mTimerBegin();
    mAscSort(S32,data2,NULL,NULL,NULL,n);
    mTimerEnd();
    
    mFree(data1);
    mFree(data2);
}

void test1_D64(int n)
{
    double *data1=mMalloc(n*sizeof(double));
    double *data2=mMalloc(n*sizeof(double));
    
    for(int i=0;i<n;i++) {data1[i] = (double)mRand(0-n,n)/(double)n; data2[i]=data1[i];}
    
    printf("D64 qsort(num %d):\n",n);
    int compare(const void *v1, const void *v2) {return ((*((double *)v1))>(*((double *)v2)))?1:-1;}
    mTimerBegin();
    qsort(data1,n,sizeof(double),compare);
    mTimerEnd();
    
    printf("D64 Morn sort(num %d):\n",n);
    mTimerBegin();
    mAscSort(D64,data2,NULL,NULL,NULL,n);
    mTimerEnd();
    
    mFree(data1);
    mFree(data2);
}

void test2_S32(int n,int m)
{
    int *data=mMalloc(n*sizeof(int));
    for(int i=0;i<n;i++) data[i] = mRand(0-n,n);
    
    printf("S32 Morn subset(in %d, out %d):\n",n,m);
    mTimerBegin();
    mMinSubset(S32,data,NULL,n,NULL,NULL,m);
    mTimerEnd();
    
    mFree(data);
}

void test2_D64(int n,int m)
{
    double *data=mMalloc(n*sizeof(double));
    for(int i=0;i<n;i++) data[i] = (double)mRand(0-n,n)/(double)n;
    
    printf("D64 Morn subset(in %d, out %d):\n",n,m);
    mTimerBegin();
    mMinSubset(D64,data,NULL,n,NULL,NULL,m);
    mTimerEnd();
    
    mFree(data);
}

int main()
{
    test1_S32(1000000);
    test1_D64(1000000);
    // for(int m=100000;m<=900000;m+=200000) test2_S32(1000000,m);
    // for(int m=100000;m<=900000;m+=200000) test2_D64(1000000,m);
    return 0;
}




