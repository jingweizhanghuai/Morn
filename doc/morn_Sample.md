## 样本

### 数据结构

----

#### 样本

``` c
typedef struct MSample
{
    int sample_num;
    int feature_num;
    
    float **feature;
    
    MMemory *memory;
    MHandleSet *handle;

    union 
    {
        int *label;
        float *value;
    };
    
    int class_num;
    
    void *reserve;
}MSample;
```

##### 成员

* sample_num：样本个数
* feature_num：特征维数
* feature：样本的各维特征值
* memory：样本内存（不建议外部使用）
* handle：样本操作集（不建议外部使用）
* label：各样本的类别标签
* value：各样本的值
* class_num：样本类别数
* reserve：预留

----

### 函数

----

####样本创建

``` c
MSample *mSampleCreate(int sample_num,int feature_num,float **feature)
```

##### 参数

* sample_num：输入，样本个数，若传入为DFLT，则不指定样本个数
* feature_num：输入，特征维数，若传入为DFLT，则不指定特征维数
* feature：输入，各维特征的初始化值，若传入为NULL，则不对样本特征值初始化

##### 返回值

所创建的样本指针

----

#### 样本释放

``` c
void mSampleRelease(MSample *sample)
```

##### 参数

* sample：输入，待释放的样本

##### 返回值

无

----

#### 样本重定义

``` c
void mSampleRedefine(MSample *sample,int sample_num,int feature_num)
```

##### 参数

* sample：输入，所需重定义的样本

* sample_num：输入，重定义的样本数，若传入为DFLT，则样本数不变
* feature_num：输入，重定义样本的特征维数，若传入为DFLT，则特征维数不变

##### 返回值

无

----

#### 样本扩充

``` c
void mSampleAppend(MSample *sample,int sample_num)
```

##### 参数

* sample：输入，待扩充的样本
* sample_num：输入，扩充的样本数（扩充之后样本总数为sample->sample_num+sample_num)

##### 返回值

无

----

#### 样本标签抽取

``` c
void mSampleClassSelect(MSample *src,MSample *dst,int label)
```

##### 参数

* src：输入，源样本
* dst：输出，抽取后的样本，若传入为NULL，则抽取后的样本存入src
* label：输入，所抽取样本的标签

##### 返回值

无



----

#### 样本随机抽取

``` c
void mSampleSubSet(MSample *src,MSample *dst,int num)
```

##### 参数

* src：输入，源样本
* dst：输出，抽取后的样本，若传入为NULL，则抽取后的样本存入src
* num：输入，抽取的样本数，若传入为DFLT，则抽取dst->sample_num个样本

##### 返回值

无

----

#### 样本随机标签抽取

```c
void mSampleClassSubSet(MSample *src,MSample *dst,int num[])
```

##### 参数

- src：输入，源样本
- dst：输出，抽取后的样本，若传入为NULL，则抽取后的样本存入src
- num：输入，抽取的各标签样本数。

##### 返回值

无

----

#### 样本子集

``` c
void mSampleSet(MSample *sample,MSample *train_set,float train_ratio,MSample *validation_set,float validation_ratio,MSample *test_set,float test_ratio)
```

##### 参数

* sample：输入，源样本
* train_set：输出，从sample中随机抽取得到的训练集，若传入为NULL，则不抽取训练集
* train_ratio：输入，训练集在总样本中所占比例
* validation_set：输出，从sample中随机抽取得到的验证集，若传入为NULL，则不抽取验证集
* validation_ratio：输入，验证集在总样本中所占比例
* test_set：输出，从sample中随机抽取得到的测试集，若传入为NULL，则不抽取测试集
* test_ratio：输入，测试集在总样本中所占比例

##### 返回值

无

----

#### 样本乱序重排

```  c
void mSampleReorder(MSample *src,MSample *dst)
```

##### 参数

- src：输入，源样本
- dst：输出，乱序重排后样本，若传入为NULL，则乱序重排后的样本存入src

##### 返回值

无

----

#### 样本合并

``` c
void mSampleMerge(MSample *src1,MSample *src2,MSample *dst)
```

##### 参数

* src1：输入，待合并源样本
* src2：输入，待合并源样本
* dst：输出，合并后样板，若传入为NULL，则合并后样本存入src1

##### 返回值

无

----

#### 样本写入



----

## 样本预处理

### 函数

----

#### 样本标准化

``` c
void mSampleStandardNormalize(MSample *src,MSample *dst,float *deviation,float *mean)
```

##### 参数

* src：输入，源样本
* dst：输出，标准化后的样本，若传入为NULL，则输出样本存入src
* deviation：输出，各维样本的方差，若传入为NULL，则不输出此参数
* mean：输出，各维样本的均值，若传入为NULL，则不输出此参数

##### 返回值

无

----

#### 样本归一化

``` c
void mSampleNormalize(MSample *src,MSample *dst,float *k,float *b)
```

##### 参数

* src：输入，源样本
* dst：输出，归一化后的样本，若传入为NULL，则输出样本存入src
* k：输出，各维特征的归一化参数，（归一化前样本，减去b，乘以k，得到归一化后样本），若传入为NULL，则不输出此参数
* b：输出，各维特征的归一化参数，（归一化前样本，减去b，乘以k，得到归一化后样本），若传入为NULL，则不输出此参数

##### 返回值

无

----

#### 样本均衡化

``` c
void mSampleEqualize(MSample *src,MSample *dst,float *min,float *max)
```

##### 参数

- src：输入，源样本
- dst：输出，均衡化后的样本，若传入为NULL，则输出样本存入src
- min：输出，各维特征的最小值，若传入为NULL，则不输出此参数
- max：输出，各维特征的最大值，若传入为NULL，则不输出此参数

##### 返回值

无

----

## 样本统计

### 函数

----

#### 样本单一特征均值

``` c
float mSampleFeatureMean(MSample *sample,int n)
```

##### 参数

* sample：输入，源样本
* n：输入，所统计特征在所有特征中的序号

##### 返回值

此维特征的平均值

----

#### 样本特征统计

``` c 
void mSampleStat(MSample *sample,float *mean,float *var,MMatrix *mat)
```

##### 参数

- sample：输入，源样本
- mean：输出，样本各维特征的均值，若传入为NULL，则不输出此统计值
- var：输出，样本各维特征的方差，若传入为NULL，则不输出此统计值
- mat：输出，样本各维特征的协方差矩阵，若传入为NULL，则不输出此统计值

##### 返回值

无

----

####样本分标签特征统计

``` c
void mSampleClassStat(MSample *sample,int quantity[],float *mean[],float *var[],MMatrix *mat[])
```

##### 参数

- sample：输入，源样本
- quantity：输出，各标签下的样本数量，若传入为NULL，则不输出此统计值
- mean：输出，各标签下的样本特征均值，若传入为NULL，则不输出此统计值
- var：输出，各标签下的样本特征方差，若传入为NULL，则不输出此统计值
- mat：输出，各标签下的样本特征协方差矩阵，若传入为NULL，则不输出此统计值

##### 返回值

无

----

#### 样本序列特征统计

``` c
void mFeatureSequenceState(MVector *feature,int duration,float *mean,float *var,MMatrix *mat)
```

##### 参数

* feature：输入，待统计的样本特征
* duration：输入，所需统计的样本数量
* mean：输出，一段时间内的样本特征均值，若传入为NULL，则不输出此统计值
* var：输出，一段时间内的样本特征方差，若传入为NULL，则不输出此统计值
* mat：输出，一段时间内的样本特征协方差矩阵，若传入为NULL，则不输出此统计值

##### 返回值

无

----

#### 样本序列分标签特征统计

``` c
void mFeatureClassState(MVector *feature,int class,int duration,float *mean,float *var,MMatrix *mat)
```

##### 参数

- feature：输入，待统计的样本特征
- class：输入，样本的标签
- duration：输入，所需统计的样本数量
- mean：输出，一段时间内相同标签的样本特征均值，若传入为NULL，则不输出此统计值
- var：输出，一段时间内相同标签的样本特征方差，若传入为NULL，则不输出此统计值
- mat：输出，一段时间内相同标签的样本特征协方差矩阵，若传入为NULL，则不输出此统计值

##### 返回值

无

----

