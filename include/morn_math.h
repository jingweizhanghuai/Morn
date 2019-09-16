//////////////////////////////////////////////////////////
// Copyright (c) Beijing IMPower Technologies Co. Ltd.
// All rights reserved.
//////////////////////////////////////////////////////////

#ifndef _MORN_MATH_H_
#define _MORN_MATH_H_

#include "morn_Util.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MORN_DEFAULT -1

#define MORN_PI 3.14159265358979f

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
#define mVectorSet(Vec,Size) {\
    if(Vec == NULL)\
        Vec = mVectorCreate(Size,NULL);\
    else\
        mVectorRedefine(Vec,Size);\
}
    
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
#define mMatrixleSet(Mat,Row,Col) {\
    if(Mat == NULL)\
        Mat = mMatrixCreate(Row,Col,NULL);\
    else\
        mMatrixRedefine(Mat,Row,Col);\
}

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

// void mQuickSort(float *data,int num);
// void mQuickSortWithIndex(float *data,int *index,int num);

// void mAscendingBubbleSort(float *data_in,float *data_out,int num);
// void mAscendingQuickSort(float *data_in,float *data_out,int num);
// void mAscendingInsertionSort(float *data_in,float *data_out,int num);
// void mAscendingSort(float *data_in,float *data_out,int num,void (*method)(float *,float *,int));

// void mAscendingQuickSortIndex(float *data_in,int *index_in,float *data_out,int *index_out,int num);

float mMinSubset(float *data_in,int num_in,float *data_out,int num_out);
float mMinSubsetIndex(float *data_in,int *index_in,int num_in,float *data_out,int *index_out,int num_out);

        double mSequenceMinSubsetD64(MObject *proc,        double data_in,int index_in,        double *data_out,int *index_out,int num); 
         float mSequenceMinSubsetF32(MObject *proc,         float data_in,int index_in,         float *data_out,int *index_out,int num);
           int mSequenceMinSubsetS32(MObject *proc,           int data_in,int index_in,           int *data_out,int *index_out,int num);
  unsigned int mSequenceMinSubsetU32(MObject *proc,  unsigned int data_in,int index_in,  unsigned int *data_out,int *index_out,int num);
         short mSequenceMinSubsetS16(MObject *proc,         short data_in,int index_in,         short *data_out,int *index_out,int num);
unsigned short mSequenceMinSubsetU16(MObject *proc,unsigned short data_in,int index_in,unsigned short *data_out,int *index_out,int num);
          char mSequenceMinSubsetS8( MObject *proc,          char data_in,int index_in,          char *data_out,int *index_out,int num);
 unsigned char mSequenceMinSubsetU8( MObject *proc, unsigned char data_in,int index_in, unsigned char *data_out,int *index_out,int num);

        double mSequenceMaxSubsetD64(MObject *proc,        double data_in,int index_in,        double *data_out,int *index_out,int num);
         float mSequenceMaxSubsetF32(MObject *proc,         float data_in,int index_in,         float *data_out,int *index_out,int num);
           int mSequenceMaxSubsetS32(MObject *proc,           int data_in,int index_in,           int *data_out,int *index_out,int num);
  unsigned int mSequenceMaxSubsetU32(MObject *proc,  unsigned int data_in,int index_in,  unsigned int *data_out,int *index_out,int num);
         short mSequenceMaxSubsetS16(MObject *proc,         short data_in,int index_in,         short *data_out,int *index_out,int num);
unsigned short mSequenceMaxSubsetU16(MObject *proc,unsigned short data_in,int index_in,unsigned short *data_out,int *index_out,int num);
          char mSequenceMaxSubsetS8( MObject *proc,          char data_in,int index_in,          char *data_out,int *index_out,int num);
 unsigned char mSequenceMaxSubsetU8( MObject *proc, unsigned char data_in,int index_in, unsigned char *data_out,int *index_out,int num);


#define MAX_TENSOR_BATCH 32




unsigned int mHash(const char *in,int size);

#ifdef __cplusplus
}
#endif

#endif
