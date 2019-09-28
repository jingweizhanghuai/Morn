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
#include "morn_util.h"

void mAscSortD64(D64 *data_in,int *index_in,D64 *data_out,int *index_out,int num);
void mAscSortF32(F32 *data_in,int *index_in,F32 *data_out,int *index_out,int num);
void mAscSortS32(S32 *data_in,int *index_in,S32 *data_out,int *index_out,int num);

int testS32(int n)
{
    int *data1=mMalloc(n*sizeof(int));
    int *data2=mMalloc(n*sizeof(int));
    
    for(int i=0;i<n;i++) {data1[i] = mRand(0-n,n);data2[i]=data1[i];}
    
    int compare(const void *v1, const void *v2) {return ((*((int *)v1))-(*((int *)v2)));}
    
    printf("S32 qsort:\n");
    mTimerBegin();
    qsort(data1,n,sizeof(int),compare);
    mTimerEnd();
    
    printf("S32 Morn sort:\n");
    mTimerBegin();
    mAscSortS32(data2,NULL,NULL,NULL,n);
    mTimerEnd();
    
    mFree(data1);
    mFree(data2);
}

int testD64(int n)
{
    double *data1=mMalloc(n*sizeof(double));
    double *data2=mMalloc(n*sizeof(double));
    
    for(int i=0;i<n;i++) {data1[i] = (double)mRand(0-n,n)/(double)n; data2[i]=data1[i];}
    
    int compare(const void *v1, const void *v2) {return ((*((double *)v1))>(*((double *)v2)))?1:-1;}
    
    printf("D64 qsort:\n");
    mTimerBegin();
    qsort(data1,n,sizeof(double),compare);
    mTimerEnd();
    
    printf("D64 Morn sort:\n");
    mTimerBegin();
    mAscSortD64(data2,NULL,NULL,NULL,n);
    mTimerEnd();
    
    mFree(data1);
    mFree(data2);
}

int main()
{
    testS32(1000000);
    testD64(1000000);
    return 0;
}




