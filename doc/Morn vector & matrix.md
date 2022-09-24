## Matrix & Vector

Morn provides some functions for small and medium dense sizes matrix/vector calculation encountered in various mobile applications and embedded development. And `blas`/`Eigen`/`MKL`/`Armadillo` are optimized primarily for large dense sizes matrix .



#### API

##### MVector

It defined in [/include/morn_Matrix.h]().

```c
typedef struct MVector
{
    int size;
    float *data;
}MVector;
```

Here, `size` is total numbers of `MVector` values. And `data` is starting address of vector value. thus `vec->data[2]` is the 3rd value of `vec`.

In Morn, only `float` is support in vector calculate.



note:

`MVector` is just a vector, and not a data container. It is different from `std::vector` in C++, which is more similar to `MArray` in Morn.



##### MMatrix

It defined in [/include/morn_Matrix.h]().

```c
typedef struct MMatrix
{
    int row;
    int col;
    float **data;
}MMatrix;
```

Here, `row`/`col` is the  the number of rows and columns in the matrix.

`data` is a pointer of Matrix value, `mat->data[2][3]` is the `mat` value at third row and fourth column.

Only `float` is supported in `MMatrix`.



note:

the value in `MMatrix` stores row by row, thus values in same row is contiguous, but between adjacent rows may not.



the source code of following functions with `MMatrix` and `MVector` is [../src/math/morn_Matrix.c](../src/math/morn_Matrix.c) and [../src/math/morn_Matrix2.c](../src/math/morn_Matrix2.c).



##### Vector Create

```c
MVector *mVectorCreate();
MVector *mVectorCreate(int size);
MVector *mVectorCreate(int size,float *data);
```

Before using `MVector`, you must use this function to create it.

`size` is total numbers of `MVector` values, when it is not given, it will be 0, thus only a header of `MVector` will be create, without allocate from memory.

If you need point `MVector`  to some existed values, you should set the `data` to the starting address of these values. And if not, set `data` NULL.



##### Vector Release

```c
void mVectorRelease(MVector *vec);
```

The `MVector` created by `mVectorCreate`, should and must release by `mVectorRelease`.



##### Vector Redefine

```c
void mVectorRedefine(MVector *vec,int size);
void mVectorRedefine(MVector *vec,int size,float *data);
```

If you want to change `MVector`'s `size` or `data`, `mVectorRedefine` is safe and necessary, rather than change the structure directly.

This function will reallocate the memory, and reset `vec`'s operation.



##### Matrix Create

```c
MMatrix *mMatrixCreate();
MMatrix *mMatrixCreate(int row,int col);
MMatrix *mMatrixCreate(int row,int col,float **data);
```

`MMatrix` can only be create by `mMatrixCreate`.

Here `row` and `col` is the row and column of create matrix, and if not set, it will create a header of `MMatrix`, thus row and column is 0, without memory allocation.

`data` is the pointer of external data, if no, the default data is `NULL`.



##### Matrix Release

```c
void mMatrixRelease(MMatrix *mat);
```

The `MMatrix` created by `mMatrixCreate`, should and must release by `mMatrixRelease`.



##### Matrix Redefine

```c
void mMatrixRedefine(MMatrix *mat,int row,int col);
void mMatrixRedefine(MMatrix *mat,int row,int col,float **data);
```

If you want to change the rows or columns of `MMatrix`, that is you need memory reallocation and data index generation, than `mMatrixRedefine` is necessary.



##### Load Vector Data

```c
void mVectorData(MVector *vec,Type *data);
```

This function is used to load data from array to `MVector`. 

The data `Type` can be `uint8_t`,  `int8_t`,  `uint16_t`,  `int16_t`,  `uint32_t`,  `int32_t`,  `uint64_t`,  `uint64_t`, `float` and `double`.  



##### Load Matrix Data

```c
void mMatrixData(MMatrix *mat,Type *data);
```

This function is used to load data from array to `MMatrix`. 

The data `Type` can be `uint8_t`,  `int8_t`,  `uint16_t`,  `int16_t`,  `uint32_t`,  `int32_t`,  `uint64_t`,  `uint64_t`, `float` and `double`.  



##### Generate Unit Matrix

```c
void mUnitMatrix(MMatrix *mat);
void mUnitMatrix(MMatrix *mat,int size);
```

When `size` of matrix is not given, the input `mat` must be a square-matrix, thus `mat->row==mat->col`.



##### Matrix Transpose

```c
void mMatrixTranspose(MMatrix *mat);
void mMatrixTranspose(MMatrix *src,MMatrix *dst);
```

`mMatrixTranspose(mat)` is same with ``mMatrixTranspose(mat,mat)`.



##### Get Matrix Maximum/Minimum Element

```c
float mMatrixMaxElement(MMatrix *mat);
float mMatrixMaxElement(MMatrix *mat,int *x,int *y);

float mMatrixMinElement(MMatrix *mat);
float mMatrixMinElement(MMatrix *mat,int *x,int *y);
```

`x` and `y` is the coordinate locate of maximum/minimum element, if necessary.



##### Vector Addition, Subtraction and Scalar Multiplication

```c
void mVectorAdd(MVector *vec1,MVector *vec2);
void mVectorAdd(MVector *vec1,MVector *vec2,MVector *dst);

void mVectorSub(MVector *vec1,MVector *vec2);
void mVectorSub(MVector *vec1,MVector *vec2,MVector *dst);

void mVectorScalarMul(MVector *vec1,MVector *vec2);
void mVectorScalarMul(MVector *vec1,MVector *vec2,MVector *dst);
```

Vectors with these operations must have same size.

When `dst` is not given, the result will be written to `vec1`.



##### Vector Multiplication

```c
float mVectorMul(MVector *vec1,MVector *vec2);
```

Vectors with dot Multiplication must have same size.



note

Scale multiplication and cross multiplication is different.



##### Multiplication with Matrix and Vector

```c
void mMatrixVectorMul(MMatrix *mat,MVector *vec);
void mMatrixVectorMul(MMatrix *mat,MVector *vec,MVector *dst);

void mVectorMatrixMul(MVector *vec,MMatrix *mat);
void mVectorMatrixMul(MVector *vec,MMatrix *mat,MVector *dst);
```

For `mMatrixVectorMul`, `mat->col==vec->size` is necessary.

For `mVectorMatrixMul`, `mat->row==vec->size` is necessary.

When `dst` is not given, the result will be writen to `vec`.



##### Matrix Addition, Subtraction and Scalar Multiplication

```c
void mMatrixAdd(MMatrix *mat1,MMatrix *mat2);
void mMatrixAdd(MMatrix *mat1,MMatrix *mat2,MMatrix *dst);

void mMatrixSub(MMatrix *mat1,MMatrix *mat2);
void mMatrixSub(MMatrix *mat1,MMatrix *mat2,MMatrix *dst);

void mMatrixScalarMul(MMatrix *mat1,MMatrix *mat2);
void mMatrixScalarMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst);
```

Matrixes with these operations must have same size.

When `dst` is not given, the result will be written to `mat1`.



##### Matrix Multiplication

```c
void mMatrixMul(MMatrix *mat1,MMatrix *mat2);
void mMatrixMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst);
```

For matrix multiplication, `mat1->cow` must be equal with `mat2->row`.

When `dst` is not given, the result will be written to `mat1`.



note

Matrix scale multiplication and matrix multiplication is different.



#####  Matrix Determinant 

```c
float mMatrixDetValue(MMatrix *mat);
```

This `mat` must be a square matrix, thus `mat->row==mat->col`.

It returns the value of determinant.



##### Matrix Inverse

```c
int mMatrixInverse(MMatrix *mat,MMatrix *inv);
```

This `mat` must be a square matrix, thus `mat->row==mat->col`.

When `inv` not given, the inverse matrix will be written to `mat`.

Node that: not all matrixes can inverse, If the matrix is irreversible the function will return 0, else return 1.



##### 计算方阵的（实）特征值和特征向量

```c
void mMatrixEigenvalue(MMatrix *src,float eigenvalue[],float *eigenvector[],int *eigennum)；
```

要求输入矩阵src为方阵。

eigennum是得到的特征值的个数。

eigenvalue是各个特征值得值。

eigenvector是各个特征值对应的特征向量。



 This is not a full linear algebra library 

We care not only about speed, but also about minimizing power usage. We specifically care about charge usage in mobile/embedded devices. This implies that we care doubly about minimizing memory bandwidth usage: we care about it, like any GEMM, because of the impact on speed, and we also care about it because it is a key factor of power usage.

Most GEMMs are optimized primarily for large dense matrix sizes (>= 1000). We do care about large sizes, but we also care specifically about the typically smaller matrix sizes encountered in various mobile applications. This means that we have to optimize for all sizes, not just for large enough sizes.



```c
#include "Eigen/Dense"
#include "morn_math.h"

void test_mul(int K)
{
    printf("eigen use simd %s\n",Eigen::SimdInstructionSetsInUse());
    
    printf("matrix mul, size: %d*%d\n",K,K);
    Eigen::MatrixXf m_a(K,K);
    Eigen::MatrixXf m_b(K,K);
    MMatrix *mat_a = mMatrixCreate(K,K);
    MMatrix *mat_b = mMatrixCreate(K,K);
    
    for(int j=0;j<K;j++)for(int i=0;i<K;i++)
    {
        float v;
        v=mRand(-1000,1000)/1000.0;
        m_a(j,i)=v;
        mat_a->data[j][i]=v;

        v=mRand(-1000,1000)/1000.0;
        m_b(j,i)=v;
        mat_b->data[j][i]=v;
    }

    Eigen::MatrixXf m_mul;
    mTimerBegin("eigen");
    for(int i=0;i<T;i++)
        m_mul = m_a*m_b;
    mTimerEnd("eigen");

    MMatrix *mat_mul = mMatrixCreate();
    mTimerBegin("Morn");
    for(int i=0;i<T;i++)
        mMatrixMul(mat_a,mat_b,mat_mul);
    mTimerEnd("Morn");
    mMatrixRelease(mat_mul);
    
    mMatrixRelease(mat_a);
    mMatrixRelease(mat_b);
}
```













