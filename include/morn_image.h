/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#ifndef _MORN_IMAGE_H_
#define _MORN_IMAGE_H_

#include "morn_math.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MORN_MAX_IMAGE_CN 4

#define MORN_IMAGE_GRAY 1
#define MORN_IMAGE_RGB  3
#define MORN_IMAGE_RGBA 4
#define MORN_IMAGE_YUV  5
#define MORN_IMAGE_HSV  6
#define MORN_IMAGE_LAB  7

#define MORN_BORDER_UNDEFINED DFLT
#define MORN_BORDER_BLACK     0
#define MORN_BORDER_WHITE     1
#define MORN_BORDER_REPLICATE 2
#define MORN_BORDER_REFLECT   3
#define MORN_BORDER_IMAGE     4
#define MORN_BORDER_INVALID   5

typedef struct MImage {
    int channel;
    int height;
    int width;
    unsigned char **data[MORN_MAX_IMAGE_CN];
    MArray *border;
    Morn;
    void *reserve;
}MImage;

#define INVALID_IMAGE(Img) ((((Img) ==NULL)||((intptr_t)(Img) == DFLT))?1:(((Img)->data == NULL)||((intptr_t)((Img)->data) == DFLT)\
                                                                    ||((Img)->channel <= 0)\
                                                                    ||((Img)->height <= 0)||((Img)->width <= 0)\
                                                                    ||((Img)->handle == NULL)))

MImage *ImageCreate(int channel,int height,int width,unsigned char **data[]);
#define mImageCreate(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==0)?ImageCreate(DFLT,DFLT,DFLT,NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==2)?ImageCreate(DFLT,VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==3)?ImageCreate(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__),NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==4)?ImageCreate(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__),(unsigned char ***)VA_ARG3(__VA_ARGS__)):\
    NULL\
)
void ImageRedefine(MImage *img,int channel,int height,int width,unsigned char **data[]);
#define mImageRedefine(Img,...) do{\
    int N=VA_ARG_NUM(__VA_ARGS__);\
         if(N==2) ImageRedefine(Img,DFLT,VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),(Img)->data);\
    else if(N==3) ImageRedefine(Img,VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__),(Img)->data);\
    else if(N==4) ImageRedefine(Img,VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__),(unsigned char ***)VA_ARG3(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input para");\
}while(0)
void mImageRelease(MImage *img);
unsigned char ***mImageBackup(MImage *img,int cn,int height,int width);

void mImageExpand(MImage *img,int r,int expand_type);
void mImageCopy(MImage *src,MImage *dst);
MImage *mImageChannelSplit(MImage *src,int num,...);
void mImageCut(MImage *img,MImage *ROI,int src_x1,int src_x2,int src_y1,int src_y2,int dst_x,int dst_y);
void mImageWipe(MImage *img);

void mImageDiff(MImage *src1,MImage *src2,MImage *diff);
void mImageAdd(MImage *src1,MImage *src2,MImage *dst);
void mImageSub(MImage *src1,MImage *src2,MImage *dst);
void mImageInvert(MImage *src,MImage *dst);
void mImageDataAnd(MImage *src1,MImage *src2,MImage *dst);
void mImageDataOr(MImage *src1,MImage *src2,MImage *dst);
void mImageDataWeightAdd(MImage *src1,MImage *src2,MImage *dst,float w1,float w2);
void mImageLinearMap(MImage *src,MImage *dst,float k,float b);

void mImageOperate(MImage *src,MImage *dst,void (*func)(unsigned char *,unsigned char *,void *),void *para);

#define MORN_IMAGE_STREAM_HWC DFLT
#define MORN_IMAGE_STREAM_HCW 1
#define MORN_IMAGE_STREAM_CHW 2
#define MORN_IMAGE_STREAM_CWH 3
#define MORN_IMAGE_STREAM_WHC 4
#define MORN_IMAGE_STREAM_WCH 5

void mImageDataInputU8 (MImage *img,U8  *stream,int stream_type,void *func,void *para);
void mImageDataInputS8 (MImage *img,S8  *stream,int stream_type,void *func,void *para);
void mImageDataInputU16(MImage *img,U16 *stream,int stream_type,void *func,void *para);
void mImageDataInputS16(MImage *img,S16 *stream,int stream_type,void *func,void *para);
void mImageDataInputU32(MImage *img,U32 *stream,int stream_type,void *func,void *para);
void mImageDataInputS32(MImage *img,S32 *stream,int stream_type,void *func,void *para);
void mImageDataInputF32(MImage *img,F32 *stream,int stream_type,void *func,void *para);
void mImageDataInputS64(MImage *img,S64 *stream,int stream_type,void *func,void *para);
void mImageDataInputU64(MImage *img,U64 *stream,int stream_type,void *func,void *para);
void mImageDataInputD64(MImage *img,D64 *stream,int stream_type,void *func,void *para);
#define m_ImageDataInput(Img,Stream,Stream_type,Func,Para) do{\
    int data_type = mDataType(Stream);\
         if(data_type==MORN_TYPE_U8 ) mImageDataInputU8 (img,(U8  *)stream,stream_type,func,para);\
    else if(data_type==MORN_TYPE_S8 ) mImageDataInputS8 (img,(S8  *)stream,stream_type,func,para);\
    else if(data_type==MORN_TYPE_U16) mImageDataInputU16(img,(U16 *)stream,stream_type,func,para);\
    else if(data_type==MORN_TYPE_S16) mImageDataInputS16(img,(S16 *)stream,stream_type,func,para);\
    else if(data_type==MORN_TYPE_U32) mImageDataInputU32(img,(U32 *)stream,stream_type,func,para);\
    else if(data_type==MORN_TYPE_S32) mImageDataInputS32(img,(S32 *)stream,stream_type,func,para);\
    else if(data_type==MORN_TYPE_F32) mImageDataInputF32(img,(F32 *)stream,stream_type,func,para);\
    else if(data_type==MORN_TYPE_S64) mImageDataInputS64(img,(S64 *)stream,stream_type,func,para);\
    else if(data_type==MORN_TYPE_U64) mImageDataInputU64(img,(U64 *)stream,stream_type,func,para);\
    else if(data_type==MORN_TYPE_D64) mImageDataInputD64(img,(D64 *)stream,stream_type,func,para);\
}while(0)
#define mImageDataInput(Img,Stream,...) do{\
    int VAN=VA_ARG_NUM(__VA_ARGS__);\
    intptr_t VA1 = (intptr_t)VA_ARG0(__VA_ARGS__),VA2=(intptr_t)VA_ARG1(__VA_ARGS__),VA3=(intptr_t)VA_ARG2(__VA_ARGS__);\
         if(VAN==0)           m_ImageDataInput(Img,Stream,    DFLT,       NULL,       NULL); \
    else if(VAN==1){if(VA1<6) m_ImageDataInput(Img,Stream,(int)VA1,       NULL,       NULL); \
                    else      m_ImageDataInput(Img,Stream,    DFLT,(void *)VA1,       NULL);}\
    else if(VAN==2){if(VA1<6) m_ImageDataInput(Img,Stream,(int)VA1,(void *)VA2,       NULL); \
                    else      m_ImageDataInput(Img,Stream,    DFLT,(void *)VA1,(void *)VA3);}\
    else if(VAN==3)           m_ImageDataInput(Img,Stream,(int)VA1,(void *)VA2,(void *)VA3); \
    else mException(1,EXIT,"invalid input");\
}while(0)

void mImageDataOutputU8 (MImage *img,U8  *stream,int stream_type,void *func,void *para);
void mImageDataOutputS8 (MImage *img,S8  *stream,int stream_type,void *func,void *para);
void mImageDataOutputU16(MImage *img,U16 *stream,int stream_type,void *func,void *para);
void mImageDataOutputS16(MImage *img,S16 *stream,int stream_type,void *func,void *para);
void mImageDataOutputU32(MImage *img,U32 *stream,int stream_type,void *func,void *para);
void mImageDataOutputS32(MImage *img,S32 *stream,int stream_type,void *func,void *para);
void mImageDataOutputF32(MImage *img,F32 *stream,int stream_type,void *func,void *para);
void mImageDataOutputS64(MImage *img,S64 *stream,int stream_type,void *func,void *para);
void mImageDataOutputU64(MImage *img,U64 *stream,int stream_type,void *func,void *para);
void mImageDataOutputD64(MImage *img,D64 *stream,int stream_type,void *func,void *para);
#define m_ImageDataOutput(Img,Stream,Stream_type,Func,Para) do{\
    int data_type = mDataType(Stream);\
         if(data_type==MORN_TYPE_U8 ) mImageDataOutputU8 (Img,(U8  *)Stream,Stream_type,Func,Para);\
    else if(data_type==MORN_TYPE_S8 ) mImageDataOutputS8 (Img,(S8  *)Stream,Stream_type,Func,Para);\
    else if(data_type==MORN_TYPE_U16) mImageDataOutputU16(Img,(U16 *)Stream,Stream_type,Func,Para);\
    else if(data_type==MORN_TYPE_S16) mImageDataOutputS16(Img,(S16 *)Stream,Stream_type,Func,Para);\
    else if(data_type==MORN_TYPE_U32) mImageDataOutputU32(Img,(U32 *)Stream,Stream_type,Func,Para);\
    else if(data_type==MORN_TYPE_S32) mImageDataOutputS32(Img,(S32 *)Stream,Stream_type,Func,Para);\
    else if(data_type==MORN_TYPE_F32) mImageDataOutputF32(Img,(F32 *)Stream,Stream_type,Func,Para);\
    else if(data_type==MORN_TYPE_S64) mImageDataOutputS64(Img,(S64 *)Stream,Stream_type,Func,Para);\
    else if(data_type==MORN_TYPE_U64) mImageDataOutputU64(Img,(U64 *)Stream,Stream_type,Func,Para);\
    else if(data_type==MORN_TYPE_D64) mImageDataOutputD64(Img,(D64 *)Stream,Stream_type,Func,Para);\
}while(0)
#define mImageDataOutput(Img,Stream,...) do{\
    int VAN=VA_ARG_NUM(__VA_ARGS__);\
    intptr_t VA1 = (intptr_t)VA_ARG0(__VA_ARGS__),VA2=(intptr_t)VA_ARG1(__VA_ARGS__),VA3=(intptr_t)VA_ARG2(__VA_ARGS__);\
         if(VAN==0)           m_ImageDataOutput(Img,Stream,    DFLT,       NULL,       NULL); \
    else if(VAN==1){if(VA1<6) m_ImageDataOutput(Img,Stream,(int)VA1,       NULL,       NULL); \
                    else      m_ImageDataOutput(Img,Stream,    DFLT,(void *)VA1,       NULL);}\
    else if(VAN==2){if(VA1<6) m_ImageDataOutput(Img,Stream,(int)VA1,(void *)VA2,       NULL); \
                    else      m_ImageDataOutput(Img,Stream,    DFLT,(void *)VA1,(void *)VA3);}\
    else if(VAN==3)           m_ImageDataOutput(Img,Stream,(int)VA1,(void *)VA2,(void *)VA3); \
    else mException(1,EXIT,"invalid input");\
}while(0)


void m_ImageRGBToYUV(MImage *src,MImage *dst);
#define mImageRGBToYUV(...) ((VA_ARG_NUM(__VA_ARGS__)==2)?m_ImageRGBToYUV(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__)):m_ImageRGBToYUV(VA_ARG0(__VA_ARGS__),NULL))
void m_ImageYUVToRGB(MImage *src,MImage *dst);
#define mImageYUVToRGB(...) ((VA_ARG_NUM(__VA_ARGS__)==2)?m_ImageYUVToRGB(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__)):m_ImageYUVToRGB(VA_ARG0(__VA_ARGS__),NULL))
void m_ImageRGBToHSV(MImage *src,MImage *dst);
#define mImageRGBToHSV(...) ((VA_ARG_NUM(__VA_ARGS__)==2)?m_ImageRGBToHSV(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__)):m_ImageRGBToHSV(VA_ARG0(__VA_ARGS__),NULL))
void m_ImageHSVToRGB(MImage *src,MImage *dst);
#define mImageHSVToRGB(...) ((VA_ARG_NUM(__VA_ARGS__)==2)?m_ImageHSVToRGB(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__)):m_ImageHSVToRGB(__VA_ARGS__),NULL))
void m_ImageRGBToGray(MImage *src,MImage *dst);
#define mImageRGBToGray(...) ((VA_ARG_NUM(__VA_ARGS__)==2)?m_ImageRGBToGray(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__)):m_ImageRGBToGray(VA_ARG0(__VA_ARGS__),NULL))
void m_ImageYUVToGray(MImage *src,MImage *dst);
#define mImageYUVToGray(...) ((VA_ARG_NUM(__VA_ARGS__)==2)?m_ImageYUVToGray(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__)):m_ImageYUVToGray(VA_ARG0(__VA_ARGS__),NULL))
void m_ImageToGray(MImage *src,MImage *dst);
#define mImageToGray(...) ((VA_ARG_NUM(__VA_ARGS__)==2)?m_ImageToGray(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__)):m_ImageToGray(VA_ARG0(__VA_ARGS__),NULL))

#define mImageExchange(Src,Dst) mObjectExchange(Src,Dst,MImage)
#define mImageReset(Img) mHandleReset(Img->handle)

typedef struct MImageRect
{
    int x1;
    int y1;
    int x2;
    int y2;
}MImageRect;
#define mRect(Rect,X1,Y1,X2,Y2) do{(Rect)->x1=MIN(X1,X2);(Rect)->y1=MIN(Y1,Y2);(Rect)->x2=MAX(X1,X2);(Rect)->y2=MAX(Y1,Y2);}while(0)
#define mRectHeight(Rect) ((Rect)->y2-(Rect)->y1)
#define mRectWidth(Rect)  ((Rect)->x2-(Rect)->x1)

typedef struct MImagePoint
{
    float x;
    float y;
}MImagePoint;
#define mPoint(Point,X,Y) do{(Point)->x=X; (Point)->y=Y;}while(0)

typedef struct MImageCircle
{
    MImagePoint center;
    float r;
}MImageCircle;
#define mCircle(Circle,Cx,Cy,R) do{(Circle)->center.x=Cx;(Circle)->center.y=Cy;(Circle)->r=R;}while(0)

#define MORN_CURVE_X -1
#define MORN_CURVE_Y  1
typedef struct MImageCurve
{
    MImagePoint v1;
    MImagePoint v2;
    int type;
    float (*curve)(float,float *);
    float para[16];
}MImageCurve;
void mCurve(MImageCurve *curve,float i1,float i2,int type,float (*func)(float,float *),float *para);
float mCurvePoint(MImageCurve *curve,float x);

void mBMPSave(MImage *img,const char *filename);
void mBMPLoad(MImage *img,const char *filename);
#if defined MORN_USE_JPEG
void mJPGSave(MImage *img,const char *filename);
void mJPGLoad(MImage *img,const char *filename);
#endif
#if defined MORN_USE_PNG
void mPNGSave(MImage *img,const char *filename);
void mPNGLoad(MImage *img,const char *filename);
#endif
void ImageLoad(MImage *img,const char *filename);
void ImageSave(MImage *img,const char *filename);
#define mImageLoad(Img,...) do{sprintf(morn_filename,__VA_ARGS__);ImageLoad(Img,morn_filename);}while(0)
#define mImageSave(Img,...) do{sprintf(morn_filename,__VA_ARGS__);ImageSave(Img,morn_filename);}while(0)

#define MORN_RESIZE_UNUNIFORM        DFLT
#define MORN_RESIZE_MINUNIFORM 0xFFFFFFFE
#define MORN_RESIZE_MAXUNIFORM 0xFFFFFFFD
#define MORN_INTERPOLATE             DFLT
#define MORN_NEAREST           0xFFFFFFEF
void m_ImageResize(MImage *src,MImage *dst,int height,int width,int type);
#define mImageResize(src,...) {\
         if(VA_ARG_NUM(__VA_ARGS__)==1) m_ImageResize(src,(MImage *)VA_ARG0(__VA_ARGS__),DFLT,DFLT,DFLT);\
    else if(VA_ARG_NUM(__VA_ARGS__)==2)\
    {\
        if((int)VA_ARG1(__VA_ARGS__)<0)\
            m_ImageResize(src,(MImage *)VA_ARG0(__VA_ARGS__),DFLT,DFLT,VA_ARG1(__VA_ARGS__));\
        else\
            m_ImageResize(src,NULL,(intptr_t)VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),DFLT);\
    }\
    else if(VA_ARG_NUM(__VA_ARGS__)==3)\
    {\
        if((int)VA_ARG2(__VA_ARGS__)<0)\
            m_ImageResize(src,NULL,(intptr_t)VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__));\
        else\
            m_ImageResize(src,(MImage *)VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__),DFLT);\
    }\
    else if(VA_ARG_NUM(__VA_ARGS__)==4) m_ImageResize(src,(MImage *)VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__),VA_ARG3(__VA_ARGS__));\
    else mException(1,EXIT,"invalid operate for image resize");\
}

void m_ImageDrawPoint (MImage *src,MImage *dst,MImagePoint *point,             unsigned char *color,int width);
void m_ImageDrawRect  (MImage *src,MImage *dst,MImageRect *rect,               unsigned char *color,int width);
void m_ImageDrawLine  (MImage *src,MImage *dst,MImagePoint *p1,MImagePoint *p2,unsigned char *color,int width);
void m_ImageDrawShape (MImage *src,MImage *dst,MList *shape,                   unsigned char *color,int width);
void m_ImageDrawCircle(MImage *src,MImage *dst,MImageCircle *circle,           unsigned char *color,int width);
void m_ImageDrawCurve (MImage *src,MImage *dst,MImageCurve *curve,             unsigned char *color,int width);
#define mImageDrawPoint(Img,...) do{\
    int VAN = VA_ARG_NUM(__VA_ARGS__);intptr_t W;\
         if(VAN==1) m_ImageDrawPoint(Img,Img,(MImagePoint *)VA_ARG0(__VA_ARGS__),NULL,DFLT);\
    else if(VAN==2)\
    {\
        W=(intptr_t)VA_ARG1(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))m_ImageDrawPoint(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),NULL,DFLT);\
        else if((W<=4)&&(W>=DFLT))                         m_ImageDrawPoint(Img,Img,(MImagePoint *)VA_ARG0(__VA_ARGS__),NULL,(int)W);\
        else                                               m_ImageDrawPoint(Img,Img,(MImagePoint *)VA_ARG0(__VA_ARGS__),(unsigned char *)VA_ARG1(__VA_ARGS__),DFLT);\
    }\
    else if(VAN==3)\
    {\
        W=(intptr_t)VA_ARG2(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))\
        {\
            if((W<=4)&&(W>=DFLT))m_ImageDrawPoint(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),NULL,(int)W);\
            else                 m_ImageDrawPoint(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),DFLT);\
        }\
        else                     m_ImageDrawPoint(Img,Img,(MImagePoint *)VA_ARG0(__VA_ARGS__),(unsigned char *)VA_ARG1(__VA_ARGS__),(int)W);\
    }\
    else if(VAN==4) m_ImageDrawPoint(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),(int)VA_ARG3(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input");\
}while(0)
#define mImageDrawRect(Img,...) do{\
    int VAN = VA_ARG_NUM(__VA_ARGS__);intptr_t W;\
         if(VAN==1) m_ImageDrawRect(Img,Img,(MImageRect *)VA_ARG0(__VA_ARGS__),NULL,DFLT);\
    else if(VAN==2)\
    {\
        W=(intptr_t)VA_ARG1(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))m_ImageDrawRect(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageRect *)VA_ARG1(__VA_ARGS__),NULL,DFLT);\
        else if((W<=4)&&(W>=DFLT))                       m_ImageDrawRect(Img,Img,(MImageRect *)VA_ARG0(__VA_ARGS__),NULL,(int)W);\
        else                                             m_ImageDrawRect(Img,Img,(MImageRect *)VA_ARG0(__VA_ARGS__),(unsigned char *)VA_ARG1(__VA_ARGS__),DFLT);\
    }\
    else if(VAN==3)\
    {\
        W=(intptr_t)VA_ARG2(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))\
        {\
            if((W<=4)&&(W>=DFLT))m_ImageDrawRect(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageRect *)VA_ARG1(__VA_ARGS__),NULL,(int)W);\
            else                 m_ImageDrawRect(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageRect *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),DFLT);\
        }\
        else                     m_ImageDrawRect(Img,Img,(MImageRect *)VA_ARG0(__VA_ARGS__),(unsigned char *)VA_ARG1(__VA_ARGS__),(int)W);\
    }\
    else if(VAN==4) m_ImageDrawRect(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageRect *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),(int)VA_ARG3(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input");\
}while(0)
#define mImageDrawLine(Img,...) do{\
    int VAN = VA_ARG_NUM(__VA_ARGS__);intptr_t W;\
         if(VAN==2) m_ImageDrawLine(Img,Img,(MImagePoint *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),NULL,DFLT);\
    else if(VAN==3)\
    {\
        W=(intptr_t)VA_ARG2(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))m_ImageDrawLine(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),(MImagePoint *)VA_ARG2(__VA_ARGS__),NULL,DFLT);\
        else if((W<=4)&&(W>=DFLT))                       m_ImageDrawLine(Img,Img,(MImagePoint *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),NULL,(int)W);\
        else                                             m_ImageDrawLine(Img,Img,(MImagePoint *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),DFLT);\
    }\
    else if(VAN==4)\
    {\
        W=(intptr_t)VA_ARG3(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))\
        {\
            if((W<=4)&&(W>=DFLT))m_ImageDrawLine(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),(MImagePoint *)VA_ARG2(__VA_ARGS__),NULL,(int)W);\
            else                 m_ImageDrawLine(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),(MImagePoint *)VA_ARG2(__VA_ARGS__),(unsigned char *)VA_ARG3(__VA_ARGS__),DFLT);\
        }\
        else                     m_ImageDrawLine(Img,Img,(MImagePoint *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),(int)W);\
    }\
    else if(VAN==5) m_ImageDrawLine(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImagePoint *)VA_ARG1(__VA_ARGS__),(MImagePoint *)VA_ARG2(__VA_ARGS__),(unsigned char *)VA_ARG3(__VA_ARGS__),(int)VA_ARG4(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input");\
}while(0)
#define mImageDrawShape(Img,...) do{\
    int VAN = VA_ARG_NUM(__VA_ARGS__);intptr_t W;\
         if(VAN==1) m_ImageDrawShape(Img,Img,(MList *)VA_ARG0(__VA_ARGS__),NULL,DFLT);\
    else if(VAN==2)\
    {\
        W=(intptr_t)VA_ARG1(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))m_ImageDrawShape(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MList *)VA_ARG1(__VA_ARGS__),NULL,DFLT);\
        else if((W<=4)&&(W>=DFLT))                       m_ImageDrawShape(Img,Img,(MList *)VA_ARG0(__VA_ARGS__),NULL,(int)W);\
        else                                             m_ImageDrawShape(Img,Img,(MList *)VA_ARG0(__VA_ARGS__),(unsigned char *)VA_ARG1(__VA_ARGS__),DFLT);\
    }\
    else if(VAN==3)\
    {\
        W=(intptr_t)VA_ARG2(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))\
        {\
            if((W<=4)&&(W>=DFLT))m_ImageDrawShape(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MList *)VA_ARG1(__VA_ARGS__),NULL,(int)W);\
            else                 m_ImageDrawShape(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MList *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),DFLT);\
        }\
        else                     m_ImageDrawShape(Img,Img,(MList *)VA_ARG0(__VA_ARGS__),(unsigned char *)VA_ARG1(__VA_ARGS__),(int)W);\
    }\
    else if(VAN==4) m_ImageDrawShape(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MList *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),(int)VA_ARG3(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input");\
}while(0)
#define mImageDrawCircle(Img,...) do{\
    int VAN = VA_ARG_NUM(__VA_ARGS__);intptr_t W;\
         if(VAN==1) m_ImageDrawCircle(Img,Img,(MImageCircle *)VA_ARG0(__VA_ARGS__),NULL,DFLT);\
    else if(VAN==2)\
    {\
        W=(intptr_t)VA_ARG1(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))m_ImageDrawCircle(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageCircle *)VA_ARG1(__VA_ARGS__),NULL,DFLT);\
        else if((W<=4)&&(W>=DFLT))                       m_ImageDrawCircle(Img,Img,(MImageCircle *)VA_ARG0(__VA_ARGS__),NULL,(int)W);\
        else                                             m_ImageDrawCircle(Img,Img,(MImageCircle *)VA_ARG0(__VA_ARGS__),(unsigned char *)VA_ARG1(__VA_ARGS__),DFLT);\
    }\
    else if(VAN==3)\
    {\
        W=(intptr_t)VA_ARG2(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))\
        {\
            if((W<=4)&&(W>=DFLT))m_ImageDrawCircle(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageCircle *)VA_ARG1(__VA_ARGS__),NULL,(int)W);\
            else                 m_ImageDrawCircle(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageCircle *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),DFLT);\
        }\
        else                     m_ImageDrawCircle(Img,Img,(MImageCircle *)VA_ARG0(__VA_ARGS__),(unsigned char *)VA_ARG1(__VA_ARGS__),(int)W);\
    }\
    else if(VAN==4) m_ImageDrawCircle(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageCircle *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),(int)VA_ARG3(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input");\
}while(0)
#define mImageDrawCurve(Img,...) do{\
    int VAN = VA_ARG_NUM(__VA_ARGS__);intptr_t W;\
         if(VAN==1) m_ImageDrawCurve(Img,Img,(MImageCurve *)VA_ARG0(__VA_ARGS__),NULL,DFLT);\
    else if(VAN==2)\
    {\
        W=(intptr_t)VA_ARG1(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))m_ImageDrawCurve(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageCurve *)VA_ARG1(__VA_ARGS__),NULL,DFLT);\
        else if((W<=4)&&(W>=DFLT))                       m_ImageDrawCurve(Img,Img,(MImageCurve *)VA_ARG0(__VA_ARGS__),NULL,(int)W);\
        else                                             m_ImageDrawCurve(Img,Img,(MImageCurve *)VA_ARG0(__VA_ARGS__),(unsigned char *)VA_ARG1(__VA_ARGS__),DFLT);\
    }\
    else if(VAN==3)\
    {\
        W=(intptr_t)VA_ARG2(__VA_ARGS__);\
        if(sizeof(*(VA_ARG0(__VA_ARGS__)))==sizeof(MImage))\
        {\
            if((W<=4)&&(W>=DFLT))m_ImageDrawCurve(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageCurve *)VA_ARG1(__VA_ARGS__),NULL,(int)W);\
            else                 m_ImageDrawCurve(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageCurve *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),DFLT);\
        }\
        else                     m_ImageDrawCurve(Img,Img,(MImageCurve *)VA_ARG0(__VA_ARGS__),(unsigned char *)VA_ARG1(__VA_ARGS__),(int)W);\
    }\
    else if(VAN==4) m_ImageDrawCurve(Img,(MImage *)VA_ARG0(__VA_ARGS__),(MImageCurve *)VA_ARG1(__VA_ARGS__),(unsigned char *)VA_ARG2(__VA_ARGS__),(int)VA_ARG3(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input");\
}while(0)

void mLineTravel(MImagePoint *p1,MImagePoint *p2,int stride,void (*func)(MImagePoint *,void *),void *para);
void mPolygonSideTravel(MList *polygon,int stride,void (*func)(MImagePoint *,void *),void *para);
void mCurveTravel(MImageCurve *curve,int stride,void (*func)(MImagePoint *,void *),void *para);

#define PointLineCheck(px,py,lx1,ly1,lx2,ly2) (((lx1)-(lx2))*((py)-(ly2))-((ly1)-(ly2))*((px)-(lx2)))
int mLinePointCheck(MList *line,MImagePoint *point);
int LineCrossCheck(double l1x1,double l1y1,double l1x2,double l1y2,double l2x1,double l2y1,double l2x2,double l2y2);
int mLineRectCrossCheck(MImagePoint *ls,MImagePoint *le,MImageRect *rect);
int LineCrossPoint(double l1x1,double l1y1,double l1x2,double l1y2,double l2x1,double l2y1,double l2x2,double l2y2,float *px,float *py);
int mLineCross(MImagePoint *l1s,MImagePoint *l1e,MImagePoint *l2s,MImagePoint *l2e,MImagePoint *point);

float mLineAngle(MImagePoint *l1s,MImagePoint *l1e,MImagePoint *l2s,MImagePoint *l2e);
void _PolygonSetup(MList *polygon,int num,float x0,float y0,float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,float x5,float y5,float x6,float y6,float x7,float y7);
#define mPolygon(Polygon,...) _PolygonSetup(Polygon,VA_ARG_NUM(__VA_ARGS__)/2,VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__),VA_ARG3(__VA_ARGS__),VA_ARG4(__VA_ARGS__),VA_ARG5(__VA_ARGS__),VA_ARG6(__VA_ARGS__),VA_ARG7(__VA_ARGS__),VA_ARG8(__VA_ARGS__),VA_ARG9(__VA_ARGS__),VA_ARG10(__VA_ARGS__),VA_ARG11(__VA_ARGS__),VA_ARG12(__VA_ARGS__),VA_ARG13(__VA_ARGS__),VA_ARG14(__VA_ARGS__),VA_ARG15(__VA_ARGS__))
#define mRectArea(Rect) ((((MImageRect *)(Rect))->x2-((MImageRect *)(Rect))->x1)*(((MImageRect *)(Rect))->y2-((MImageRect *)(Rect))->y1))
float mPolygonArea(MList *polygon);
float TriangleArea(float x1,float y1,float x2,float y2,float x3,float y3);
float mTriangleArea(MImagePoint *p1,MImagePoint *p2,MImagePoint *p3);
#define mCircleArea(Circle) (((MImageCircle *)(Circle))->r*((MImageCircle *)(Circle))->r*MORN_PI)
float QuadrangleArea(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4);
float mQuadrangleArea(MImagePoint *p1,MImagePoint *p2,MImagePoint *p3,MImagePoint *p4);
#define PointDistance(X1,Y1,X2,Y2) ((float)sqrt(((X1)-(X2))*((X1)-(X2))+((Y1)-(Y2))*((Y1)-(Y2))))
#define mPointDistance(P1,P2) PointDistance(((MImagePoint *)(P1))->x,((MImagePoint *)(P1))->y,((MImagePoint *)(P2))->x,((MImagePoint *)(P2))->y)
double PointVerticalDistance(double px,double py,double lx1,double ly1,double lx2,double ly2,float *vx,float *vy);
float mPointVerticalDistance(MImagePoint *point,MImagePoint *p1,MImagePoint *p2,MImagePoint *pedal);
#define mPointInRect(Point,Rect) ((((MImagePoint *)(Point))->x>((MImageRect *)(Rect))->x1)&&(((MImagePoint *)(Point))->x<((MImageRect *)(Rect))->x2)&&(((MImagePoint *)(Point))->y>((MImageRect *)(Rect))->y1)&&(((MImagePoint *)(Point))->y<((MImageRect *)(Rect))->y2));
int PointInPolygon(double x,double y,MList *polygon);
int mPointInPolygon(MImagePoint *point,MList *polygon);
float mRectUnionsetArea(MImageRect *rect1,MImageRect *rect2);
float mRectIntersetArea(MImageRect *rect1,MImageRect *rect2);
float mCircleIntersetArea(MImageCircle *circle1,MImageCircle *circle2);
float mPolygonIntersetArea(MList *polygon1,MList *polygon2);
int PolygonCross(MList *polygon1,MList *polygon2);
int mPolygonCrossCheck(MList *polygon1,MList *polygon2);
#define mRectCrossCheck(Rect1,Rect2) ((MIN(Rect1->x2,Rect2->x2)>MAX(Rect1->x1,Rect2->x1))&&(MIN(Rect1->y2,Rect2->y2)>MAX(Rect1->y1,Rect2->y1)))
int mPolygonConcaveCheck(MList *polygon);
void mShapeBounding(MList *shape,MList *bounding);

void mConvexHull(MList *point,MList *polygon);

void ImagePolygonBorder(MArray *border,int height,int width,MList *polygon);
#define mImagePolygonBorder(Border,Height,Width,...) do{\
    MList *Polygon=ListCreate(DFLT,NULL);\
    mPolygon(Polygon,__VA_ARGS__);\
    ImagePolygonBorder(Border,Height,Width,Polygon);\
    mListRelease(Polygon);\
}while(0)

void mImageRectBorder(MArray *border,int height,int width,int x1,int x2,int y1,int y2);
#define ImageY1(Img) (((Img)->border==NULL)?0:(int)(((Img)->border)->info.value[0]))
#define ImageY2(Img) (((Img)->border==NULL)?((Img)->height):(int)(((Img)->border)->info.value[1]))
#define ImageX1(Img,n) (((Img)->border==NULL)?0:(((Img)->border)->dataS16[n+n]))
#define ImageX2(Img,n) (((Img)->border==NULL)?((Img)->width):(((Img)->border)->dataS16[n+n+1]))


void mImageBinaryEdge(MImage *src,MSheet *edge,MList *rect);
void mEdgeBoundary(MList *edge,MList *polygon,int thresh);
int mImageBinaryArea(MImage *src,MList *list,MImagePoint *point);
void mImageBinaryDenoise(MImage *src,MImage *dst,int num_thresh);
void mImageBinaryBurrRemove(MImage *src,MImage *dst);

void ImageIntegration(MImage *src,MTable **dst);
void m_ImageIntegration(MImage *src,MTable **sum,MTable **sqsum);
#define mImageIntegration(src,...) do{\
    int NPara=VA_ARG_NUM(__VA_ARGS__);\
         if(NPara==1)   ImageIntegration(src,VA_ARG0(__VA_ARGS__));\
    else if(NPara==2) m_ImageIntegration(src,VA_ARG0(__VA_ARGS__),(MTable **)VA_ARG1(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input");\
}while(0)
#define mImageIntegrationSum(Tab,X1,X2,Y1,Y2) (Tab->dataS32[(Y2)+1][(X2)+1]+Tab->dataS32[Y1][X1]-Tab->dataS32[(Y2)+1][X1]-Tab->dataS32[Y1][(X2)+1])

void mVideoBackground(MImage *src,MImage *bgd,int time_thresh,int diff_thresh);

void mImageMeanFilter(MImage *src,MImage *dst,int r);
void mImageMidValueFilter(MImage *src,MImage *dst);
void mImageBinaryFilter(MImage *src,MImage *dst,int r,float threshold1,float threshold2);

void mImageDirection(MImage *src,MImage *dst,int r,int thresh);
void mImageDirectionGradient(MImage *src,MImage *dst,int direction,int r,int thresh);
void mImageGradientValue(MImage *src,MImage *dst,int r,int thresh);
void mImageGradientCheck(MImage *src,MImage *dst,int r,int thresh);
void mImageGradientAdaptCheck(MImage *src,MImage *dst,int r,float thresh);
void mImageGradient(MImage *src,MImage *direction,MImage *value,int r,int thresh);
void mImageGradientFilter(MImage *dir,MImage *value,MImage *ddst,MImage *vdst,int r);
void mImageGradientSuppression(MImage *dir,MImage *value,MImage *dst,int r);
void mImageCanny(MImage *src,MImage *dst,int r,int thresh);

void mImageRotate(MImage *src,MImage *dst,MImagePoint *src_hold,MImagePoint *dst_hold,float angle,int mode);
void mImageCoordinateTransform(MImage *src,MImage *dst,float (*x_func)(int,int,void *),float (*y_func)(int,int,void *),void *para,int mode);
void mImageAffineCorrection(MImage *src,MImage *dst,MImagePoint *ps,MImagePoint *pd,int mode);
void mImagePerspectiveCorrection(MImage *src,MImage *dst,MImagePoint *ps,MImagePoint *pd,int mode);
void mImageLensTemplate(MObject *temp,float k,int r);
void mImageTemplateTransform(MImage *src,MImage *dst,MObject *temp,int x,int y,int mode);

int mColorCluster(MImage *src,MImage *dst,MList *list,int r,int thresh);

void mImageDilation(MImage *src,MImage *dst,int r,int stride);
void mImageErosion(MImage *src,MImage *dst,int r,int stride);

void mUSBCamera(MImage *img,const char *camera_name);

#ifdef __cplusplus
}
#endif

#endif
