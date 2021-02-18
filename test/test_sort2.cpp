/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// 编译：g++ -O2 -fopenmp test_sort2.cpp -o test_sort2.exe -IC:/ProgramFiles/CPackage/gsl/include -LC:/ProgramFiles/CPackage/gsl/lib_x64_mingw -lgsl -I ..\include\ -L ..\lib\x64_mingw\ -lmorn

#include <algorithm>
#include <gsl/gsl_sort_double.h>
#include "morn_math.h"

int compare(const void *v1, const void *v2) {return ((*((double *)v1))>(*((double *)v2)))?1:-1;}
void test1()
{
    double *data1= (double *)mMalloc(1000000* sizeof(double));
    double *data2= (double *)mMalloc(1000000* sizeof(double));
    double *data3= (double *)mMalloc(1000000* sizeof(double));
    double *data4= (double *)mMalloc(1000000* sizeof(double));
    
    for(int n=100000;n<=1000000;n*=2)
    {
        printf("\nn=%d:\n",n);
        for(int i=0;i<n;i++)
        {
            data1[i]=((double)mRand(-10000,10000))/10000.0;
            data2[i]=data1[i];data3[i]=data1[i];data4[i]=data1[i];
        }
        mTimerBegin("qsort"); qsort(data1,n,sizeof(int),compare);  mTimerEnd("qsort");
        mTimerBegin("gsl"  ); gsl_sort(data2,1,n);                 mTimerEnd("gsl"  );
        mTimerBegin("stl"  ); std::sort(data3,data3+n);            mTimerEnd("stl"  );
        mTimerBegin("Morn" ); mAscSort(data4,n);                   mTimerEnd("Morn" );
    }
    
    mFree(data1); mFree(data2); mFree(data3); mFree(data4);
}

void test2()
{
    int n=1000000;int m;
    double *in  = (double *)mMalloc(n * sizeof(double));
    double *out1= (double *)mMalloc(n * sizeof(double));
    double *out2= (double *)mMalloc(n * sizeof(double));
    for (int i=0;i<n;i++) in[i] = ((double)mRand(-10000,10000))/10000.0;
    
    for(m=100000;m<n;m+=200000)
    {
        printf("\nm=%d,n=%d:\n",m,n);
        mTimerBegin("gsl" ); gsl_sort_smallest(out1,m,in,1,n); mTimerEnd("gsl" );
        mTimerBegin("Morn"); mMinSubset(in,n,out2,m);          mTimerEnd("Morn");
    }

    mFree(in); mFree(out1); mFree(out2);
}

void test3()
{
    double *data1 = (double *)mMalloc(1000000* sizeof(double));
    double *data2 = (double *)mMalloc(1000000* sizeof(double));
    size_t *index1= (size_t *)mMalloc(1000000* sizeof(size_t));
    int    *index2= (int    *)mMalloc(1000000* sizeof(int   ));

    for(int n=100000;n<=1000000;n*=2)
    {
        printf("\nn=%d:\n",n);
        for(int i=0;i<n;i++)
        {
            data1[i]=((double)mRand(-10000,10000))/10000.0;data2[i]=data1[i];
        }
        mTimerBegin("gsl" ); gsl_sort_index(index1,data1,1,n);  mTimerEnd("gsl" );
        mTimerBegin("Morn"); mAscSort(data2,NULL,index2,n);     mTimerEnd("Morn");
    }
    
    mFree(data1); mFree(data2);mFree(index1);mFree(index2);
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
        printf("\nm=%d,n=%d:\n",m,n);
        mTimerBegin("gsl" ); gsl_sort_largest_index(out1,m,in,1,n); mTimerEnd("gsl" );
        mTimerBegin("Morn"); mMaxSubset(in,n,NULL,out2,m);          mTimerEnd("Morn");
    }

    mFree(in); mFree(out1); mFree(out2);
}

int main() 
{
    test1();
    test2();
    test3();
    test4();
    return 0;
}

