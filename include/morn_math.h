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

double mSin(float a);
double mCos(float a);
#define mTan(a) (mSin(a)/mCos(a))
#define mCot(a) (mCos(a)/mSin(a))
double mLn(float a);
#define mLoga(a,b) (mLn(b)/mLn(a))
#define mLg(a) (mLn(a)/2.302585092994050)
double mExp(double a);
#define mPow(A,B) (mExp((B)*mLn(A)))

void mMean(float *in,int num,float *sum,float *mean);
void mVariance(float *in,int num,float *mean,float *variance);
void mCovariance(float *in1,float *in2,int num,float *mean1,float *mean2,float *covariance);

int mBinaryCeil(int data);
int mBinaryFloor(int data);
int mBinaryRound(int data);

int GreatestCommonDivisor(int n,...);
#define mGCD(...) GreatestCommonDivisor(VANumber(__VA_ARGS__),__VA_ARGS__)
int LowestCommonMultiple(int n,...);
#define mLCM(...) LowestCommonMultiple(VANumber(__VA_ARGS__),__VA_ARGS__)

// static inline float mSup() {float a=0.0f;return  1.0f/a;}
// static inline float mInf() {float a=0.0f;return -1.0f/a;}
// #define mIsSup(A) (isinf(A)&&(A>0))
// #define mIsInf(A) (isinf(A)&&(A<0))

extern char morn_float_sup[8];// = {0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f};
#define mSup() (*((double *)morn_float_sup))
extern char morn_float_inf[8];// = {0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe};
#define mInf() (*((double *)morn_float_inf))

#ifdef NAN
#define mNan() NAN
#else
#define mNan() nan(NULL)
#endif

#define mIsSup(A) (A>=mSup())
#define mIsInf(A) (A<=mInf())
#define mIsNan(A) isnan(A)
#define mIsInteger(A) (ABS(A-((int)(A+0.5f)))<0.00001f)

int mPermutation(MArray *array,int n);
int mCombination(MArray *array,int n);

double mSigmoid(float x);

typedef struct MVector{
    int size;
    float *data;
}MVector;

#define INVALID_VEC(Vec) ((((Vec) ==NULL)||((intptr_t)(Vec) == -1))?1:(((Vec)->size <= 0)||((intptr_t)((Vec)->data) <= 0)))

MVector *VectorCreate(int size,float *data);
#define mVectorCreate(...) (\
    (VANumber(__VA_ARGS__)==0)?VectorCreate(DFLT,NULL):(\
    (VANumber(__VA_ARGS__)==1)?VectorCreate(VA0(__VA_ARGS__),NULL):(\
    (VANumber(__VA_ARGS__)==2)?VectorCreate(VA0(__VA_ARGS__),(float *)VA1(__VA_ARGS__)):(\
    NULL)))\
)

void VectorRedefine(MVector *vec,int size,float *data);
#define mVectorRedefine(Vec,...) do{\
    int N=VANumber(__VA_ARGS__);\
         if(N==1) VectorRedefine(Vec,VA0(__VA_ARGS__),(Vec)->data);\
    else if(N==2) VectorRedefine(Vec,VA0(__VA_ARGS__),(float *)VA1(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input para");\
}while(0)

void mVectorRelease(MVector *vec);

#define mVectorData(Vec,Data) do{\
    float *Buff;if(Vec->dev==MORN_HOST_CPU) Buff=Vec->data; else Buff=mMalloc(Vec->size*sizeof(float));\
    for(int I=0;I<Vec->size;I++) Buff[I]=(float)(Data[I]);\
    if(Buff!=Vec->data) {MemCopy(Vec->data,Vec->dev,Buff,MORN_HOST_CPU,Vec->size);mFree(Buff);}\
}while(0)
    
#define mVectorExchange(Vec1,Vec2) mObjectExchange(Vec1,Vec2,MVector)

typedef struct MMatrix{
    int row;
    int col;
    float **data;
}MMatrix;

#define INVALID_MAT(Mat) ((((Mat) ==NULL)||((intptr_t)(Mat) == -1))?1:(((Mat)->data == NULL)||((intptr_t)((Mat)->data) <= 0)\
                                                                ||((Mat)->col <= 0)\
                                                                ||((Mat)->row <= 0)))
void _PrintMat(char *name,MMatrix *mat);
#define PrintMat(Mat) _PrintMat(#Mat,Mat)

MMatrix * MatrixCreate(int row,int col,float **data);
#define mMatrixCreate(...) (\
    (VANumber(__VA_ARGS__)==0)?MatrixCreate(DFLT,DFLT,NULL):(\
    (VANumber(__VA_ARGS__)==2)?MatrixCreate(VA0(__VA_ARGS__),VA1(__VA_ARGS__),NULL):(\
    (VANumber(__VA_ARGS__)==3)?MatrixCreate(VA0(__VA_ARGS__),VA1(__VA_ARGS__),(float **)VA2(__VA_ARGS__)):(\
    NULL)))\
)

void mMatrixRelease(MMatrix *mat);

void MatrixRedefine(MMatrix *mat,int row,int col,float **data);
#define mMatrixRedefine(Mat,...) do{\
    int N=VANumber(__VA_ARGS__);\
         if(N==2) MatrixRedefine(Mat,VA0(__VA_ARGS__),VA1(__VA_ARGS__),(Mat)->data);\
    else if(N==3) MatrixRedefine(Mat,VA0(__VA_ARGS__),VA1(__VA_ARGS__),(float **)VA2(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input para");\
}while(0)

void m_UnitMatrix(MMatrix *mat,int size);
#define mUnitMatrix(...) do{\
    if(VANumber(__VA_ARGS__)==1) m_UnitMatrix(VA0(__VA_ARGS__),DFLT);\
    else                         m_UnitMatrix(VA0(__VA_ARGS__),VA1(__VA_ARGS__));\
}while(0)

#define mMatrixExchange(Mat1,Mat2) mObjectExchange(Mat1,Mat2,MMatrix)
void mMATWrite(MFile *file,MMatrix *mat,char *matname);
void mMATRead(MFile *file,char *matname,MMatrix *mat);

#define mMatrixData(Mat,Data) do{\
    int N=0;for(int J=0;J<Mat->row;J++)for(int I=0;I<Mat->col;I++) Mat->data[J][I]=Data[N++];\
    int Dev=MORN_HOST; mPropertyRead( Mat,"device",&Dev);\
    if(Dev!=MORN_HOST) mPropertyWrite(Mat,"device",&Dev);\
}while(0)

void mVectorAdd(MVector *vec1,MVector *vec2,MVector *dst);
float mVectorMul(MVector *vec1,MVector *vec2);
void mMatrixVectorMul(MMatrix *mat,MVector *vec,MVector *dst);
void mVectorMatrixMul(MVector *vec,MMatrix *mat,MVector *dst);
void m_MatrixMul(MMatrix *mat1,MMatrix *mat2,MMatrix *dst);
#define mMatrixMul(Mat,...) do{\
    if(VANumber(__VA_ARGS__)==2) m_MatrixMul(Mat,VA0(__VA_ARGS__),(MMatrix *)((intptr_t)VA1(__VA_ARGS__)));\
    else                         m_MatrixMul(Mat,VA0(__VA_ARGS__),Mat);\
}while(0)

void m_MatrixTranspose(MMatrix *mat,MMatrix *dst);
#define mMatrixTranspose(...) do{\
    if(VANumber(__VA_ARGS__)==2) m_MatrixTranspose(VA0(__VA_ARGS__),(MMatrix *)((intptr_t)VA1(__VA_ARGS__)));\
    else                         m_MatrixTranspose(VA0(__VA_ARGS__),VA0(__VA_ARGS__));\
}while(0)

float mMatrixDetValue(MMatrix *mat);
int mMatrixInverse(MMatrix *mat,MMatrix *inv);

float m_MatrixMaxElement(MMatrix *src,int *x,int *y);
float m_MatrixMinElement(MMatrix *src,int *x,int *y);
#define mMatrixMaxElement(...) ((VANumber(__VA_ARGS__)==1)?m_MatrixMaxElement(VA0(__VA_ARGS__),NULL,NULL):m_MatrixMaxElement(VA0(__VA_ARGS__),VA1(__VA_ARGS__),VA2(__VA_ARGS__)))
#define mMatrixMinElement(...) ((VANumber(__VA_ARGS__)==1)?m_MatrixMinElement(VA0(__VA_ARGS__),NULL,NULL):m_MatrixMinElement(VA0(__VA_ARGS__),VA1(__VA_ARGS__),VA2(__VA_ARGS__)))

void mMatrixEigenValue(MMatrix *src,MList *eigenvalue,MList *eigenvector);

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
#define m_AscSort(DataIn,IndexIn,DataOut,IndexOut,Num) do{\
    int Type=mDataType((DataIn)[0]);\
         if(Type==MORN_TYPE_U8 ) mAscSortU8 ((U8  *)(DataIn),IndexIn,(U8  *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S8 ) mAscSortS8 ((S8  *)(DataIn),IndexIn,(S8  *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U16) mAscSortU16((U16 *)(DataIn),IndexIn,(U16 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S16) mAscSortS16((S16 *)(DataIn),IndexIn,(S16 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U32) mAscSortU32((U32 *)(DataIn),IndexIn,(U32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S32) mAscSortS32((S32 *)(DataIn),IndexIn,(S32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U64) mAscSortU64((U64 *)(DataIn),IndexIn,(U64 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S64) mAscSortS64((S64 *)(DataIn),IndexIn,(S64 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_F32) mAscSortF32((F32 *)(DataIn),IndexIn,(F32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_D64) mAscSortD64((D64 *)(DataIn),IndexIn,(D64 *)DataOut,IndexOut,Num);\
    else mException(1,EXIT,"invalid input dD64ata type");\
}while(0)

#define mAscSort(...) do{\
    int VAN=VANumber(__VA_ARGS__);\
    intptr_t VA1=(intptr_t)VA1(__VA_ARGS__),VA2=(intptr_t)VA2(__VA_ARGS__),VA3=(intptr_t)VA3(__VA_ARGS__),VA4=(intptr_t)VA4(__VA_ARGS__);\
         if(VAN==2) m_AscSort(VA0(__VA_ARGS__),      NULL,       NULL,      NULL,(int)VA1);\
    else if(VAN==3) m_AscSort(VA0(__VA_ARGS__),      NULL,(void *)VA1,      NULL,(int)VA2);\
    else if(VAN==4) m_AscSort(VA0(__VA_ARGS__),      NULL,(void *)VA1,(int *)VA2,(int)VA3);\
    else if(VAN==5) m_AscSort(VA0(__VA_ARGS__),(int *)VA1,(void *)VA2,(int *)VA3,(int)VA4);\
    else mException(1,EXIT,"invalid input");\
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
#define m_DescSort(DataIn,IndexIn,DataOut,IndexOut,Num) do{\
    int Type=mDataType((DataIn)[0]);\
         if(Type==MORN_TYPE_U8 ) mDescSortU8 ((U8  *)(DataIn),IndexIn,(U8  *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S8 ) mDescSortS8 ((S8  *)(DataIn),IndexIn,(S8  *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U16) mDescSortU16((U16 *)(DataIn),IndexIn,(U16 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S16) mDescSortS16((S16 *)(DataIn),IndexIn,(S16 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U32) mDescSortU32((U32 *)(DataIn),IndexIn,(U32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S32) mDescSortS32((S32 *)(DataIn),IndexIn,(S32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_U64) mDescSortU64((U64 *)(DataIn),IndexIn,(U64 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_S64) mDescSortS64((S64 *)(DataIn),IndexIn,(S64 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_F32) mDescSortF32((F32 *)(DataIn),IndexIn,(F32 *)DataOut,IndexOut,Num);\
    else if(Type==MORN_TYPE_D64) mDescSortD64((D64 *)(DataIn),IndexIn,(D64 *)DataOut,IndexOut,Num);\
    else mException(1,EXIT,"invalid input data type");\
}while(0)
#define mDescSort(...) do{\
    int VAN=VANumber(__VA_ARGS__);\
    intptr_t VA1=(intptr_t)VA1(__VA_ARGS__),VA2=(intptr_t)VA2(__VA_ARGS__),VA3=(intptr_t)VA3(__VA_ARGS__),VA4=(intptr_t)VA4(__VA_ARGS__);\
         if(VAN==2) m_DescSort(VA0(__VA_ARGS__),      NULL,       NULL,      NULL,(int)VA1);\
    else if(VAN==3) m_DescSort(VA0(__VA_ARGS__),      NULL,(void *)VA1,      NULL,(int)VA2);\
    else if(VAN==4) m_DescSort(VA0(__VA_ARGS__),      NULL,(void *)VA1,(int *)VA2,(int)VA3);\
    else if(VAN==5) m_DescSort(VA0(__VA_ARGS__),(int *)VA1,(void *)VA2,(int *)VA3,(int)VA4);\
    else mException(1,EXIT,"invalid input");\
}while(0)

U8  mMinSubsetU8 (U8  *data_in,int *index_in,int num_in,U8  *data_out,int *index_out,int num_out);
S8  mMinSubsetS8 (S8  *data_in,int *index_in,int num_in,S8  *data_out,int *index_out,int num_out);
U16 mMinSubsetU16(U16 *data_in,int *index_in,int num_in,U16 *data_out,int *index_out,int num_out);
S16 mMinSubsetS16(S16 *data_in,int *index_in,int num_in,S16 *data_out,int *index_out,int num_out);
U32 mMinSubsetU32(U32 *data_in,int *index_in,int num_in,U32 *data_out,int *index_out,int num_out);
S32 mMinSubsetS32(S32 *data_in,int *index_in,int num_in,S32 *data_out,int *index_out,int num_out);
U64 mMinSubsetU64(U64 *data_in,int *index_in,int num_in,U64 *data_out,int *index_out,int num_out);
S64 mMinSubsetS64(S64 *data_in,int *index_in,int num_in,S64 *data_out,int *index_out,int num_out);
F32 mMinSubsetF32(F32 *data_in,int *index_in,int num_in,F32 *data_out,int *index_out,int num_out);
D64 mMinSubsetD64(D64 *data_in,int *index_in,int num_in,D64 *data_out,int *index_out,int num_out);
#define m_MinSubset(DataIn,IndexIn,NumIn,DataOut,IndexOut,NumOut) (\
(mDataType((DataIn)[0])==MORN_TYPE_U8 )?mMinSubsetU8 ((U8  *)(DataIn),IndexIn,NumIn,(U8  *)DataOut,IndexOut,NumOut):(\
        (morn_data_type==MORN_TYPE_S8 )?mMinSubsetS8 ((S8  *)(DataIn),IndexIn,NumIn,(S8  *)DataOut,IndexOut,NumOut):(\
        (morn_data_type==MORN_TYPE_U16)?mMinSubsetU16((U16 *)(DataIn),IndexIn,NumIn,(U16 *)DataOut,IndexOut,NumOut):(\
        (morn_data_type==MORN_TYPE_S16)?mMinSubsetS16((S16 *)(DataIn),IndexIn,NumIn,(S16 *)DataOut,IndexOut,NumOut):(\
        (morn_data_type==MORN_TYPE_U32)?mMinSubsetU32((U32 *)(DataIn),IndexIn,NumIn,(U32 *)DataOut,IndexOut,NumOut):(\
        (morn_data_type==MORN_TYPE_S32)?mMinSubsetS32((S32 *)(DataIn),IndexIn,NumIn,(S32 *)DataOut,IndexOut,NumOut):(\
        (morn_data_type==MORN_TYPE_U64)?mMinSubsetU64((U64 *)(DataIn),IndexIn,NumIn,(U64 *)DataOut,IndexOut,NumOut):(\
        (morn_data_type==MORN_TYPE_S64)?mMinSubsetS64((S64 *)(DataIn),IndexIn,NumIn,(S64 *)DataOut,IndexOut,NumOut):(\
        (morn_data_type==MORN_TYPE_F32)?mMinSubsetF32((F32 *)(DataIn),IndexIn,NumIn,(F32 *)DataOut,IndexOut,NumOut):(\
        (morn_data_type==MORN_TYPE_D64)?mMinSubsetD64((D64 *)(DataIn),IndexIn,NumIn,(D64 *)DataOut,IndexOut,NumOut):DFLT)))))))))\
)
#define mMinSubset(...) (\
    (VANumber(__VA_ARGS__)==3)?m_MinSubset(VA0(__VA_ARGS__),                                   NULL,(int)((intptr_t)VA1(__VA_ARGS__)),                          NULL,                                   NULL,(int)((intptr_t)VA2(__VA_ARGS__))):(\
    (VANumber(__VA_ARGS__)==4)?m_MinSubset(VA0(__VA_ARGS__),                                   NULL,(int)((intptr_t)VA1(__VA_ARGS__)),(intptr_t)VA2(__VA_ARGS__),                                   NULL,(int)((intptr_t)VA3(__VA_ARGS__))):(\
    (VANumber(__VA_ARGS__)==5)?m_MinSubset(VA0(__VA_ARGS__),                                   NULL,(int)((intptr_t)VA1(__VA_ARGS__)),(intptr_t)VA2(__VA_ARGS__),(int *)((intptr_t)VA3(__VA_ARGS__)),(int)((intptr_t)VA4(__VA_ARGS__))):(\
    (VANumber(__VA_ARGS__)==6)?m_MinSubset(VA0(__VA_ARGS__),(int *)((intptr_t)VA1(__VA_ARGS__)),(int)((intptr_t)VA2(__VA_ARGS__)),(intptr_t)VA3(__VA_ARGS__),(int *)((intptr_t)VA4(__VA_ARGS__)),(int)((intptr_t)VA5(__VA_ARGS__))): \
    DFLT)))\
)

U8  mMaxSubsetU8 (U8  *data_in,int *index_in,int num_in,U8  *data_out,int *index_out,int num_out);
S8  mMaxSubsetS8 (S8  *data_in,int *index_in,int num_in,S8  *data_out,int *index_out,int num_out);
U16 mMaxSubsetU16(U16 *data_in,int *index_in,int num_in,U16 *data_out,int *index_out,int num_out);
S16 mMaxSubsetS16(S16 *data_in,int *index_in,int num_in,S16 *data_out,int *index_out,int num_out);
U32 mMaxSubsetU32(U32 *data_in,int *index_in,int num_in,U32 *data_out,int *index_out,int num_out);
S32 mMaxSubsetS32(S32 *data_in,int *index_in,int num_in,S32 *data_out,int *index_out,int num_out);
U64 mMaxSubsetU64(U64 *data_in,int *index_in,int num_in,U64 *data_out,int *index_out,int num_out);
S64 mMaxSubsetS64(S64 *data_in,int *index_in,int num_in,S64 *data_out,int *index_out,int num_out);
F32 mMaxSubsetF32(F32 *data_in,int *index_in,int num_in,F32 *data_out,int *index_out,int num_out);
D64 mMaxSubsetD64(D64 *data_in,int *index_in,int num_in,D64 *data_out,int *index_out,int num_out);
#define m_MaxSubset(DataIn,IndexIn,NumIn,DataOut,IndexOut,NumOut) (\
(mDataType((DataIn)[0])==MORN_TYPE_U8 )?mMaxSubsetU8 ((U8  *)(DataIn),IndexIn,(int)NumIn,(U8  *)DataOut,IndexOut,(int)NumOut):(\
        (morn_data_type==MORN_TYPE_S8 )?mMaxSubsetS8 ((S8  *)(DataIn),IndexIn,(int)NumIn,(S8  *)DataOut,IndexOut,(int)NumOut):(\
        (morn_data_type==MORN_TYPE_U16)?mMaxSubsetU16((U16 *)(DataIn),IndexIn,(int)NumIn,(U16 *)DataOut,IndexOut,(int)NumOut):(\
        (morn_data_type==MORN_TYPE_S16)?mMaxSubsetS16((S16 *)(DataIn),IndexIn,(int)NumIn,(S16 *)DataOut,IndexOut,(int)NumOut):(\
        (morn_data_type==MORN_TYPE_U32)?mMaxSubsetU32((U32 *)(DataIn),IndexIn,(int)NumIn,(U32 *)DataOut,IndexOut,(int)NumOut):(\
        (morn_data_type==MORN_TYPE_S32)?mMaxSubsetS32((S32 *)(DataIn),IndexIn,(int)NumIn,(S32 *)DataOut,IndexOut,(int)NumOut):(\
        (morn_data_type==MORN_TYPE_U64)?mMaxSubsetU64((U64 *)(DataIn),IndexIn,(int)NumIn,(U64 *)DataOut,IndexOut,(int)NumOut):(\
        (morn_data_type==MORN_TYPE_S64)?mMaxSubsetS64((S64 *)(DataIn),IndexIn,(int)NumIn,(S64 *)DataOut,IndexOut,(int)NumOut):(\
        (morn_data_type==MORN_TYPE_F32)?mMaxSubsetF32((F32 *)(DataIn),IndexIn,(int)NumIn,(F32 *)DataOut,IndexOut,(int)NumOut):(\
        (morn_data_type==MORN_TYPE_D64)?mMaxSubsetD64((D64 *)(DataIn),IndexIn,(int)NumIn,(D64 *)DataOut,IndexOut,(int)NumOut):DFLT)))))))))\
)
#define mMaxSubset(...) (\
    (VANumber(__VA_ARGS__)==3)?m_MaxSubset(VA0(__VA_ARGS__),                                   NULL,(int)((intptr_t)VA1(__VA_ARGS__)),                          NULL,                                   NULL,(int)((intptr_t)VA2(__VA_ARGS__))):(\
    (VANumber(__VA_ARGS__)==4)?m_MaxSubset(VA0(__VA_ARGS__),                                   NULL,(int)((intptr_t)VA1(__VA_ARGS__)),(intptr_t)VA2(__VA_ARGS__),                                   NULL,(int)((intptr_t)VA3(__VA_ARGS__))):(\
    (VANumber(__VA_ARGS__)==5)?m_MaxSubset(VA0(__VA_ARGS__),                                   NULL,(int)((intptr_t)VA1(__VA_ARGS__)),(intptr_t)VA2(__VA_ARGS__),(int *)((intptr_t)VA3(__VA_ARGS__)),(int)((intptr_t)VA4(__VA_ARGS__))):(\
    (VANumber(__VA_ARGS__)==6)?m_MaxSubset(VA0(__VA_ARGS__),(int *)((intptr_t)VA1(__VA_ARGS__)),(int)((intptr_t)VA2(__VA_ARGS__)),(intptr_t)VA3(__VA_ARGS__),(int *)((intptr_t)VA4(__VA_ARGS__)),(int)((intptr_t)VA5(__VA_ARGS__))): \
    DFLT)))\
)

#define MAX_TENSOR_BATCH 32

double mCalculate(char *str);
void m_CalculateFunction(const char *name,void *func);
#define mCalculateFunction(Func,...) do{\
    int VA=VANumber(__VA_ARGS__);\
    if(VA==0) m_CalculateFunction(#Func,(void *)Func);\
    else m_CalculateFunction((const char*)Func,(void *)VA0(__VA_ARGS__));\
}while(0)

// unsigned int mHash(const char *in,int size);

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

void mLinearRegression(float **x,float *y,int n,int m,float *A);

void mPointInterpolation(MList *curve,MList *list,float k1,float k2);

#define MORN_NO_TRANS 0
#define MORN_TRANS    1
void mSgemm(int device,int a_trans,int b_trans,int m,int n,int k,float alpha,MMemoryBlock *a,int sa,MMemoryBlock *b,int sb,float beta,MMemoryBlock *c,int sc);
void mSaxpby(int device,int n,float alpha,MMemoryBlock *a,int sa,float beta,MMemoryBlock *b,int sb);

#ifdef __cplusplus
}
#endif

#endif
