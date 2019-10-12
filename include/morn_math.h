/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _MORN_MATH_H_
#define _MORN_MATH_H_

#include "morn_util.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MORN_DEFAULT -1

#define MORN_PI 3.141592653589793238462643383279502884197169399375105820974944592308
#define MORN_E  2.718281828459045235360287471352662497757247093699959574966967627724

#define mMathABS(x) (((x)>0)?(x):(-(x)))
#define mMathABSDiff(x,y) (((x)>(y))?((x)-(y)):((y)-(x)))

#define mMathFLoatABS(pin,pout) {*((int *)(pout)) =(*((int *)(pin)))&0x7FFFFFFF;}
#define mMathIntABS(pin,pout) {*pout = mMathABS(*pin);}

#define mLinearInterp(x1,l1,x2,l2,l) (((x1)*((l2)-(l))+(x2)*((l)-(l1)))/((l2)-(l1)))

float mSin(float a);
float mCos(float a);
void mMean(float *in,int num,float *sum,float *mean);
void mVariance(float *in,int num,float *mean,float *variance);
void mCovariance(float *in1,float *in2,int num,float *mean1,float *mean2,float *covariance);

static inline float mSup() {float a=0.0f;return  1.0f/a;}
static inline float mInf() {float a=0.0f;return -1.0f/a;}
#ifdef NAN
#define mNan() NAN
#else
#define mNan() nan(NULL)
#endif
#define mIsSup(A) (isinf(A)&&(A>0))
#define mIsInf(A) (isinf(A)&&(A<0))
#define mIsNan(A) isnan(A)
#define mIsInteger(A) (ABS(A-((int)(A+0.5f)))<0.00001f)

double mSigmoid(float x);

typedef struct MVector{
    int size;
    float *data;
    
    MList *handle;
    MInfo info;
    void *reserve;
}MVector;

#define INVALID_VEC(Vec) ((((Vec) ==NULL)||((intptr_t)(Vec) == -1))?1:(((Vec)->size <= 0)||((intptr_t)((Vec)->data) <= 0)))

MVector *mVectorCreate(int size,float *data);
void mVectorSetData(MVector *vec,float *data);
void mVectorRelease(MVector *vec);
void mVectorRedefine(MVector *vec,int size,float *data);
    
#define mVectorExchange(Vec1,Vec2) mObjectExchange(Vec1,Vec2,MVector)
#define mVectorReset(Vec) mHandleReset(Vec->handle)

typedef struct MMatrix{
    int row;
    int col;
    
    float **data;
    
    MList *handle;
    MInfo info;
    void *reserve;
}MMatrix;

#define INVALID_MAT(Mat) ((((Mat) ==NULL)||((intptr_t)(Mat) == -1))?1:(((Mat)->data == NULL)||((intptr_t)((Mat)->data) <= 0)\
                                                                ||((Mat)->col <= 0)\
                                                                ||((Mat)->row <= 0)))

MMatrix *mMatrixCreate(int row,int col,float **data);
void mMatrixRelease(MMatrix *mat);
void mMatrixRedefine(MMatrix *mat,int row,int col,float ** data);

#define mMatrixExchange(Mat1,Mat2) mObjectExchange(Mat1,Mat2,MMatrix)
#define mMatrixReset(Mat) mHandleReset(Mat->handle)
void mMATWrite(MFile *file,MMatrix *mat,char *matname);
void mMATRead(MFile *file,char *matname,MMatrix *mat);

void mVectorAdd(MVector *vec1,MVector *vec2,MVector *dst);
float mVectorMul(MVector *vec1,MVector *vec2);
void mMatrixVectorMul(MMatrix *mat,MVector *vec,MVector *dst);
void mVectorMatrixMul(MVector *vec,MMatrix *mat,MVector *dst);
void mMatrixMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst);

void mMatrixTranspose(MMatrix *mat,MMatrix *dst);
float mMatrixDetValue(MMatrix *mat);
int mMatrixInverse(MMatrix *mat,MMatrix *inv);

void mMatrixMaxElement(MMatrix *src,float *element,int *x,int *y);
void mMatrixEigenvalue(MMatrix *src,float eigenvalue[],float *eigenvector[],int *eigennum);

int mLinearEquation(MMatrix *mat,float *answer);

void mLinearFitLSM(float *XIn,float *YIn,int N,float *A);
void mParabolaFitLSM(float *XIn,float *YIn,int N,float *A);
void mPolyFitLSM(float *XIn,float *YIn,int N,float *A,int k);

void mLinearFitWeight(float *XIn,float *YIn,float *WIn,int N,float *A);
void mParabolaFitWeight(float *XIn,float *YIn,float *WIn,int N,float *A);
void mPolyFitWeight(float *XIn,float *YIn,float *WIn,int N,float *A,int k);

void mLinearFit(float *XIn,float *YIn,int N,float *A);
void mParabolaFit(float *XIn,float *YIn,int N,float *A);
void mPolyFit(float *XIn,float *YIn,int N,float *A,int k);

void mAscSortD64(D64 *data_in,int *index_in,D64 *data_out,int *index_out,int num);
void mAscSortF32(F32 *data_in,int *index_in,F32 *data_out,int *index_out,int num);
void mAscSortS32(S32 *data_in,int *index_in,S32 *data_out,int *index_out,int num);
void mAscSortU32(U32 *data_in,int *index_in,U32 *data_out,int *index_out,int num);
void mAscSortS16(S16 *data_in,int *index_in,S16 *data_out,int *index_out,int num);
void mAscSortU16(U16 *data_in,int *index_in,U16 *data_out,int *index_out,int num);
void mAscSortS8 (S8  *data_in,int *index_in,S8  *data_out,int *index_out,int num);
void mAscSortU8 (U8  *data_in,int *index_in,U8  *data_out,int *index_out,int num);
#define mAscSort(Type,DataIn,IndexIn,DataOut,IndexOut,Num) mAscSort##Type(DataIn,IndexIn,DataOut,IndexOut,Num)

void mDescSortD64(D64 *data_in,int *index_in,D64 *data_out,int *index_out,int num);
void mDescSortF32(F32 *data_in,int *index_in,F32 *data_out,int *index_out,int num);
void mDescSortS32(S32 *data_in,int *index_in,S32 *data_out,int *index_out,int num);
void mDescSortU32(U32 *data_in,int *index_in,U32 *data_out,int *index_out,int num);
void mDescSortS16(S16 *data_in,int *index_in,S16 *data_out,int *index_out,int num);
void mDescSortU16(U16 *data_in,int *index_in,U16 *data_out,int *index_out,int num);
void mDescSortS8 (S8  *data_in,int *index_in,S8  *data_out,int *index_out,int num);
void mDescSortU8 (U8  *data_in,int *index_in,U8  *data_out,int *index_out,int num);
#define mDescSort(Type,DataIn,IndexIn,DataOut,IndexOut,Num) mDescSort##Type(DataIn,IndexIn,DataOut,IndexOut,Num)

D64 mMinSubsetD64(D64 *data_in,int *index_in,int num_in,D64 *data_out,int *index_out,int num_out);
F32 mMinSubsetF32(F32 *data_in,int *index_in,int num_in,F32 *data_out,int *index_out,int num_out);
S32 mMinSubsetS32(S32 *data_in,int *index_in,int num_in,S32 *data_out,int *index_out,int num_out);
U32 mMinSubsetU32(U32 *data_in,int *index_in,int num_in,U32 *data_out,int *index_out,int num_out);
S16 mMinSubsetS16(S16 *data_in,int *index_in,int num_in,S16 *data_out,int *index_out,int num_out);
U16 mMinSubsetU16(U16 *data_in,int *index_in,int num_in,U16 *data_out,int *index_out,int num_out);
 S8 mMinSubsetS8 ( S8 *data_in,int *index_in,int num_in, S8 *data_out,int *index_out,int num_out);
 U8 mMinSubsetU8 ( U8 *data_in,int *index_in,int num_in, U8 *data_out,int *index_out,int num_out);
#define mMinSubset(Type,DataIn,IndexIn,NumIn,DataOut,IndexOut,NumOut) mMinSubset##Type(DataIn,IndexIn,NumIn,DataOut,IndexOut,NumOut)

D64 mMaxSubsetD64(D64 *data_in,int *index_in,int num_in,D64 *data_out,int *index_out,int num_out);
F32 mMaxSubsetF32(F32 *data_in,int *index_in,int num_in,F32 *data_out,int *index_out,int num_out);
S32 mMaxSubsetS32(S32 *data_in,int *index_in,int num_in,S32 *data_out,int *index_out,int num_out);
U32 mMaxSubsetU32(U32 *data_in,int *index_in,int num_in,U32 *data_out,int *index_out,int num_out);
S16 mMaxSubsetS16(S16 *data_in,int *index_in,int num_in,S16 *data_out,int *index_out,int num_out);
U16 mMaxSubsetU16(U16 *data_in,int *index_in,int num_in,U16 *data_out,int *index_out,int num_out);
 S8 mMaxSubsetS8 ( S8 *data_in,int *index_in,int num_in, S8 *data_out,int *index_out,int num_out);
 U8 mMaxSubsetU8 ( U8 *data_in,int *index_in,int num_in, U8 *data_out,int *index_out,int num_out);
#define mMaxSubset(Type,DataIn,IndexIn,NumIn,DataOut,IndexOut,NumOut) mMaxSubset##Type(DataIn,IndexIn,NumIn,DataOut,IndexOut,NumOut)

#define MAX_TENSOR_BATCH 32

double mCaculate(char *str);


unsigned int mHash(const char *in,int size);

#ifdef __cplusplus
}
#endif

#endif
