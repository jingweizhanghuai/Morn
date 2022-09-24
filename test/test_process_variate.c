#include "morn_ptc.h"

int main()
{
    char data[4096][128];
    for(int i=0;i<4096;i++)
    {
        char *pdata=&(data[i][0]);
        *((int *)pdata)=i;
        mRandString(pdata+4,124);
    }

    char name[512][16];
    int size[512];
    for(int i=0;i<512;i++)
    {
        mRandString(&(name[i][0]),4,16);
        // printf("\"%s\",",&(name[i][0]));
        size[i]=mRand(4,128);
        char *p=mProcVariate(&(name[i][0]),size[i]);
        memcpy(p,&(data[i][0]),size[i]);
    }

    for(int i=0;i<10000;i++)
    {
        int idx=mRand(0,512);
        int *p=mProcVariate(&(name[idx][0]),size[idx]);

        int error=0;
        if(p[0]>4096) error=1;

        char *pdata = &(data[p[0]][0]);
        if(memcmp(p,pdata,size[idx])!=0) error=1;
        if(error)
        {
            printf("i=%d,idx=%d,name=%s,size=%d\n",i,idx,&(name[idx][0]),size[idx]);
            printf("p    :%d: %s\n",    p[0],(char *)(p+1)    );
            printf("pdata:%d: %s\n",pdata[0],(char *)(pdata+1));
            return 1;
        }

        int n=mRand(0,4096);
        memcpy(p,&(data[n][0]),size[idx]);
    }
    return 0;
}
