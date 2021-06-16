#include "morn_util.h"

void func0(int *data,int s,void *para)
{
    printf("data is %d,run func0\n",*data);
}

void func1(int *data,int s,void *para)
{
    printf("data is %d,run func1\n",*data);
}

void func2(int *data,int s,void *para)
{
    printf("data is %d,run func2\n",*data);
}

void func3(int *data,int s,void *para)
{
    printf("data is %d,run func3\n",*data);
}

int main()
{
    MMap *map = mMapCreate();
    int n;
    n=0;mSignalMapFunction(map,&n,sizeof(int),func0,NULL,DFLT);
    n=1;mSignalMapFunction(map,&n,sizeof(int),func1,NULL,DFLT);
    n=2;mSignalMapFunction(map,&n,sizeof(int),func2,NULL,DFLT);
    n=3;mSignalMapFunction(map,&n,sizeof(int),func3,NULL,DFLT);
    
    int i;
    for(i=0;i<10;i++)
    {
        n=mRand(0,4);
        mSignalMap(map,&n,sizeof(int),&i,sizeof(int));
    }
    n=4;mSignalMap(map,&n,sizeof(int),&i,sizeof(int));
    
    mMapRelease(map);
}