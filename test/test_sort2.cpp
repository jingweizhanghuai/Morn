/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: g++ -Ofast -fopenmp test_sort2.cpp -o test_sort2.exe -lgsl -lgslcblas -lmorn

#include <algorithm>
#include <gsl/gsl_sort_double.h>
#include "morn_math.h"

int compare(const void *v1, const void *v2) {return ((*((double *)v1))>(*((double *)v2)))?1:-1;}
void test1()
{
    double *data1= (double *)mMalloc(10000000* sizeof(double));
    double *data2= (double *)mMalloc(10000000* sizeof(double));
    double *data3= (double *)mMalloc(10000000* sizeof(double));
    double *data4= (double *)mMalloc(10000000* sizeof(double));
    
    for(int n=1000;n<=10000000;n*=10)
    {
        printf("\n%d data sort for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1[i]=((double)mRand(-10000000,10000000))/((double)mRand(1,10000));
            data2[i]=data1[i];data3[i]=data1[i];data4[i]=data1[i];
        }
        
        mTimerBegin("qsort");
        for(int i=0;i<10000000;i+=n) qsort(data1+i,n,sizeof(double),compare);
        mTimerEnd("qsort");
        
        mTimerBegin("gsl");
        for(int i=0;i<10000000;i+=n) gsl_sort(data2+i,1,n);
        mTimerEnd("gsl");
        
        mTimerBegin("stl");
        for(int i=0;i<10000000;i+=n) std::sort(data3+i,data3+i+n);
        mTimerEnd("stl");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data4+i,n);
        mTimerEnd("Morn");
    }
    
    mFree(data1); mFree(data2); mFree(data3); mFree(data4);
}

void test2()
{
    double *data1 = (double *)mMalloc(10000000* sizeof(double));
    double *data2 = (double *)mMalloc(10000000* sizeof(double));
    size_t *index1= (size_t *)mMalloc(10000000* sizeof(size_t));
    int    *index2= (int    *)mMalloc(10000000* sizeof(int   ));

    for(int n=1000;n<=10000000;n*=10)
    {
        printf("\n%d data sort with index for %d times:\n",n,10000000/n);
        for(int i=0;i<10000000;i++)
        {
            data1[i]=((double)mRand(-10000000,10000000))/((double)mRand(1,10000));
            data2[i]=data1[i];
        }
        mTimerBegin("gsl");
        for(int i=0;i<10000000;i+=n) gsl_sort_index(index1,data1+i,1,n);
        mTimerEnd("gsl");
        
        mTimerBegin("Morn");
        for(int i=0;i<10000000;i+=n) mAscSort(data2+i,NULL,index2,n);
        mTimerEnd("Morn");
    }
    
    mFree(data1); mFree(data2);mFree(index1);mFree(index2);
}

void test3_1()
{
    double *data1= (double *)mMalloc(10000000*sizeof(double));
    double *data2= (double *)mMalloc(10000000*sizeof(double));
    for(int n=100000;n<=10000000;n*=10)
        for(int m=n/10;m<n;m+=n/5)
        {
            printf("\nselect %d from %d data for %d times\n",m,n,10000000/n);
            for(int i=0;i<10000000;i++)
            {
                data1[i]=((double)mRand(-1000000,1000000))/((double)mRand(1,1000));
                data2[i]=data1[i];
            }
            mTimerBegin("stl");
            for(int i=0;i<10000000;i+=n) std::nth_element(data1+i,data1+i+m-1,data1+i+n);
            mTimerEnd("stl");
            
            mTimerBegin("Morn");
            for(int i=0;i<10000000;i+=n) mMinSubset(data2+i,n,m);
            mTimerEnd("Morn");
        }
    mFree(data1);mFree(data2);
}

void test3_2()
{
    int n=1000000;int m;
    double *in  = (double *)mMalloc(n * sizeof(double));
    double *out1= (double *)mMalloc(n * sizeof(double));
    double *out2= (double *)mMalloc(n * sizeof(double));
    for (int i=0;i<n;i++) in[i] = ((double)mRand(-10000,10000))/10000.0;
    
    for(m=100000;m<n;m+=200000)
    {
        printf("\nselect %d from %d data\n",m,n);
        mTimerBegin("gsl" ); gsl_sort_smallest(out1,m,in,1,n); mTimerEnd("gsl" );
        mTimerBegin("Morn"); mMinSubset(in,n,out2,m);          mTimerEnd("Morn");
    }

    mFree(in); mFree(out1); mFree(out2);
}

void test4()
{
    int n=1000000;int m;
    double *in  = (double *)mMalloc(n * sizeof(double));
    size_t *out1= (size_t *)mMalloc(n * sizeof(size_t));
    int    *out2= (int    *)mMalloc(n * sizeof(int   ));
    for (int i=0;i<n;i++) in[i] = ((double)mRand(-10000,10000))/10000.0;
    
    for(m=100000;m<n;m+=200000)
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
    if(strcmp(argv[1],"test1")==0) test1();
    if(strcmp(argv[1],"test2")==0) test2();
    if(strcmp(argv[1],"test3")==0) {test3_1();test3_2();}
    if(strcmp(argv[1],"test4")==0) test4();
    return 0;
}

