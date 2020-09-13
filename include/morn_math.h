/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
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
#ifndef PI
#define PI MORN_PI
#endif

#define mMathABS(x) (((x)>0)?(x):(-(x)))
#define mMathABSDiff(x,y) (((x)>(y))?((x)-(y)):((y)-(x)))

#define mMathFLoatABS(pin,pout) {*((int *)(pout)) =(*((int *)(pin)))&0x7FFFFFFF;}
#define mMathIntABS(pin,pout) {*pout = mMathABS(*pin);}

#define mLinearInterp(x1,l1,x2,l2,l) (((x1)*((l2)-(l))+(x2)*((l)-(l1)))/((l2)-(l1)))

float mSin(float a);
float mCos(float a);
#define mTan(a) (mSin(a)/mCos(a))
#define mCot(a) (mCos(a)/mSin(a))
void mMean(float *in,int num,float *sum,float *mean);
void mVariance(float *in,int num,float *mean,float *variance);
void mCovariance(float *in1,float *in2,int num,float *mean1,float *mean2,float *covariance);

int GreatestCommonDivisor(int n,...);
#define mGCD(...) GreatestCommonDivisor(VA_ARG_NUM(__VA_ARGS__),__VA_ARGS__)
int LowestCommonMultiple(int n,...);
#define mLCM(...) LowestCommonMultiple(VA_ARG_NUM(__VA_ARGS__),__VA_ARGS__)

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

int mPermutation(int *idx,int n,int m);
int mCombination(int *idx,int n,int m);

double mSigmoid(float x);

typedef struct MVector{
    int size;
    float *data;
    Morn;
    void *reserve;
}MVector;

#define INVALID_VEC(Vec) ((((Vec) ==NULL)||((intptr_t)(Vec) == -1))?1:(((Vec)->size <= 0)||((intptr_t)((Vec)->data) <= 0)))

MVector *VectorCreate(int size,float *data,int dev);
#define mVectorCreate(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==0)?VectorCreate(DFLT,NULL,DFLT):\
    (VA_ARG_NUM(__VA_ARGS__)==1)?VectorCreate(VA_ARG0(__VA_ARGS__),NULL,DFLT):\
    (VA_ARG_NUM(__VA_ARGS__)==2)?VectorCreate(VA_ARG0(__VA_ARGS__),(float *)VA_ARG1(__VA_ARGS__),DFLT):\
    (VA_ARG_NUM(__VA_ARGS__)==3)?VectorCreate(VA_ARG0(__VA_ARGS__),(float *)VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__)):\
    NULL\
)

void VectorRedefine(MVector *vec,int size,float *data,int dev);
#define mVectorRedefine(Vec,...) do{\
    int N=VA_ARG_NUM(__VA_ARGS__);\
         if(N==1) VectorRedefine(Vec,VA_ARG0(__VA_ARGS__),(Vec)->data,DFLT);\
    else if(N==2) VectorRedefine(Vec,VA_ARG0(__VA_ARGS__),(float *)VA_ARG1(__VA_ARGS__),DFLT);\
    else if(N==3) VectorRedefine(Vec,VA_ARG0(__VA_ARGS__),(float *)VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input para");\
}while(0)

void mVectorRelease(MVector *vec);

#define mVectorData(Vec,Data) do{\
    float *Buff;if(Vec->dev==MORN_HOST_CPU) Buff=Vec->data; else Buff=mMalloc(Vec->size*sizeof(float));\
    for(int I=0;I<Vec->size;I++) Buff[I]=(float)(Data[I]);\
    if(Buff!=Vec->data) {MemCopy(Vec->data,Vec->dev,Buff,MORN_HOST_CPU,Vec->size);mFree(Buff);}\
}while(0)
    
#define mVectorExchange(Vec1,Vec2) mObjectExchange(Vec1,Vec2,MVector)
#define mVectorReset(Vec) mHandleReset(Vec->handle)

typedef struct MMatrix{
    int row;
    int col;
    float **data;
    Morn;
    void *reserve;
}MMatrix;

#define INVALID_MAT(Mat) ((((Mat) ==NULL)||((intptr_t)(Mat) == -1))?1:(((Mat)->data == NULL)||((intptr_t)((Mat)->data) <= 0)\
                                                                ||((Mat)->col <= 0)\
                                                                ||((Mat)->row <= 0)))

MMatrix *MatrixCreate(int row,int col,float **data,int dev);
#define mMatrixCreate(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==0)?MatrixCreate(DFLT,DFLT,NULL,DFLT):\
    (VA_ARG_NUM(__VA_ARGS__)==2)?MatrixCreate(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),NULL,DFLT):\
    (VA_ARG_NUM(__VA_ARGS__)==3)?MatrixCreate(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),(float **)VA_ARG2(__VA_ARGS__),DFLT):\
    (VA_ARG_NUM(__VA_ARGS__)==4)?MatrixCreate(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),(float **)VA_ARG2(__VA_ARGS__),VA_ARG3(__VA_ARGS__)):\
    NULL\
)

void mMatrixRelease(MMatrix *mat);

void MatrixRedefine(MMatrix *mat,int row,int col,float ** data,int dev);
#define mMatrixRedefine(Mat,...) do{\
    int N=VA_ARG_NUM(__VA_ARGS__);\
         if(N==2) MatrixRedefine(Mat,VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),(Mat)->data,DFLT);\
    else if(N==3) MatrixRedefine(Mat,VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),(float **)VA_ARG2(__VA_ARGS__),DFLT);\
    else if(N==4) MatrixRedefine(Mat,VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),(float **)VA_ARG2(__VA_ARGS__),VA_ARG3(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input para");\
}while(0)

#define mMatrixExchange(Mat1,Mat2) mObjectExchange(Mat1,Mat2,MMatrix)
#define mMatrixReset(Mat) mHandleReset(Mat->handle)
void mMATWrite(MFile *file,MMatrix *mat,char *matname);
void mMATRead(MFile *file,char *matname,MMatrix *mat);

#define mMatrixData(Mat,Data) do{\
    int Num = Mat->col*Mat->row;\
    float *Buff;if(Mat->dev==MORN_HOST_CPU) Buff=&(Mat->data[0][0]);else Buff=mMalloc(Num*sizeof(float));\
    for(int i=0;i<Num;i++) Buff[i]=(float)(Data[i]);\
    if(Mat->dev!=MORN_HOST_CPU)\
    {\
        MemCopy(&(Mat->data[0][0]),Mat->dev,Buff,MORN_HOST_CPU,Num);\
        mFree(Buff);\
    }\
}while(0)


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

void mAscSortU8 (U8  *data_in,int *index_in,U8  *data_out,int *index_out,int num);
void mAscSortS8 (S8  *data_in,int *index_in,S8  *data_out,int *index_out,int num);
void mAscSortU16(U16 *data_in,int *index_in,U16 *data_out,int *index_out,int num);
void mAscSortS16(S16 *data_in,int *index_in,S16 *data_out,int *index_out,int num);
void mAscSortU32(U32 *data_in,int *index_in,U32 *data_out,int *index_out,int num);
void mAscSortS32(S32 *data_in,int *index_in,S32 *data_out,int *index_out,int num);
void mAscSortU64(U64 *data_in,int *index_in,U64 *data_out,int *index_out,int num);
void mAscSortS64(S64 *data_in,int *index_in,S64 *data_out,int *index_out,int num);
void mAscSortF32(F32 *data_in,int *index_in,F32 *data_out,int *index_out,int num);
void mAscSortD64(D64 *data_in,int *index_in,D64 *data_out,int *index_out,int num);
#define mAscSort(DataIn,IndexIn,DataOut,IndexOut,Num) do{\
    int Type=mDataType(DataIn);\
         if(Type==MORN_TYPE_U8 ) mAscSortU8 ((U8  *)DataIn,IndexIn,(U8  *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S8 ) mAscSortS8 ((S8  *)DataIn,IndexIn,(S8  *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U16) mAscSortU16((U16 *)DataIn,IndexIn,(U16 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S16) mAscSortS16((S16 *)DataIn,IndexIn,(S16 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U32) mAscSortU32((U32 *)DataIn,IndexIn,(U32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S32) mAscSortS32((S32 *)DataIn,IndexIn,(S32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U64) mAscSortU64((U64 *)DataIn,IndexIn,(U64 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S64) mAscSortS64((S64 *)DataIn,IndexIn,(S64 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_F32) mAscSortF32((F32 *)DataIn,IndexIn,(F32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_D64) mAscSortD64((D64 *)DataIn,IndexIn,(D64 *)DataOut,IndexOut,Num);\
    else mException(1,EXIT,"invalid input dD64ata type");\
}while(0)

void mDescSortU8 (U8  *data_in,int *index_in,U8  *data_out,int *index_out,int num);
void mDescSortS8 (S8  *data_in,int *index_in,S8  *data_out,int *index_out,int num);
void mDescSortU16(U16 *data_in,int *index_in,U16 *data_out,int *index_out,int num);
void mDescSortS16(S16 *data_in,int *index_in,S16 *data_out,int *index_out,int num);
void mDescSortU32(U32 *data_in,int *index_in,U32 *data_out,int *index_out,int num);
void mDescSortS32(S32 *data_in,int *index_in,S32 *data_out,int *index_out,int num);
void mDescSortU64(U64 *data_in,int *index_in,U64 *data_out,int *index_out,int num);
void mDescSortS64(S64 *data_in,int *index_in,S64 *data_out,int *index_out,int num);
void mDescSortF32(F32 *data_in,int *index_in,F32 *data_out,int *index_out,int num);
void mDescSortD64(D64 *data_in,int *index_in,D64 *data_out,int *index_out,int num);
#define mDescSort(DataIn,IndexIn,DataOut,IndexOut,Num) do{\
    int Type=mDataType(DataIn);\
         if(Type==MORN_TYPE_U8 ) mDescSortU8 ((U8  *)DataIn,IndexIn,(U8  *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S8 ) mDescSortS8 ((S8  *)DataIn,IndexIn,(S8  *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U16) mDescSortU16((U16 *)DataIn,IndexIn,(U16 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S16) mDescSortS16((S16 *)DataIn,IndexIn,(S16 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U32) mDescSortU32((U32 *)DataIn,IndexIn,(U32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S32) mDescSortS32((S32 *)DataIn,IndexIn,(S32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U64) mDescSortU64((U64 *)DataIn,IndexIn,(U64 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S64) mDescSortS64((S64 *)DataIn,IndexIn,(S64 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_F32) mDescSortF32((F32 *)DataIn,IndexIn,(F32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_D64) mDescSortD64((D64 *)DataIn,IndexIn,(D64 *)DataOut,IndexOut,Num);\
    else mException(1,EXIT,"invalid input data type");\
}while(0)

double mMinSubsetU8 (U8  *data_in,int *index_in,int num_in,U8  *data_out,int *index_out,int num_out);
double mMinSubsetS8 (S8  *data_in,int *index_in,int num_in,S8  *data_out,int *index_out,int num_out);
double mMinSubsetU16(U16 *data_in,int *index_in,int num_in,U16 *data_out,int *index_out,int num_out);
double mMinSubsetS16(S16 *data_in,int *index_in,int num_in,S16 *data_out,int *index_out,int num_out);
double mMinSubsetU32(U32 *data_in,int *index_in,int num_in,U32 *data_out,int *index_out,int num_out);
double mMinSubsetS32(S32 *data_in,int *index_in,int num_in,S32 *data_out,int *index_out,int num_out);
double mMinSubsetU64(U64 *data_in,int *index_in,int num_in,U64 *data_out,int *index_out,int num_out);
double mMinSubsetS64(S64 *data_in,int *index_in,int num_in,S64 *data_out,int *index_out,int num_out);
double mMinSubsetF32(F32 *data_in,int *index_in,int num_in,F32 *data_out,int *index_out,int num_out);
double mMinSubsetD64(D64 *data_in,int *index_in,int num_in,D64 *data_out,int *index_out,int num_out);
#define mMinSubset(DataIn,IndexIn,NumIn,DataOut,IndexOut,NumOut) (\
 (mDataType(DataIn)==MORN_TYPE_U8 )?mMinSubsetU8 ((U8  *)DataIn,IndexIn,NumIn,(U8  *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_S8 )?mMinSubsetS8 ((S8  *)DataIn,IndexIn,NumIn,(S8  *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_U16)?mMinSubsetU16((U16 *)DataIn,IndexIn,NumIn,(U16 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_S16)?mMinSubsetS16((S16 *)DataIn,IndexIn,NumIn,(S16 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_U32)?mMinSubsetU32((U32 *)DataIn,IndexIn,NumIn,(U32 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_S32)?mMinSubsetS32((S32 *)DataIn,IndexIn,NumIn,(S32 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_U64)?mMinSubsetU64((U64 *)DataIn,IndexIn,NumIn,(U64 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_S64)?mMinSubsetS64((S64 *)DataIn,IndexIn,NumIn,(S64 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_F32)?mMinSubsetF32((F32 *)DataIn,IndexIn,NumIn,(F32 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_D64)?mMinSubsetD64((D64 *)DataIn,IndexIn,NumIn,(D64 *)DataOut,IndexOut,NumOut):DFLT)))))))))\
)

double mMaxSubsetU8 (U8  *data_in,int *index_in,int num_in,U8  *data_out,int *index_out,int num_out);
double mMaxSubsetS8 (S8  *data_in,int *index_in,int num_in,S8  *data_out,int *index_out,int num_out);
double mMaxSubsetU16(U16 *data_in,int *index_in,int num_in,U16 *data_out,int *index_out,int num_out);
double mMaxSubsetS16(S16 *data_in,int *index_in,int num_in,S16 *data_out,int *index_out,int num_out);
double mMaxSubsetU32(U32 *data_in,int *index_in,int num_in,U32 *data_out,int *index_out,int num_out);
double mMaxSubsetS32(S32 *data_in,int *index_in,int num_in,S32 *data_out,int *index_out,int num_out);
double mMaxSubsetU64(U64 *data_in,int *index_in,int num_in,U64 *data_out,int *index_out,int num_out);
double mMaxSubsetS64(S64 *data_in,int *index_in,int num_in,S64 *data_out,int *index_out,int num_out);
double mMaxSubsetF32(F32 *data_in,int *index_in,int num_in,F32 *data_out,int *index_out,int num_out);
double mMaxSubsetD64(D64 *data_in,int *index_in,int num_in,D64 *data_out,int *index_out,int num_out);
#define mMaxSubset(DataIn,IndexIn,NumIn,DataOut,IndexOut,NumOut) (\
 (mDataType(DataIn)==MORN_TYPE_U8 )?mMaxSubsetU8 ((U8  *)DataIn,IndexIn,NumIn,(U8  *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_S8 )?mMaxSubsetS8 ((S8  *)DataIn,IndexIn,NumIn,(S8  *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_U16)?mMaxSubsetU16((U16 *)DataIn,IndexIn,NumIn,(U16 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_S16)?mMaxSubsetS16((S16 *)DataIn,IndexIn,NumIn,(S16 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_U32)?mMaxSubsetU32((U32 *)DataIn,IndexIn,NumIn,(U32 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_S32)?mMaxSubsetS32((S32 *)DataIn,IndexIn,NumIn,(S32 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_U64)?mMaxSubsetU64((U64 *)DataIn,IndexIn,NumIn,(U64 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_S64)?mMaxSubsetS64((S64 *)DataIn,IndexIn,NumIn,(S64 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_F32)?mMaxSubsetF32((F32 *)DataIn,IndexIn,NumIn,(F32 *)DataOut,IndexOut,NumOut):(\
    (morn_data_type==MORN_TYPE_D64)?mMaxSubsetD64((D64 *)DataIn,IndexIn,NumIn,(D64 *)DataOut,IndexOut,NumOut):DFLT)))))))))\
)

#define MAX_TENSOR_BATCH 32

double mCaculate(char *str);

unsigned int mHash(const char *in,int size);

typedef struct MLInt
{
	uint32_t data[32];
	char sign;
	char len;
}MLInt;
void mS64ToLInt(int64_t in,MLInt *a);
int64_t mLIntToS64(MLInt *a);
int mLIntCompare(MLInt *a,MLInt *b);

void mLIntAddU32(MLInt *a,uint32_t b,MLInt *c);
void mLIntAddS32(MLInt *a,int b,MLInt *c);
void mLIntAdd(MLInt *a,MLInt *b,MLInt *c);

void mLIntSubU32(MLInt *a,uint32_t b,MLInt *c);
void mLIntSubS32(MLInt *a,int b,MLInt *c);
void mLIntSub(MLInt *a,MLInt *b,MLInt *c);

void mLIntMulU32(MLInt *a,uint32_t b,MLInt *c);
void mLIntMulS32(MLInt *a,int b,MLInt *c);
void mLIntMul(MLInt *a,MLInt *b,MLInt *c);

void mLIntDivU32(MLInt *a,uint32_t b,MLInt *c,int32_t *remainder);
void mLIntDivS32(MLInt *a,int b,MLInt *c,int *remainder);
void mLIntDiv(MLInt *a,MLInt *b,MLInt *c,MLInt *remainder);

void mLIntToString(MLInt *a,char *str);
void mStringToLInt(char *str,MLInt *a);

#ifdef __cplusplus
}
#endif

#endif
