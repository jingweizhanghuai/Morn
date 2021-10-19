/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
//编译： gcc -O2 test_sort.c -lmorn -o test_sort.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_math.h"

#define N 16
#define M 4

int main()
{
    int *data=mMalloc(N*sizeof(int));
    
    int *index=mMalloc(N*sizeof(int));
    
    printf(    "in :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mAscSort(data,N);
    printf( "\nout :");for(int i=0;i<N;i++) {printf("%d,",data[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mAscSort(data,NULL,index,N);
    printf(" \nout :");for(int i=0;i<N;i++) {printf("%d(%d),",data[i],index[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mDescSort(data,N);
    printf( "\nout :");for(int i=0;i<N;i++) {printf("%d,",data[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mDescSort(data,NULL,index,N);
    printf( "\nout :");for(int i=0;i<N;i++) {printf("%d(%d),",data[i],index[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mMinSubset(data,N,M);
    printf( "\nout :");for(int i=0;i<M;i++) {printf("%d,",data[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mMinSubset(data,N,NULL,index,M);
    printf( "\nout :");for(int i=0;i<M;i++) {printf("%d(%d),",data[i],index[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mMaxSubset(data,N,M);
    printf( "\nout :");for(int i=0;i<M;i++) {printf("%d,",data[i]);}
    
    printf("\n\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mMaxSubset(data,N,NULL,index,M);
    printf( "\nout :");for(int i=0;i<M;i++) {printf("%d(%d),",data[i],index[i]);}
    
    mFree(data);
    mFree(index);
}




