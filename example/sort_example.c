#include "morn_math.h"

#define N 16
#define M 4

int main()
{
    int *data=mMalloc(N*sizeof(int));
    int *index=mMalloc(N*sizeof(int));
    int threshold;
    
    printf("asc sort:");
    printf("\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mAscSort(data,N);
    printf("\nout:");for(int i=0;i<N;i++) {printf("%d,",data[i]);}
    
    printf("\n\nasc sort with index");
    printf("\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d(%d),",data[i],i);}
    mAscSort(data,NULL,index,N);
    printf("\nout:");for(int i=0;i<N;i++) {printf("%d(%d),",data[i],index[i]);}
    
    printf("\n\ndesc sort:");
    printf("\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    mDescSort(data,N);
    printf("\nout:");for(int i=0;i<N;i++) {printf("%d,",data[i]);}
    
    printf("\n\ndesc sort with index:");
    printf("\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d(%d),",data[i],i);}
    mDescSort(data,NULL,index,N);
    printf("\nout:");for(int i=0;i<N;i++) {printf("%d(%d),",data[i],index[i]);}
    
    printf("\n\nmin %d subset from %d data:",M,N);
    printf("\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    threshold=mMinSubset(data,N,M);
    printf("\nout:");for(int i=0;i<M;i++) {printf("%d,",data[i]);}
    printf("\nthreshold=%d",threshold);

    printf("\n\nmin %d subset with index from %d data:",M,N);
    printf("\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d(%d),",data[i],i);}
    threshold=mMinSubset(data,N,NULL,index,M);
    printf("\nout:");for(int i=0;i<M;i++) {printf("%d(%d),",data[i],index[i]);}
    printf("\nthreshold=%d",threshold);
    
    printf("\n\nmax %d subset from %d data:",M,N);
    printf("\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d,",data[i]);}
    threshold=mMaxSubset(data,N,M);
    printf("\nout:");for(int i=0;i<M;i++) {printf("%d,",data[i]);}
    printf("\nthreshold=%d",threshold);
    
    printf("\n\nmax %d subset withindex from %d data:",M,N);
    printf("\nin :");for(int i=0;i<N;i++) {data[i] = mRand(-1000,1000);printf("%d(%d),",data[i],i);}
    threshold=mMaxSubset(data,N,NULL,index,M);
    printf("\nout:");for(int i=0;i<M;i++) {printf("%d(%d),",data[i],index[i]);}
    printf("\nthreshold=%d\n",threshold);
    
    mFree(data);
    mFree(index);
}




