/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: g++ -Ofast -fopenmp test_sort2.cpp -o test_sort2.exe -lgsl -lgslcblas -lmorn

#include <algorithm>
#include <gsl/gsl_sort_double.h>
#include "morn_math.h"

#include "boost/sort/sort.hpp"

int compareD64(const void *v1, const void *v2) {return ((*((double *)v1))>(*((double *)v2)))?1:-1;}
int compareF32(const void *v1, const void *v2) {return ((*((float  *)v1))>(*((float  *)v2)))?1:-1;}
int compareS32(const void *v1, const void *v2) {return ((*((int    *)v1))>(*((int    *)v2)))?1:-1;}
void test0()
{
    double *data1= (double *)mMalloc(10000000* sizeof(double));
    double *data2= (double *)mMalloc(10000000* sizeof(double));
    double *data3= (double *)mMalloc(10000000* sizeof(double));
    double *data4= (double *)mMalloc(10000000* sizeof(double));
    double *data5= (double *)mMalloc(10000000* sizeof(double));
    
    for(int n=100;n<=10000000;n*=10)
    {
        printf("\n%d data sort for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1[i]=((double)mRand(-100000000,100000000))/((double)mRand(1,10000));
            data2[i]=data1[i];data3[i]=data1[i];data4[i]=data1[i];data5[i]=data1[i];
        }
        
        mTimerBegin("qsort");
        for(int i=0;i<10000000;i+=n) qsort(data1+i,n,sizeof(double),compareD64);
        mTimerEnd("qsort");
        
        mTimerBegin("gsl");
        for(int i=0;i<10000000;i+=n) gsl_sort(data2+i,1,n);
        mTimerEnd("gsl");
        
        mTimerBegin("std::sort");
        for(int i=0;i<10000000;i+=n) std::sort(data3+i,data3+i+n);
        mTimerEnd("std::sort");
        
        mTimerBegin("boost spreadsort");
        for(int i=0;i<10000000;i+=n) boost::sort::spreadsort::spreadsort(data4+i,data4+i+n);
        mTimerEnd("boost spreadsort");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data5+i,n);
        mTimerEnd("Morn");
    }
    
    mFree(data1); mFree(data2); mFree(data3); mFree(data4);mFree(data5);
}

void test1_1()
{
    void *data1 = mMalloc(10000000* 8);
    void *data2 = mMalloc(10000000* 8);
    void *data3 = mMalloc(10000000* 8);
    double *data1_d64=(double *)data1;float *data1_f32=(float *)data1;int *data1_s32=(int *)data1;
    double *data2_d64=(double *)data2;float *data2_f32=(float *)data2;int *data2_s32=(int *)data2;
    double *data3_d64=(double *)data3;float *data3_f32=(float *)data3;int *data3_s32=(int *)data3;
    
    for(int n=100;n<=10000000;n*=10)
    {
        printf("\n%d double data sort for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1_d64[i]=((double)mRand(-100000000,100000000))/((double)mRand(1,10000));
            data2_d64[i]=data1_d64[i];data3_d64[i]=data1_d64[i];
        }
        
        mTimerBegin("qsort");
        for(int i=0;i<10000000;i+=n) qsort(data1_d64+i,n,sizeof(double),compareD64);
        mTimerEnd("qsort");
        
        mTimerBegin("std::sort");
        for(int i=0;i<10000000;i+=n) std::sort(data2_d64+i,data2_d64+i+n);
        mTimerEnd("std::sort");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data3_d64+i,n);
        mTimerEnd("Morn");
    }

    for(int n=100;n<=10000000;n*=10)
    {
        printf("\n%d float data sort for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1_f32[i]=((float)mRand(-100000000,100000000))/((float)mRand(1,10000));
            data2_f32[i]=data1_f32[i];data3_f32[i]=data1_f32[i];
        }
        
        mTimerBegin("qsort");
        for(int i=0;i<10000000;i+=n) qsort(data1_f32+i,n,sizeof(float),compareF32);
        mTimerEnd("qsort");
        
        mTimerBegin("std::sort");
        for(int i=0;i<10000000;i+=n) std::sort(data2_f32+i,data2_f32+i+n);
        mTimerEnd("std::sort");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data3_f32+i,n);
        mTimerEnd("Morn");
    }

    for(int n=100;n<=10000000;n*=10)
    {
        printf("\n%d int data sort for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1_s32[i]=mRand(-100000000,100000000);
            data2_s32[i]=data1_s32[i];data3_s32[i]=data1_s32[i];
        }
        
        mTimerBegin("qsort");
        for(int i=0;i<10000000;i+=n) qsort(data1_s32+i,n,sizeof(int),compareS32);
        mTimerEnd("qsort");
        
        mTimerBegin("std::sort");
        for(int i=0;i<10000000;i+=n) std::sort(data2_s32+i,data2_s32+i+n);
        mTimerEnd("std::sort");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data3_s32+i,n);
        mTimerEnd("Morn");

        // for(int i=0;i<10000000;i++) {if(data1_s32[i]!=data2_s32[i]) printf("%d:%d\n",data1_s32[i],data2_s32[i]);}
    }
    
    mFree(data1);mFree(data2);mFree(data3);
}

void test1_2()
{
    void *data1 = mMalloc(10000000*8);
    void *data2 = mMalloc(10000000*8);
    void *data3 = mMalloc(10000000*8);
    double *data1_d64=(double *)data1;float *data1_f32=(float *)data1;int *data1_s32=(int *)data1;
    double *data2_d64=(double *)data2;float *data2_f32=(float *)data2;int *data2_s32=(int *)data2;
    double *data3_d64=(double *)data3;float *data3_f32=(float *)data3;int *data3_s32=(int *)data3;

    for(int n=100;n<=10000000;n*=10)
    {
        printf("\n%d nearly ordered double data sort for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1_d64[i]=((double)(i+mRand(-9,10)))/13579.0;
            data2_d64[i]=data1_d64[i];data3_d64[i]=data1_d64[i];
        }
        
        mTimerBegin("qsort");
        for(int i=0;i<10000000;i+=n) qsort(data1_d64+i,n,sizeof(double),compareD64);
        mTimerEnd("qsort");
        
        mTimerBegin("std::sort");
        for(int i=0;i<10000000;i+=n) std::sort(data2_d64+i,data2_d64+i+n);
        mTimerEnd("std::sort");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data3_d64+i,n);
        mTimerEnd("Morn");
    }
    
    for(int n=100;n<=10000000;n*=10)
    {
        printf("\n%d nearly ordered float data sort for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1_f32[i]=((float)(i+mRand(-9,10)))/13579.0;
            data2_f32[i]=data1_f32[i];data3_f32[i]=data1_f32[i];
        }
        
        mTimerBegin("qsort");
        for(int i=0;i<10000000;i+=n) qsort(data1_f32+i,n,sizeof(float),compareF32);
        mTimerEnd("qsort");
        
        mTimerBegin("std::sort");
        for(int i=0;i<10000000;i+=n) std::sort(data2_f32+i,data2_f32+i+n);
        mTimerEnd("std::sort");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data3_f32+i,n);
        mTimerEnd("Morn");
    }
    
    for(int n=100;n<=10000000;n*=10)
    {
        printf("\n%d nearly ordered int data sort for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1_s32[i]=i+mRand(-9,10);
            data2_s32[i]=data1_s32[i];data3_s32[i]=data1_s32[i];
        }
        
        mTimerBegin("qsort");
        for(int i=0;i<10000000;i+=n) qsort(data1_s32+i,n,sizeof(int),compareS32);
        mTimerEnd("qsort");
        
        mTimerBegin("std::sort");
        for(int i=0;i<10000000;i+=n) std::sort(data2_s32+i,data2_s32+i+n);
        mTimerEnd("std::sort");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data3_s32+i,n);
        mTimerEnd("Morn");

//         for(int i=0;i<10000000;i++) {if(data1_s32[i]!=data2_s32[i]) printf("%d:%d\n",data1_s32[i],data2_s32[i]);}
    }
    
    mFree(data1);mFree(data2);mFree(data3);
}

double *g_data_D64;
float *g_data_F32;
int *g_data_S32;
int compare_qsort_D64(const void *i1, const void *i2) {return (g_data_D64[*((int *)i1)]>g_data_D64[*((int *)i2)])?1:-1;}
int compare_qsort_F32(const void *i1, const void *i2) {return (g_data_F32[*((int *)i1)]>g_data_F32[*((int *)i2)])?1:-1;}
int compare_qsort_S32(const void *i1, const void *i2) {return (g_data_S32[*((int *)i1)]>g_data_S32[*((int *)i2)])?1:-1;}
int compare_stdsort_D64(int i1,int i2) {return g_data_D64[i1]<g_data_D64[i2];}
int compare_stdsort_F32(int i1,int i2) {return g_data_F32[i1]<g_data_F32[i2];}
int compare_stdsort_S32(int i1,int i2) {return g_data_S32[i1]<g_data_S32[i2];}
void test2()
{
    void *data1 = mMalloc(10000000*8);double *data1_d64=(double *)data1;float *data1_f32=(float *)data1;int *data1_s32=(int *)data1;
    void *data2 = mMalloc(10000000*8);double *data2_d64=(double *)data2;float *data2_f32=(float *)data2;int *data2_s32=(int *)data2;
    void *data3 = mMalloc(10000000*8);double *data3_d64=(double *)data3;float *data3_f32=(float *)data3;int *data3_s32=(int *)data3;
    int *index1= (int *)mMalloc(10000000*sizeof(int));
    int *index2= (int *)mMalloc(10000000*sizeof(int));
    int *index3= (int *)mMalloc(10000000*sizeof(int));

    for(int n=100;n<=10000000;n*=10)
    {
        printf("\n%d double data sort with index for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1_d64[i]=((double)mRand(-10000000,10000000))/((double)mRand(1,10000));
            data2_d64[i]=data1_d64[i];data3_d64[i]=data1_d64[i];
            index1[i]=i;index2[i]=i;index3[i]=i;
        }
        
        mTimerBegin("qsort");
        g_data_D64=data1_d64;
        for(int i=0;i<10000000;i+=n) qsort(index1+i,n,sizeof(int),compare_qsort_D64);
        mTimerEnd("qsort");

        mTimerBegin("std::sort");
        g_data_D64=data2_d64;
        for(int i=0;i<10000000;i+=n) std::sort(index2+i,index2+i+n,compare_stdsort_D64);
        mTimerEnd("std::sort");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data3_d64+i,index3+i,NULL,index3+i,n);
        mTimerEnd("Morn");
    }
    
    for(int n=100;n<=10000000;n*=10)
    {
        printf("\n%d float data sort with index for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1_f32[i]=((float)mRand(-10000000,10000000))/((float)mRand(1,10000));
            data2_f32[i]=data1_f32[i];data3_f32[i]=data1_f32[i];
            index1[i]=i;index2[i]=i;index3[i]=i;
        }
        
        mTimerBegin("qsort");
        g_data_F32=data1_f32;
        for(int i=0;i<10000000;i+=n) qsort(index1+i,n,sizeof(int),compare_qsort_F32);
        mTimerEnd("qsort");

        mTimerBegin("std::sort");
        g_data_F32=data2_f32;
        for(int i=0;i<10000000;i+=n) std::sort(index2+i,index2+i+n,compare_stdsort_F32);
        mTimerEnd("std::sort");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data3_f32+i,index3+i,NULL,index3+i,n);
        mTimerEnd("Morn");
    }
    
    for(int n=100;n<=10000000;n*=10)
    {
        printf("\n%d int data sort with index for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1_s32[i]=mRand(-10000000,10000000);
            data2_s32[i]=data1_s32[i];data3_s32[i]=data1_s32[i];
            index1[i]=i;index2[i]=i;index3[i]=i;
        }
        
        mTimerBegin("qsort");
        g_data_S32=data1_s32;
        for(int i=0;i<10000000;i+=n) qsort(index1+i,n,sizeof(int),compare_qsort_S32);
        mTimerEnd("qsort");

        mTimerBegin("std::sort");
        g_data_S32=data2_s32;
        for(int i=0;i<10000000;i+=n) std::sort(index2+i,index2+i+n,compare_stdsort_S32);
        mTimerEnd("std::sort");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data3_s32+i,index3+i,NULL,index3+i,n);
        mTimerEnd("Morn");
    }
    
    mFree(data1);mFree(data2);mFree(data3);mFree(index1);mFree(index2);mFree(index3);
}

void test3()
{
    void *data1 = mMalloc(10000000*8);double *data1_d64=(double *)data1;float *data1_f32=(float *)data1;int *data1_s32=(int *)data1;
    void *data2 = mMalloc(10000000*8);double *data2_d64=(double *)data2;float *data2_f32=(float *)data2;int *data2_s32=(int *)data2;
    
    for(int n=100;n<=10000000;n*=10)
        for(int m=n/10;m<n;m+=n/5)
        {
            printf("\nselect %d from %d double data for %d times\n",m,n,10000000/n);
            for(int i=0;i<10000000;i++)
            {
                data1_d64[i]=((double)mRand(-1000000,1000000))/((double)mRand(1,1000));
                data2_d64[i]=data1_d64[i];
            }
            mTimerBegin("std::nth_element");
            for(int i=0;i<10000000;i+=n) std::nth_element(data1_d64+i,data1_d64+i+m-1,data1_d64+i+n);
            mTimerEnd("std::nth_element");
            
            mTimerBegin("Morn");
            for(int i=0;i<10000000;i+=n) mMinSubset(data2_d64+i,n,m);
            mTimerEnd("Morn");
        }
        
    for(int n=100;n<=10000000;n*=10)
        for(int m=n/10;m<n;m+=n/5)
        {
            printf("\nselect %d from %d float data for %d times\n",m,n,10000000/n);
            for(int i=0;i<10000000;i++)
            {
                data1_f32[i]=((float)mRand(-1000000,1000000))/((float)mRand(1,1000));
                data2_f32[i]=data1_f32[i];
            }
            mTimerBegin("std::nth_element");
            for(int i=0;i<10000000;i+=n) std::nth_element(data1_f32+i,data1_f32+i+m-1,data1_f32+i+n);
            mTimerEnd("std::nth_element");
            
            mTimerBegin("Morn");
            for(int i=0;i<10000000;i+=n) mMinSubset(data2_f32+i,n,m);
            mTimerEnd("Morn");
        }
        
    for(int n=100;n<=10000000;n*=10)
        for(int m=n/10;m<n;m+=n/5)
        {
            printf("\nselect %d from %d int data for %d times\n",m,n,10000000/n);
            for(int i=0;i<10000000;i++)
            {
                data1_s32[i]=mRand(-10000000,10000000);
                data2_s32[i]=data1_s32[i];
            }
            mTimerBegin("std::nth_element");
            for(int i=0;i<10000000;i+=n) std::nth_element(data1_s32+i,data1_s32+i+m-1,data1_s32+i+n);
            mTimerEnd("std::nth_element");
            
            mTimerBegin("Morn");
            for(int i=0;i<10000000;i+=n) mMinSubset(data2_s32+i,n,m);
            mTimerEnd("Morn");
        }
    mFree(data1);mFree(data2);
}

void test3_2()
{
    int n=100000;int m;
    double *in  = (double *)mMalloc(n * sizeof(double));
    double *out1= (double *)mMalloc(n * sizeof(double));
    double *out2= (double *)mMalloc(n * sizeof(double));
    for (int i=0;i<n;i++) in[i] = ((double)mRand(-10000,10000))/10000.0;
    
    for(m=10000;m<n;m+=20000)
    {
        printf("\nselect %d from %d data\n",m,n);
        mTimerBegin("gsl" ); gsl_sort_smallest(out1,m,in,1,n); mTimerEnd("gsl" );
        mTimerBegin("Morn"); mMinSubset(in,n,out2,m);          mTimerEnd("Morn");
    }

    mFree(in); mFree(out1); mFree(out2);
}

void test4()
{
    void *data1 = mMalloc(10000000*8);double *data1_d64=(double *)data1;float *data1_f32=(float *)data1;int *data1_s32=(int *)data1;
    void *data2 = mMalloc(10000000*8);double *data2_d64=(double *)data2;float *data2_f32=(float *)data2;int *data2_s32=(int *)data2;
    int *index1 = (int *)mMalloc(10000000*sizeof(int));
    int *index2 = (int *)mMalloc(10000000*sizeof(int));
    
    for(int n=100;n<=10000000;n*=10)
        for(int m=n/10;m<n;m+=n/5)
        {
            printf("\nselect %d from %d double data with index for %d times\n",m,n,10000000/n);
            for(int i=0;i<10000000;i++)
            {
                data1_d64[i]=((double)mRand(-1000000,1000000))/((double)mRand(1,1000));
                data2_d64[i]=data1_d64[i];
                index1[i]=i;index2[i]=i;
            }
            
            int i=0;
            mTimerBegin("std::nth_element");
            g_data_D64=data1_d64;
            for(int i=0;i<10000000;i+=n) std::nth_element(index1+i,index1+i+m-1,index1+i+n,compare_stdsort_D64);
            mTimerEnd("std::nth_element");
            
            mTimerBegin("Morn");
            for(int i=0;i<10000000;i+=n) mMinSubset(data2_d64+i,index2+i,n,data2_d64+i,index2+i,m);
            mTimerEnd("Morn");
        }
        
    for(int n=100;n<=10000000;n*=10)
        for(int m=n/10;m<n;m+=n/5)
        {
            printf("\nselect %d from %d float data with index for %d times\n",m,n,10000000/n);
            for(int i=0;i<10000000;i++)
            {
                data1_f32[i]=((float)mRand(-1000000,1000000))/((float)mRand(1,1000));
                data2_f32[i]=data1_f32[i];
                index1[i]=i;index2[i]=i;
            }
            
            int i=0;
            mTimerBegin("std::nth_element");
            g_data_F32=data1_f32;
            for(int i=0;i<10000000;i+=n) std::nth_element(index1+i,index1+i+m-1,index1+i+n,compare_stdsort_F32);
            mTimerEnd("std::nth_element");
            
            mTimerBegin("Morn");
            for(int i=0;i<10000000;i+=n) mMinSubset(data2_f32+i,index2+i,n,data2_f32+i,index2+i,m);
            mTimerEnd("Morn");
        }
        
    for(int n=100;n<=10000000;n*=10)
        for(int m=n/10;m<n;m+=n/5)
        {
            printf("\nselect %d from %d int data with index for %d times\n",m,n,10000000/n);
            for(int i=0;i<10000000;i++)
            {
                data1_s32[i]=mRand(-10000000,10000000);
                data2_s32[i]=data1_s32[i];
                index1[i]=i;index2[i]=i;
            }
            
            int i=0;
            mTimerBegin("std::nth_element");
            g_data_S32=data1_s32;
            for(int i=0;i<10000000;i+=n) std::nth_element(index1+i,index1+i+m-1,index1+i+n,compare_stdsort_S32);
            mTimerEnd("std::nth_element");
            
            mTimerBegin("Morn");
            for(int i=0;i<10000000;i+=n) mMinSubset(data2_s32+i,index2+i,n,data2_s32+i,index2+i,m);
            mTimerEnd("Morn");
        }
        
    mFree(data1);mFree(data2);mFree(index1);mFree(index2);
}

void test4_2()
{
    int n=100000;int m;
    double *in  = (double *)mMalloc(n * sizeof(double));
    size_t *out1= (size_t *)mMalloc(n * sizeof(size_t));
    int    *out2= (int    *)mMalloc(n * sizeof(int   ));
    for (int i=0;i<n;i++) in[i] = ((double)mRand(-10000,10000))/10000.0;
    
    for(m=10000;m<n;m+=20000)
    {
        printf("\nselect %d from %d data with index\n",m,n);
        mTimerBegin("gsl" ); gsl_sort_largest_index(out1,m,in,1,n); mTimerEnd("gsl" );
        mTimerBegin("Morn"); mMaxSubset(in,n,NULL,out2,m);          mTimerEnd("Morn");
    }

    mFree(in); mFree(out1); mFree(out2);
}

int main(int argc,char *argv[])
{
    if(argc!=2) {printf("try as: \"test_sort2.exe test1\" or \"test_sort2.exe test2\"\n"); return 0;}
         if(strcmp(argv[1],"test1")==0) {test1_1();test1_2();}
    else if(strcmp(argv[1],"test2")==0) test2();
    else if(strcmp(argv[1],"test3")==0) test3();
    else if(strcmp(argv[1],"test4")==0) test4();
    else                                test0();
    return 0;
}

