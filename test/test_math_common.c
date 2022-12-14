
#include "morn_math.h"

int main1()
{
    #define NUM 10000
    float value[NUM];
    int count[20]={0};
    for(int i=0;i<NUM;i++)
    {
        value[i] = mNormalRand(100.0,2.0);
        
        int idx=(int)(value[i]-90.0);
        if((idx<0)||(idx>=20)) continue;
        count[idx]++;
    }
    
    for(int i=0;i<20;i++) 
        printf("count from %3d to %3d is %d\n",i+90,i+91,count[i]);
    
    float mean,variance;
    mVariance(value,NUM,&mean,&variance);
    printf("\nmean=%f,delta=%f\n",mean,mSqrt(variance));
}

// int mDecimalDigits(uint64_t data);
// uint64_t mDecimalFloor(uint64_t data);
// int main()
// {
//     mTimerBegin();
//     for(int i=0;i<1000000000;i++)
//     {
//         int digits=mDecimalDigits(i);
//     }
//     mTimerEnd();
//     return 0;
// }
