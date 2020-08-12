// 编译：gcc -fopenmp -O2 test_fit_LSM.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_fit_LSM.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_image.h"

int main1()
{
    float x[21]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};
    float y[21]={830,1287,1975,2744,4515,5974,7711,9692,11791,14380,17205,20438,24324,28018,31161,34546,37198,40171,42638,44653,59804};
    float a[5];
    mLinearFitLSM(x,y,20,a);
    printf("linear fit result is y=(%f)*x+(%f)\n",a[0],a[1]);
    mParabolaFitLSM(x,y,20,a);
    printf("parabola fit result is y=(%f)*x^2+(%f)*x+(%f)\n",a[0],a[1],a[2]);
    mPolyFitLSM(x,y,20,a,4);
    printf("poly fit result is y=(%f)*x^4+(%f)*x^3+(%f)*x^2+(%f)*x+(%f)\n",a[0],a[1],a[2],a[3],a[4]);
    mExpFitLSM(x,y,20,a,4);
    printf("exp fit result is y=%f*exp(%f*x)\n",a[0],a[1]);
    mLnFitLSM(x,y,20,a,4);
    printf("ln fit result is y=%f*ln(x)+(%f)\n",a[0],a[1]);
    mPowerFitLSM(x,y,20,a,4);
    printf("power fit result is y=%f*x^(%f)\n",a[0],a[1]);

    mPolyFitLSM(x,y,21,a,4);
    printf("poly fit result is y=(%f)*x^4+(%f)*x^3+(%f)*x^2+(%f)*x+(%f)\n",a[0],a[1],a[2],a[3],a[4]);
    mPolyFit(x,y,21,a,4);
    printf("poly fit result is y=(%f)*x^4+(%f)*x^3+(%f)*x^2+(%f)*x+(%f)\n",a[0],a[1],a[2],a[3],a[4]);
 
    float x1[24]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
    float y1[24]={154,277,365,398,480,619,705,762,755,669,726,890,731,707,696,558,509,444,381,377,312,267,221,166};
    float a1[4];
    mPolyFitLSM(x1,y1,24,a1,3);
    printf("fit result is y=(%f)*x^3+(%f)*x^2+(%f)*x+(%f)\n",a1[0],a1[1],a1[2],a1[3]);
}



// void mFitLSM(float *xin,float *yin,int n,float (*func)(float,float *),float *a,int k);
// int main2()
// {
//     float x1[24]={1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
//     float y1[24]={154,277,365,398,480,619,705,762,755,669,726,890,731,707,696,558,509,444,381,377,312,267,221,166};
//     float a1[4];
//     float func(float x,float *a) {return a[0]*x*x*x+a[1]*x*x+a[2]*x+a[3];}
//     mFitLSM(x1,y1,24,func,a1,4);
//     printf("fit result is y=(%f*x^3)+(%f*x^2)+(%f*x)+(%f)\n",a1[0],a1[1],a1[2],a1[3]);
// }

int main()
{
    float x[1230];float y[1230];
    FILE *f = fopen("./test/test17.csv","r");

    MList *list = mListCreate(DFLT,NULL);
    MImageCurve curve;curve.type=1;
    char string[128];
    for(int i=0;i<1230;i++)
    {
        fgets(string,128,f);
        sscanf(string,"%f,%f,\n",y+i,x+i);
        // y[i]-=1150;

        float p[2];p[0]=y[i];p[1]=x[i];
        mListWrite(list,DFLT,p,2*sizeof(float));
        
        printf("%f,%f\n",x[i],y[i]);
    }
    
    float a[5];
    mPolyFitLSM(x,y,1230,a,4);
    printf("a is %f,%f,%f,%f\n",a[0],a[1],a[2],a[3],a[4]);

    mPolyLSMFit(list,&curve,4);
    printf("a is %f,%f,%f,%f\n",curve.para[1],curve.para[2],curve.para[3],curve.para[4],curve.para[5]);

    mListRelease(list);
}
    












