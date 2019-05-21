## 排序

### 函数

#### 冒泡（鸡尾酒）升序排序

```cpp
void mAscendingBubbleSort(float *data_in,int *index_in,float *data_out,int *index_out,int num)
```

##### 参数

* data_in：输入，待排序数据
* index_in：输入，待排序数据的索引
* data_out：输出，排序后的升序数据
* index_out：输出，排序后数据的索引
* num：输入，待排序的数据的个数

##### 返回值

无

##### 备注

以下调用含义如注释：

```cpp
mAscendingBubbleSort(data_in,NULL,data_out,NULL,num);			//不带索引的数据排序
mAscendingBubbleSort(data_in,NULL,NULL,NULL,num);				//不带索引的数据排序，排序后数据仍然存入data_in所指向的内存区
mAscendingBubbleSort(data_in,index_in,data_out,index_out,num);	//带索引的数据排序
mAscendingBubbleSort(data_in,NULL,data_out,index_out,num);		//带索引的数据排序，输入索引自动生成为1、2、3……
mAscendingBubbleSort(data_in,index_in,data_out,NULL,num);		//带索引的数据排序，输出的索引仍然存入index_in所指向的内存区
mAscendingBubbleSort(data_in,index_in,NULL,NULL,num);			//带索引的数据排序，输出的数据和索引分别存入data_in和index_in所指向的内存区
```

以下其它排序方法，调用同此

----

#### 快速升序排序

```cpp
void mAscendingQuickSort(float *data_in,int *index_in,float *data_out,int *index_out,int num)
```

##### 参数

- data_in：输入，待排序数据
- index_in：输入，待排序数据的索引
- data_out：输出，排序后的升序数据
- index_out：输出，排序后数据的索引
- num：输入，待排序的数据的个数

##### 返回值

无

----

#### 插入升序排序

```cpp
void mAscendingInsertionSort(float *data_in,int *index_in,float *data_out,int *index_out,int num)
```

##### 参数

- data_in：输入，待排序数据
- index_in：输入，待排序数据的索引
- data_out：输出，排序后的升序数据
- index_out：输出，排序后数据的索引
- num：输入，待排序的数据的个数

##### 返回值

无

----



##向量与矩阵

### 数据结构

####向量（MVector）

```cpp
typedef struct MVector{

    int size;
    float *data;
    
    MMemory *memory;
    MHandleSet *handle;
    
    void *reserve;

}MVector;

```

##### 成员：

* size：向量维数
* data：各维向量的值
* memory：向量内存（不建议外部使用）
* handle：向量操作集（不建议外部使用）
* reserve：预留



#### 矩阵（MMetrix）

```cpp
typedef struct MMatrix{
    int row;
    int col;
    
    float **data;
    
    MMemory *memory;
    MHandleSet *handle;
    
    void *reserve;
}MMatrix;
```

##### 成员

* row：矩阵的行数
* col：矩阵的列数
* data：矩阵元素的值
* memory：矩阵内存（不建议外部使用）
* handle：矩阵操作集（不建议外部使用）
* reserve：预留



### 函数

----

####向量创建

```cpp
 MVector *mVectorCreate(int size,float *data)
```

##### 参数

* size：输入。所创建向量的维数，传入默认值，则不指定向量维数
* data：输入。初始化向量元素的值，传入为NULL，则不指定元素的值

#####返回值

所创建向量的指针

##### 备注

当不确定向量的维数时，可使用 

```cpp
mVectorCreate(DFLT,NULL);
```

当只创建向量，不初始化时，可使用

```cpp
mVectorCreate(size,NULL);
```

-----

#### 向量释放

```cpp
void mVectorRelease(MVector *vec)
```

##### 参数

* vec：输入，所需要释放的向量

##### 返回值

无

----

#### 向量初始化

```cpp
void mVectorSetData(MVector *vec,float *data)
```

##### 参数

* vec：输入，所需要初始化的向量
* data：输入，初始化向量的值

##### 返回值

无

##### 备注

当传入的data指针所指向的值变化时，向量的相应维元素也将变化

----

#### 向量重定义

```cpp
void mVectorRedefine(MVector *vec,int size)
```

##### 参数

* vec：输入，所需重定义的向量
* size：输入，重定义后向量的维数

##### 返回值

无

----

#### 矩阵创建

```cpp
MMatrix *mMatrixCreate(int row,int col,float **data)
```

##### 参数

* row：输入，所需要创建的矩阵的行数，传入默认值，则不指定矩阵行数
* col：输入，所需要创建的矩阵的列数，传入默认值，则不指定矩阵列数
* data：输入，初始化的矩阵元素的值，传入NULL，则不对矩阵初始化

##### 返回值

所创建矩阵的指针

##### 备注

当不确定矩阵大小时，可使用

```cpp
mMatrixCreate(DFLT,DFLT,NULL);
```

当不需要对矩阵初始化时，可使用

```
mMatrixCreate(row,col,NULL);
```

----

#### 矩阵释放

```cpp
void mMatrixRelease(MMatrix *mat)
```

##### 参数

* vec：输入，所需要释放的矩阵

##### 返回值

无

-------

#### 矩阵重定义

```cpp
void mMatrixRedefine(MMatrix *mat,int row,int col)
```

##### 参数

* mat：输入，所需要重定义的矩阵
* row：输入，重定义后矩阵的行数，若传入默认值，则保持矩阵行数不变
* col：输入，重定义后矩阵的列数，若传入默认值，则保持矩阵列数不变

##### 返回值

无

----

#### 单位矩阵创建

```cpp
MMatrix *mMatrixUnitCreate(int size)
```

##### 参数

* size：输入，单位矩阵的大小

##### 返回值

所创建的单位矩阵的指针

----

#### 矩阵转置

```cpp
void mMatrixTranspose(MMatrix *src,MMatrix *dst)
```

##### 参数

* src：输入
* dst：输出，转置后的矩阵，若传入为NULL，则src源矩阵转置

##### 返回值

无

------

#### 向量加法

```cpp
void mVectorAdd(MVector *vec1,MVector *vec2,MVector *dst)
```

##### 参数

* vec1：输入，待加向量
* vec2：输入，待加向量
* dst：输出，相加后向量，若传入为NULL，则vec1指向相加后向量

##### 返回值

无

----

#### 向量乘法

```cpp
float mVectorMul(MVector *vec1,MVector *vec2)
```

##### 参数

* vec1：输入，待乘向量
* vec2：输入，待乘向量

##### 返回值

乘法运算结果

----

#### 向量数乘

```cpp
void mVectorScalarMul(MVector *vec1,MVector *vec2,MVector *dst)
```

##### 参数

- vec1：输入，待乘向量
- vec2：输入，待乘向量
- dst：输出，数乘后向量，若传入为NULL，则vec1指向数乘后向量

##### 返回值

无

----

#### 矩阵乘以向量

```cpp
void mMatrixVectorMul(MMatrix *mat,MVector *vec,MVector *dst)
```

##### 参数

* mat：输入，待乘矩阵
* vec：输入，待乘向量
* dst：输出，相乘所得向量，若传入为NULL，则vec指向相乘后向量

##### 返回值

无

----

#### 向量乘以矩阵

```cpp
void mVectorMatrixMul(MVector *vec,MMatrix *mat,MVector *dst)
```

##### 参数

- vec：输入，待乘向量
- mat：输入，待乘矩阵
- dst：输出，相乘所得向量，若传入为NULL，则vec指向相乘后向量

##### 返回值

无

----

#### 矩阵乘法

```cpp
void mMatrixMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
```

##### 参数

- mat1：输入，待乘矩阵
- mat2：输入，待乘矩阵
- dst：输出，相乘所得矩阵，若传入为NULL，则mat1指向相乘后向量

##### 返回值

无

##### 备注

支持以下调用

```cpp
mMatrixMul(mat1,mat2,NULL);
mMatrixMul(mat1,mat2,mat1);	//与前一调用方法结果相同
mMatrixMul(mat1,mat2,mat2);
```

----

#### 矩阵数乘

```cpp
void mMatrixScalarMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
```

##### 参数

- mat1：输入，待乘矩阵
- mat2：输入，待乘矩阵
- dst：输出，数乘所得矩阵，若传入为NULL，则mat1指向数乘后向量

##### 返回值

无

----

#### 矩阵行列式的值

```cpp
float mMatrixDetValue(MMatrix *mat)
```

##### 参数

- mat：输入，待计算矩阵

##### 返回值

算得的行列式的值

----

#### 矩阵的逆

```cpp
int mMatrixInverse(MMatrix *mat,MMatrix *inv)
```

##### 参数

- mat：输入，待计算矩阵
- inv：输出，mat的逆矩阵，若传入为NULL，则源矩阵mat指向逆矩阵

##### 返回值

若矩阵可逆，则返回1，否则返回0

----

#### 求解线性方程组

```cpp
int mLinearEquation(MMatrix *mat,float *answer)
```

##### 参数

- mat：输入，线性方程组的系数矩阵
- inv：输出，方程组求解的结果

##### 返回值

若方程组有解，则返回1，否则返回0

##### 备注

假设方程组为：

a * x1 + b * x2 + c = 0

d * x1 + e * x2 + f = 0

则方程组的系数矩阵为

a,b,c

d,e,f

其解为{x1,x2}

----

#### 矩阵最大元素

```cpp
void mMatrixMaxElement(MMatrix *src,float *element,int *x,int *y)
```

##### 参数

* src：输入
* element：输出，最大元素的值，若传入为NULL，则不输出此值
* x：输出，最大元素所在行数，若传入为NULL，则不输出此坐标
* y：输出，最大元素所在列数，若传入为NULL，则不输出此坐标

##### 返回值

无

----

#### 矩阵的特征值和特征向量

```cpp
void mMatrixEigenvalue(MMatrix *src,float eigenvalue[],float *eigenvector[],int *eigennum)
```

#####参数

* src：输入
* eigenvalue：输出，所找到的矩阵（实）特征值，若传入为NULL，则不输出此值
* eigenvalue：输出，矩阵（实）特征值对应的特征向量，若传入为NULL，则不输出此向量
* eigennum：输出，所找到的矩阵（实）特征值的个数，若传入为NULL，则不输出此值

##### 返回值

无

----





