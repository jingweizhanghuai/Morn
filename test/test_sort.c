#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_util.h"

void mAscSortD64(D64 *data_in,int *index_in,D64 *data_out,int *index_out,int num);
void mAscSortF32(F32 *data_in,int *index_in,F32 *data_out,int *index_out,int num);
void mAscSortS32(S32 *data_in,int *index_in,S32 *data_out,int *index_out,int num);

int test_ascsort(int n)
{
    int *data=mMalloc(n*sizeof(int));
    
    printf(   "in :");for(int i=0;i<n;i++) {data[i] = mRand(DFLT,DFLT);printf("%d,",data[i]);}
    mAscSortS32(data,NULL,NULL,NULL,n);
    printf("\nout :");for(int i=0;i<n;i++) {printf("%d,",data[i]);}
    
    int *index=mMalloc(n*sizeof(int));
    
    mFree(data);
}