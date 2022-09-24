## Sort

Sort with Morn is fast and simple, Generic is supported to some extent.

### API

The source code of Morn sort is [morn_sort.c](../src/math/morn_sort.c), and the API is defined in [morn_math.h](../include/morn_math.h).

Tips:

"Generic" is supported，**`Type`can be `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `float` or `double`**.

#### Ascending Sort

```c
void mAscSort(Type *data_in,int num);
void mAscSort(Type *data_in,Type *data_out,int num);
void mAscSort(Type *data_in,Type *data_out,int *index_out,int num);
void mAscSort(Type *data_in,int *index_in,Type *data_out,int *index_out,int num);
```

`data_in` is the input data before sort, and `data_out` is the output after sort.

when`data_out` is equal as `data_in` (or `data_out` is not set, or `data_out`==NULL), the output data will overwrite the input.

`index_in` is the index of `data_in`, `index_out` is output index for `data_out`. 

If neither is set(or both are set NULL), it means index is not care.

If `index_in`==NULL and `index_out`!=NULL, the default ascending order(0, 1, 2, 3 etc.) is used.

If `index_out`==`index_in` or `index_out`==NULL, the output index will overwrite the input.

`num` is the number of input.

for example:

```c
int data[10];
printf("in :");
for(int i=0;i<10;i++) {data[i] = mRand(-100,100);printf("%d,",data[i]);}
mAscSort(data,NULL,index,10);
printf("\nout :");
for(int i=0;i<10;i++) {printf("%d(%d),",data[i],index[i]);}
```

Output is:

```
in :5,45,-19,-73,61,-9,95,42,-73,-64,
out :-73(8),-73(3),-64(9),-19(2),-9(5),5(0),42(7),45(1),61(4),95(6),
```



#### Descending Sort

```c
void mDescSort(Type *data_in,int num);
void mDescSort(Type *data_in,Type *data_out,int num);
void mDescSort(Type *data_in,Type *data_out,int *index_out,int num);
void mDescSort(Type *data_in,int *index_in,Type *data_out,int *index_out,int num);
```

It is same with `mAscSort`。

for example:

```c
int data[10];
printf("in :");
for(int i=0;i<10;i++) {data[i] = mRand(-100,100);printf("%d,",data[i]);}
mDescSort(data,10);
printf( "\nout :");
for(int i=0;i<10;i++) {printf("%d,",data[i]);}
```

Output is:

```
in :91,-96,2,53,-8,82,-79,16,18,-5,
out :91,82,53,18,16,2,-5,-8,-79,-96,
```



#### Minimum Subset

也就是从num_in个数据里，取出num_out个最小的数（注意：取出的数并非按照大小顺序排列）。

```c
Type mMinSubset(Type,Type *data_in,int num_in,int num_out);
Type mMinSubset(Type,Type *data_in,int num_in,Type *data_out,int num_out);
Type mMinSubset(Type,Type *data_in,int num_in,Type *data_out,int *index_out,int num_out);
Type mMinSubset(Type,Type *data_in,int *index_in,int num_in, Type *data_out,int *index_out,int num_out);
```

This is used to selected `num_out` smallest from all `num_in` data.

Note:

The data selected is not sorted in order.

`data_in`, `data_out`, `index_in`, `index_out`  is same with `mAscSort` and `mDescSort`

The return value is the threshold, the maximum one in all output data.

For example:

```c
int data[10];
printf("in :");
for(int i=0;i<10;i++) {data[i] = mRand(-100,100);printf("%d,",data[i]);}
int threshold = mMinSubset(data,10,NULL,index,4);
printf( "\nout :");
for(int i=0;i<4;i++) {printf("%d(%d),",data[i],index[i]);}
printf("threshold=%d\n",threshold);    
```

Output is:

```
in :47,-56,-38,57,-63,-41,23,41,29,78,
out :-41(5),-56(1),-38(2),-63(4),
threshold=-38
```



####  Maximum Subset 

```c
Type mMaxSubset(Type,Type *data_in,int num_in,int num_out);
Type mMaxSubset(Type,Type *data_in,int num_in,Type *data_out,int num_out);
Type mMaxSubset(Type,Type *data_in,int num_in,Type *data_out,int *index_out,int num_out);
Type mMaxSubset(Type,Type *data_in,int *index_in,int num_in, Type *data_out,int *index_out,int num_out);
```

it is same with `mMinSubset`.

The return value is the threshold, the Minimum one in all output data.

For example:

```c
int data[10];
printf("\nin :");
for(int i=0;i<10;i++) {data[i] = mRand(-100,100);printf("%d,",data[i]);}
int threshold=mMaxSubset(data,10,4);
printf( "\nout :");
for(int i=0;i<4;i++) {printf("%d,",data[i]);}
printf("threshold=%d\n",threshold); 
```

Output is:

```
in :16,-65,90,-58,-12,6,-60,42,-36,-52,
out :16,42,90,6,
threshold=6
```



#### Sort List Element

All above APIs is for types of number, and Morn provides `mListSort` for `MList`, which is a data containers for all types. See [MList](Morn：容器2) for details.



### Performance

Complete testing code is：[test_sort2.cpp](../test/test_sort2.cpp). Compile command is:

```shell
g++ -Ofast -DNDEBUG test_sort2.cpp -o test_sort2.exe -lgsl -lgslcblas -lmorn
```



#### Data Sort

Here, we compared Morn with other 3 libraries: `qsort`  in C standard library, `gsl_sort` in GSL(The GNU Scientific Library) and `std::sort` in C++ STL.

Testing Code is:

```c
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
```

In above program, we firstly generate some random double data, and then measure time-consuming of: 1. sorting 1000 data for 10000times, 2. sorting 10000 data for 1000times, 3.sorting 100000 data for 100 times, 4.sorting 1000000 data for 10 times and 5.sorting all 10000000 data for 1 time. Output is:

[![c0WVPA.png](https://z3.ax1x.com/2021/04/11/c0WVPA.png)](https://imgtu.com/i/c0WVPA)

It can be seen that: **1.`std::sort` and `mAscSort`  in Morn is the fastest**, 2.for small amount of data, `gsl_sort` is faster then `qsort`, but for the large amount of data `qsort` is faster.



#### Sort with Index

Here we compared `mAscSort` in Morn and `gsl_sort_index` in GSL. Testing code is:

```c
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
```

In above program, we firstly generate some random double data, and then measure time-consuming of: 1. sorting 1000 data for 10000times, 2. sorting 10000 data for 1000times, 3.sorting 100000 data for 100 times, 4.sorting 1000000 data for 10 times and 5.sorting all 10000000 data for 1 time. Output is:

[![c0fVwF.png](https://z3.ax1x.com/2021/04/11/c0fVwF.png)](https://imgtu.com/i/c0fVwF)

 Obviously: **Morn sort  is  faster than GSL **. And as the amount increases, the speed gap widens.  

Note:

`gsl_sort_index` and `mAscSort` are different with: `gsl_sort_index` Outputs only sorted index, without sorted data, But `mAscSort` Outputs sorted data and sorted index.



#### Select Minimum/Maximum Subset

Firstly, we compared `mMinSubset` in Morn and `std::nth_element` in C++ STL. Test code is:

```c
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
```

In above program, we generate some double data, and then test: 

1.selecting 10000\30000\50000\70000\90000 data from 100000 for 100 times, 

2.selecting 100000\300000\500000\700000\900000 data from 1000000 for 10 times, 

3.selecting 1000000\3000000\5000000\7000000\9000000 data from 10000000 for 1 times. 

The testing code is:

[![c0htBT.png](https://z3.ax1x.com/2021/04/11/c0htBT.png)](https://imgtu.com/i/c0htBT)

It shows that: **`mMinSubset`and `std::nth_element` perform at roughly the same level **.   

Note:

`mMinSubset` and `std::nth_element` have some difference. For top-N program, these 2 functions all output unsorted subset, but `std::nth_element` outputs the threshold in array position n, ``mMinSubset` outputs the threshold as return.



And then, we compared `mMinSubset` in Morn and `gsl_sort_smallest` in GSL. Testing code is:


```c
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
```

Here, we select 100000/300000/500000/700000/900000 data from 1000000. Output is:

[![c07YuR.png](https://z3.ax1x.com/2021/04/12/c07YuR.png)](https://imgtu.com/i/c07YuR)

It shows that: gap of time-consuming between Morn and GSL is huge.

Note:

`gsl_sort_smallest` and `mMinSubset` are different: the output of `gsl_sort_smallest` is sorted, which is similarity as `std::partial_sort`, and the output of `mMinSubset` is unsorted.



#### Select Minimum/Maximum Subset with Index
Here we compared `mMaxSubset` in Morn and `gsl_sort_largest_index` in GSL. Testing code is:
```c
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
```
Here, we select 100000/30000/500000/700000/900000 largest data from 1000000. Testing code is:

[![c07Gv9.png](https://z3.ax1x.com/2021/04/12/c07Gv9.png)](https://imgtu.com/i/c07Gv9)

Obviously: Morn is much faster then GSL.

Note:

`gsl_sort_largest_index` and `mMaxSubset` are also different: `gsl_sort_largest_index` output only index, and it is sorted,   `mMaxSubset` outputs the index and data, but it is unsorted.



