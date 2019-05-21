## Morn：MMatrix矩阵和MVector向量

首先必须要说：Morn的矩阵运算不能代替Blas，因为作者测过，Morn的矩阵运算比起Blas还是要慢（这也在意料之中）。不过对于并非大规模的矩阵运算，或者对速度不敏感的情况下，还是可以考虑一下Morn的矩阵运算。

####MMatrix定义

Morn的矩阵叫做MMatrix，它的定义在[../include/morn_Matrix.h]()文件中。

```c
typedef struct MMatrix{
    int row;
    int col;
    float **data;
    MList *handle;
    MInfo info;
    void *reserve;
}MMatrix;
```

结构体中：handle、info和reserve是几乎所有Morn结构体所共有的，就不多介绍了。除此以外就只有row、col和data三个成员，row就是矩阵的行数，col就是矩阵的列数，data是个索引，里面存放的是矩阵中每个元素的值，例如mat->data\[2][3]就是mat这个矩阵中第二行第三列的值。

另外，可以看到，MMatrix中的元素是float类型的，如果元素是其它类型，就不能用MMatrix了，可以考虑用MTable，或者可以考虑把数据转成float类型。

还有，MMatrix中元素的存放是按行存放的，同一行元素在内存中是连续存放的，行之间可不一定。



####MMatrix基本操作

MList的函数都定义在[../src/math/morn_Matrix.c](../src/math/morn_Matrix.c)文件中。



**创建MMatrix**：

```c
MMatrix *mMatrixCreate(int row,int col,float **data);
```

其中row是矩阵的行数，col是矩阵的列数，（注意：row是第一个参数，col是第二个参数，用的时候别写反了），data是数据索引，如果没有初始化的外部数据（通常都没有），那么data传入NULL。col和row的默认值都是0，所以如果取默认值得话，只会创建一个矩阵头，而不会为矩阵元素申请内存空间。



**释放MMatrix**：

```c
void mMatrixRelease(MMatrix *mat);
```

用`mMatrixCreate`创建的MMatrix必须且只能用`mMatrixRelease`来释放。



**矩阵重定义**：

```c
void mMatrixRedefine(MMatrix *mat,int row,int col)
```

如果你想改变一个矩阵的行数或者列数，那么你需要重新分配内存空间，重新生成索引，那么你就需要使用`mMatrixRedefine`。



**将矩阵设置为单位矩阵**：

```c
void mUnitMatrix(MMatrix *mat,int size);
```

如果不指定size的话，矩阵应该是个方阵（行数==列数）。



**矩阵转置**：

```c
void mMatrixTranspose(MMatrix *mat,MMatrix *dst);
```

dst的默认值是mat。



**找出矩阵中的最大元素、最小元素**：

```c
void mMatrixMaxElement(MMatrix *src,float *element,int *x,int *y);
void mMatrixMinElement(MMatrix *src,float *element,int *x,int *y);
```

element是矩阵中最大元素的值，x、y是最大元素出现的位置（第x列、第y行）。



**矩阵乘以向量**：

```c
void mMatrixVectorMul(MMatrix *mat,MVector *vec,MVector *dst);
```

向量的长度应该等于矩阵的列数，dst的默认值是vec。



**向量乘以矩阵**：

```c
void mVectorMatrixMul(MVector *vec,MMatrix *mat,MVector *dst);
```

向量的长度应该等于矩阵的行数，dst的默认值是vec。



**矩阵的加法、减法和数乘**：

```c
void mMatrixAdd(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)；
void mMatrixSub(MMatrix *mat1,MMatrix *mat2,MMatrix *dst)
void mMatrixScalarMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst);
```

要求两个矩阵尺寸相同。dst的默认值是mat1。

注意矩阵数乘与矩阵乘法的区别，这个数乘就是两个矩阵里面相同位置的数做乘法。



**矩阵乘法**：

```c
void mMatrixMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst);
```

注意与矩阵数乘的区别，这个才是线性代数里的矩阵乘法。要求mat1的列数等于mat2的行数。dst的默认值是mat1。



**计算行列式的值**：

```c
float mMatrixDetValue(MMatrix *mat);
```

要求矩阵必须是方阵（也就是行数==列数），返回值就是行列式的值。



**计算矩阵的逆**：

```c
int mMatrixInverse(MMatrix *mat,MMatrix *inv);
```

要求矩阵必须是方阵（也就是行数==列数），inv的默认值是mat，也就是说你没有指定输出的话，结果会保存在输入矩阵里。

注意，不是所有的矩阵都有逆矩阵，所以这个函数有返回值，当返回1的时候表示矩阵可逆，返回0的时候表示矩阵不可逆。如果矩阵不可逆，那么inv的值没有意义。



**求解线性方程组**：

```c
int mLinearEquation(MMatrix *mat,float *answer);
```

如果你有一个二元一次方程组为`ax+by+c=0`，`dx+ey+f=0`，那么你应该把矩阵设置为

```
第一行：a  b  c
第二行：d  e  f
```

也就是说对于二元一次方程组，输入的矩阵尺寸应该是2×3，对于三元一次方程组，输入的矩阵尺寸应该是3×4，以此类推，对于n元一次方程组，输入的矩阵应该是(n)×(n+1)。

方程组的求解结果放在answer里，如上例，answer[0]就是x的求解结果，answer[1]就是y的求解结果，以此类推。

另外，因为不是所有的方程组都有解，所以这个函数有返回值，当返回为1的时候，表示方程组可解，返回值为0的时候表示方程组无解，返回为0的话answer的值没有意义。



**计算方阵的（实）特征值和特征向量**：

```c
void mMatrixEigenvalue(MMatrix *src,float eigenvalue[],float *eigenvector[],int *eigennum)；
```

要求输入矩阵src为方阵。

eigennum是得到的特征值的个数。

eigenvalue是各个特征值得值。

eigenvector是各个特征值对应的特征向量。

#### mat文件











