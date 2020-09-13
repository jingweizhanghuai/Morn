/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#include "morn_image.h"

#if defined _MSC_VER
#define mPragma(InFo) __pragma(InFo)
#elif defined __GNUC__
#define mPragma(InFo) _Pragma(#InFo)
#else
#define mPragma(InFo)
#endif

#define DATA_INPUT(Type,c1,c2,c3,d1,d2,d3,img_data) {\
    int thread = MIN(omp_get_max_threads(),d1);\
    if(Func==NULL)\
    {\
        mPragma(omp parallel for num_threads(thread))\
        for(c1=0;c1<d1;c1++)\
            for(c2=0;c2<d2;c2++)\
            {\
                Type *s = Stream + c1*d2*d3+c2*d3;\
                for(c3=0;c3<d3;c3++)\
                    img_data = (unsigned char)s[c3];\
            }\
    }\
    else\
    {\
        for(c1=0;c1<d1;c1++)\
            for(c2=0;c2<d2;c2++)\
            {\
                Type *s = Stream + c1*d2*d3+c2*d3;\
                for(c3=0;c3<d3;c3++)\
                    img_data = Func(s[c3],para);\
            }\
    }\
}


#define ImageDataInput(Type,img,data_stream,stream_type,function,para) {\
    mException(INVALID_IMAGE(img),EXIT,"invalid input image");\
    unsigned char (*Func)(Type,void *) = function;\
    Type *Stream = (Type *)(data_stream);\
    int height = img->height;int width = img->width;int cn = img->channel;\
    int h,w,c;\
         if(stream_type == MORN_IMAGE_STREAM_HWC) {DATA_INPUT(Type,h,w,c,height,width ,cn    ,img->data[c][h][w]);}\
    else if(stream_type == MORN_IMAGE_STREAM_HCW) {DATA_INPUT(Type,h,c,w,height,cn    ,width ,img->data[c][h][w]);}\
    else if(stream_type == MORN_IMAGE_STREAM_CHW) {DATA_INPUT(Type,c,h,w,cn    ,height,width ,img->data[c][h][w]);}\
    else if(stream_type == MORN_IMAGE_STREAM_CWH) {DATA_INPUT(Type,c,w,h,cn    ,width ,height,img->data[c][h][w]);}\
    else if(stream_type == MORN_IMAGE_STREAM_WHC) {DATA_INPUT(Type,w,h,c,width ,height,cn    ,img->data[c][h][w]);}\
    else if(stream_type == MORN_IMAGE_STREAM_WCH) {DATA_INPUT(Type,w,c,h,width ,cn    ,height,img->data[c][h][w]);}\
    else mException(1,EXIT,"invalid operate");\
}

void mImageDataInputU8 (MImage *img,U8  *stream,int stream_type,void *func,void *para) {ImageDataInput(U8 ,img,stream,stream_type,func,para);}
void mImageDataInputS8 (MImage *img,S8  *stream,int stream_type,void *func,void *para) {ImageDataInput(S8 ,img,stream,stream_type,func,para);}
void mImageDataInputU16(MImage *img,U16 *stream,int stream_type,void *func,void *para) {ImageDataInput(U16,img,stream,stream_type,func,para);}
void mImageDataInputS16(MImage *img,S16 *stream,int stream_type,void *func,void *para) {ImageDataInput(S16,img,stream,stream_type,func,para);}
void mImageDataInputS32(MImage *img,S32 *stream,int stream_type,void *func,void *para) {ImageDataInput(S32,img,stream,stream_type,func,para);}
void mImageDataInputU32(MImage *img,U32 *stream,int stream_type,void *func,void *para) {ImageDataInput(U32,img,stream,stream_type,func,para);}
void mImageDataInputF32(MImage *img,F32 *stream,int stream_type,void *func,void *para) {ImageDataInput(F32,img,stream,stream_type,func,para);}
void mImageDataInputS64(MImage *img,S64 *stream,int stream_type,void *func,void *para) {ImageDataInput(S64,img,stream,stream_type,func,para);}
void mImageDataInputU64(MImage *img,U64 *stream,int stream_type,void *func,void *para) {ImageDataInput(U64,img,stream,stream_type,func,para);}
void mImageDataInputD64(MImage *img,D64 *stream,int stream_type,void *func,void *para) {ImageDataInput(D64,img,stream,stream_type,func,para);}

#define DATA_OUTPUT(c1,c2,c3,d1,d2,d3,img_data) {\
    if(Func==NULL)\
    {\
        for(c1=0;c1<d1;c1++)\
            for(c2=0;c2<d2;c2++)\
            {\
                for(c3=0;c3<d3;c3++)\
                    Stream[c3] = img_data;\
                Stream = Stream+d3;\
            }\
    }\
    else\
    {\
        for(c1=0;c1<d1;c1++)\
            for(c2=0;c2<d2;c2++)\
            {\
                for(c3=0;c3<d3;c3++)\
                    Stream[c3] = Func(img_data,para);\
                Stream = Stream+d3;\
            }\
    }\
}

#define ImageDataOutput(Type,img,data_stream,stream_type,function,para) {\
    mException(INVALID_IMAGE(img),EXIT,"invalid input image");\
    Type (*Func)(unsigned char,void *) = function;\
    Type *Stream = (Type *)(data_stream);\
    int height = img->height;int width = img->width;int cn = img->channel;\
    int h,w,c;\
         if(stream_type == MORN_IMAGE_STREAM_HWC) DATA_OUTPUT(h,w,c,height,width ,cn    ,img->data[c][h][w])\
    else if(stream_type == MORN_IMAGE_STREAM_HCW) DATA_OUTPUT(h,c,w,height,cn    ,width ,img->data[c][h][w])\
    else if(stream_type == MORN_IMAGE_STREAM_CHW) DATA_OUTPUT(c,h,w,cn    ,height,width ,img->data[c][h][w])\
    else if(stream_type == MORN_IMAGE_STREAM_CWH) DATA_OUTPUT(c,w,h,cn    ,width ,height,img->data[c][h][w])\
    else if(stream_type == MORN_IMAGE_STREAM_WHC) DATA_OUTPUT(w,h,c,width ,height,cn    ,img->data[c][h][w])\
    else if(stream_type == MORN_IMAGE_STREAM_WCH) DATA_OUTPUT(w,c,h,width ,cn    ,height,img->data[c][h][w])\
    else mException(1,EXIT,"invalid operate");\
}

void mImageDataOutputU8 (MImage *img,U8  *stream,int stream_type,void *func,void *para) {ImageDataOutput(U8 ,img,stream,stream_type,func,para);}
void mImageDataOutputS8 (MImage *img,S8  *stream,int stream_type,void *func,void *para) {ImageDataOutput(S8 ,img,stream,stream_type,func,para);}
void mImageDataOutputU16(MImage *img,U16 *stream,int stream_type,void *func,void *para) {ImageDataOutput(U16,img,stream,stream_type,func,para);}
void mImageDataOutputS16(MImage *img,S16 *stream,int stream_type,void *func,void *para) {ImageDataOutput(S16,img,stream,stream_type,func,para);}
void mImageDataOutputS32(MImage *img,S32 *stream,int stream_type,void *func,void *para) {ImageDataOutput(S32,img,stream,stream_type,func,para);}
void mImageDataOutputU32(MImage *img,U32 *stream,int stream_type,void *func,void *para) {ImageDataOutput(U32,img,stream,stream_type,func,para);}
void mImageDataOutputF32(MImage *img,F32 *stream,int stream_type,void *func,void *para) {ImageDataOutput(F32,img,stream,stream_type,func,para);}
void mImageDataOutputS64(MImage *img,S64 *stream,int stream_type,void *func,void *para) {ImageDataOutput(S64,img,stream,stream_type,func,para);}
void mImageDataOutputU64(MImage *img,U64 *stream,int stream_type,void *func,void *para) {ImageDataOutput(U64,img,stream,stream_type,func,para);}
void mImageDataOutputD64(MImage *img,D64 *stream,int stream_type,void *func,void *para) {ImageDataOutput(D64,img,stream,stream_type,func,para);}
