/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 test_sort.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_sort.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_math.h"

#define N 10
#define M 4

int main()
{
    int *data=mMalloc(N*sizeof(int));
    
    int *index=mMalloc(N*sizeof(int));
    
    printf(    "in :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mAscSort(data,NULL,NULL,NULL,N);
    printf( "\nout :");for(int i=0;i<N;i++) {printf("%d,",data[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mAscSort(data,NULL,NULL,index,N);
    printf(" \nout :");for(int i=0;i<N;i++) {printf("%d(%d),",data[i],index[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mDescSort(data,NULL,NULL,NULL,N);
    printf( "\nout :");for(int i=0;i<N;i++) {printf("%d,",data[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mDescSort(data,NULL,NULL,index,N);
    printf( "\nout :");for(int i=0;i<N;i++) {printf("%d(%d),",data[i],index[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mMinSubset(data,NULL,N,NULL,NULL,M);
    printf( "\nout :");for(int i=0;i<M;i++) {printf("%d,",data[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mMinSubset(data,NULL,N,NULL,index,M);
    printf( "\nout :");for(int i=0;i<M;i++) {printf("%d(%d),",data[i],index[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mMaxSubset(data,NULL,N,NULL,NULL,M);
    printf( "\nout :");for(int i=0;i<M;i++) {printf("%d,",data[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mMaxSubset(data,NULL,N,NULL,index,M);
    printf( "\nout :");for(int i=0;i<M;i++) {printf("%d(%d),",data[i],index[i]);}
    
    mFree(data);
    mFree(index);
}




