#include "morn_math.h"

// gcc -O2 -fopenmp test_matrix.c -o test_matrix.exe -lmorn
int main()
{
    MMatrix *mat1 = mMatrixCreate(4,4);
    MMatrix *mat2 = mMatrixCreate(4,4);

    mUnitMatrix(mat1,4);
    mUnitMatrix(mat2,4);
    float **m2 = mat2->data;
    for(int j=0;j<4;j++)for(int i=0;i<4;i++)
    {
        m2[j][i] = (float)mRand(0,10);
    }
    PrintMat(mat1);
    PrintMat(mat2);

    mMatrixMul(mat1,mat2,mat2);
    PrintMat(mat2);
    
    mMatrixRelease(mat1);
    mMatrixRelease(mat2);
}