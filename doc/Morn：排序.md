## Morn：排序

### 接口

此文档相关函数都定义在[morn_sort.c](../src/math/morn_sort.c)里，接口在[morn_math.h](../include/morn_math.h)里。

这里值得一提的是：此处接口都是**支持“泛型”的C语言接口**，这里的**Type支持`int8_t`、`uint8_t`、`int16_t`、`uint16_t`、`int32_t`、`uint32_t`、`int64_t`、`uint64_t`、`float`、`double`十种数据类型**。

#### 升序排序

```c
void mAscSort(Type *data_in,int num);
void mAscSort(Type *data_in,Type *data_out,int num);
void mAscSort(Type *data_in,Type *data_out,int *index_out,int num);
void mAscSort(Type *data_in,int *index_in,Type *data_out,int *index_out,int num);
```

data_in是数据的输入（待排序数据），data_out是数据的输出（排序后数据）。

data_out可以等于data_in，也就是输出覆盖输入。data_out若不设置或设为NULL，则取其默认值data_in，也即默认覆盖输入。

index_in是输入数据的顺序，index_out是输出数据的顺序，如果两者都不设置或取NULL，表示排序时不记录顺序。

index_in若取NULL，则表示使用默认的递增顺序，即0，1，2，3，……，num-1的顺序。

index_out可以等于index_in，也就是输出覆盖原输入顺序。

index_out若取NULL，则取默认值index_in，也即覆盖原输入顺序。

num是数据的个数。

例如：

```c
printf("in :");
for(int i=0;i<10;i++) {data[i] = mRand(-100,100);printf("%d,",data[i]);}
mAscSort(data,NULL,index,10);
printf("\nout :");
for(int i=0;i<10;i++) {printf("%d(%d),",data[i],index[i]);}
```

其运行结果为

```
in :5,45,-19,-73,61,-9,95,42,-73,-64,
out :-73(8),-73(3),-64(9),-19(2),-9(5),5(0),42(7),45(1),61(4),95(6),
```



#### 降序排序

```c
void mDescSort(Type *data_in,int num);
void mDescSort(Type *data_in,Type *data_out,int num);
void mDescSort(Type *data_in,Type *data_out,int *index_out,int num);
void mDescSort(Type *data_in,int *index_in,Type *data_out,int *index_out,int num);
```

其参数与`mAscSort`相同，不再赘述。

例如：

```c
printf("in :");
for(int i=0;i<10;i++) {data[i] = mRand(-100,100);printf("%d,",data[i]);}
mDescSort(data,10);
printf( "\nout :");
for(int i=0;i<10;i++) {printf("%d,",data[i]);}
```

其运行结果为

```
in :91,-96,2,53,-8,82,-79,16,18,-5,
out :91,82,53,18,16,2,-5,-8,-79,-96,
```



#### 最小值子集

也就是从num_in个数据里，取出num_out个最小的数（注意：取出的数并非按照大小顺序排列）。

```c
Type mMinSubset(Type,Type *data_in,int num_in,int num_out);
Type mMinSubset(Type,Type *data_in,int num_in,Type *data_out,int num_out);
Type mMinSubset(Type,Type *data_in,int num_in,Type *data_out,int *index_out,int num_out);
Type mMinSubset(Type,Type *data_in,int *index_in,int num_in, Type *data_out,int *index_out,int num_out);
```

其参数Type、data_in、data_out、index_in、index_out与`mAscSort`相同，不再赘述。

num_in即输入数据的个数。num_out即输出数据的个数。

返回值是临界值，即输出数据中的最大值。

例如：

```c
printf("in :");
for(int i=0;i<10;i++) {data[i] = mRand(-100,100);printf("%d,",data[i]);}
int threshold = mMinSubset(data,10,NULL,index,4);
printf( "\nout :");
for(int i=0;i<4;i++) {printf("%d(%d),",data[i],index[i]);}
printf("threshold=%d\n",threshold);    
```

其运行结果为

```
in :47,-56,-38,57,-63,-41,23,41,29,78,
out :-41(5),-56(1),-38(2),-63(4),
threshold=-38
```



#### 最大值子集

```c
Type mMaxSubset(Type,Type *data_in,int num_in,int num_out);
Type mMaxSubset(Type,Type *data_in,int num_in,Type *data_out,int num_out);
Type mMaxSubset(Type,Type *data_in,int num_in,Type *data_out,int *index_out,int num_out);
Type mMaxSubset(Type,Type *data_in,int *index_in,int num_in, Type *data_out,int *index_out,int num_out);
```

其参数与`mMinSubset`相同，不再赘述。

返回值是临界值，即输出数据中的最小值。

例如：

```c
printf("\nin :");
for(int i=0;i<10;i++) {data[i] = mRand(-100,100);printf("%d,",data[i]);}
int threshold=mMaxSubset(data,10,4);
printf( "\nout :");
for(int i=0;i<4;i++) {printf("%d,",data[i]);}
printf("threshold=%d\n",threshold); 
```

其运行结果为

```
in :16,-65,90,-58,-12,6,-60,42,-36,-52,
out :16,42,90,6,
threshold=6
```



#### 容器排序

以上的排序都只针对“数”来进行排序，Morn的容器MList，还提供了更通用的排序`mListSort`，具体可参见文档[Morn:容器2](Morn：容器2)。



### 性能

性能测试程序源码为：[test_sort2.cpp](../test/test_sort2.cpp)。编译使用以下命令：

```shell
g++ -O2 -fopenmp -DNDEBUG test_sort2.cpp -o test_sort2.exe -lgsl -lgslcblas -lmorn
```



#### 排序性能

这里，把Morn的排序与另外三种排序方法做了对比，它们分别是：①C语言标准库函数里的`qsort`函数，②科学计算库GSL里的`gsl_sort`函数，③C++ STL里的`std::sort`函数。测试程序如下：

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

以上对随机生成的双精度浮点：①1000个数据排序，计时10000次；②10000个数据排序，计时1000次；③100000个数据排序，计时100次；④1000000个数据排序，计时10次；⑤10000000个数据排序，计时1次。其运行结果如下：

[![c0WVPA.png](https://z3.ax1x.com/2021/04/11/c0WVPA.png)](https://imgtu.com/i/c0WVPA)

可以看到：**`std::sort`和Morn的`mAscSort`速度最快（`std::sort`比`mAscSort`稍快，差距约2%）**，在排序数据量小时`gsl_sort`快于`qsort`，在数据量大时`qsort`快于`gsl_sort`。



#### 带索引排序的性能

这里比较的是Morn的`mAscSort`和GSL的`gsl_sort_index`函数。测试程序如下：

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

以上对随机生成的双精度浮点：①1000个数据排序，计时10000次；②10000个数据排序，计时1000次；③100000个数据排序，计时100次；④1000000个数据排序，计时10次；⑤10000000个数据排序，计时1次。其运行结果如下：

[![c0fVwF.png](https://z3.ax1x.com/2021/04/11/c0fVwF.png)](https://imgtu.com/i/c0fVwF)

显然：**相比GSL，Morn的排序更快**。且随着数据量的增加，差距拉大。
这里`gsl_sort_index`与`mAscSort`函数功能有区别。`gsl_sort_index`只输出排序后的索引，对数据不会排序，`mAscSort`对数据和索引都会排序。



#### 最大最小值子集的性能

首先比较Morn的`mMinSubset`函数和stl的`std::nth_element`函数。测试程序如下：

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

以上对随机生成的双精度浮点：①从100000数据中分别择出10000、30000、50000、70000、90000个最小子集，计时100次；②从1000000数据中分别择出100000、300000、500000、700000、900000个最小子集，计时10次；③从10000000数据中分别择出1000000、3000000、5000000、7000000、9000000个最小子集，计时1次；

测试结果如下：

[![c0htBT.png](https://z3.ax1x.com/2021/04/11/c0htBT.png)](https://imgtu.com/i/c0htBT)

可见，**`mMinSubset`与`std::nth_element`性能基本处于同一水平**。

这里`mMinSubset`与`std::nth_element`函数功能有细微差别，在解决top n问题时，两者输出的最小子集都是无序的，但是`std::nth_element`会将临界值写入数组的位置n，而`mMinSubset`不会将临界值写入位置n，而是以返回值的形式输出。



其次比较Morn的`mMinSubset`函数和GSL库的`gsl_sort_smallest`函数。测试程序如下：


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

这里，从1000000个随机生成的double数据中，分别取出100000、300000、500000、700000、900000个数据，

[![c07YuR.png](https://z3.ax1x.com/2021/04/12/c07YuR.png)](https://imgtu.com/i/c07YuR)

可以看到，两者的耗时差距巨大。
不过这里需要说明的是：`gsl_sort_smallest`和`mMinSubset`函数的功能不完全相同，它们虽然都是从m个数据中取出最小的n个，但是`gsl_sort_smallest`取出的数据是按照顺序排列好的，其取值的阈值就是`out1[n]`；`mMinSubset`取出的数据是无序的，其取值的阈值是函数的返回值。（`gsl_sort_smallest`的功能更类似`std::partial_sort`）。



#### 带索引的最大最小值子集的性能
这里比较了Morn的`mMaxSubset`函数和GSL的`gsl_sort_largest_index`函数，测试程序如下：
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
这里，从1000000个随机生成的double数据中，分别取出100000、300000、500000、700000、900000个数据，测试结果如下：

[![c07Gv9.png](https://z3.ax1x.com/2021/04/12/c07Gv9.png)](https://imgtu.com/i/c07Gv9)

显然Morn的排序快的多。
这里`gsl_sort_largest_index`与`mMaxSubset`也是有区别的。`gsl_sort_largest_index`只输出排序后的索引，且索引对应的数据是有序的。`mAscSort`会输出索引和数据，且数据是无序的。

总结：

|项目|Morn|std::sort|GSL|qsort|
|-|-|-|-|-|
|速度|快|快|慢|较慢|
|数据类型|任意数值类型，其他类型使用`mListSort`|任意类型|double|任意类型（使用void*)|
|索引|有|无（但可借助回调函数实现）|有|无（但可借助回调函数实现）|
|支持回调|`mListSort`支持|支持|不支持|支持|
|降序排序|支持|借助回调函数|不支持|借助回调函数|
|最大最小子集|支持|支持|支持|不支持|

