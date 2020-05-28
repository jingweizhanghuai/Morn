/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_image.h"
#define SRC(CN,X,Y) ((src)->data[CN][Y][X])

void TransformGrid(MImage *src,float (*x_func)(int,int,void *),float (*y_func)(int,int,void *),void *para,MTable *gridx,MTable *gridy,MTable *w)
{
    int i, j;
    int dst_height = w->row;
    int dst_width = w->col;
    // printf("dst_height=%d,dst_width=%d\n",dst_height,dst_width);

    // #pragma omp parallel for
    for(j=0;j<dst_height;j++)for(i=0;i<dst_width;i++)
    {
        float ly = y_func(i,j,para);
        float lx = x_func(i,j,para);
        
        if(lx > 0.00001f) lx -= 0.00001f;
        if(ly > 0.00001f) ly -= 0.00001f;

        short x_locate = (short)lx;
        short y_locate = (short)ly;

        if((y_locate<ImageY1(src))||(y_locate>=ImageY2(src)-1)||(x_locate<ImageX1(src,y_locate))||(lx>=ImageX2(src,y_locate)-1))
        {
            gridx->dataS16[j][i] = DFLT;
            gridy->dataS16[j][i] = DFLT;
            continue;
        }

        gridx->dataS16[j][i] = x_locate;
        gridy->dataS16[j][i] = y_locate;
        
        x_locate = 15-(short)((lx-(float)x_locate)*15.0f+0.5f);
        y_locate = 15-(short)((ly-(float)y_locate)*15.0f+0.5f);

        w->dataU8[j][i] = (x_locate<<4)+y_locate;
    }
}

void GridInterpolation(MImage *src,MImage *dst,MTable *gridx,MTable *gridy,MTable *w,int mode)
{
	int i, j;
    int height = dst->height;
    int width = dst->width;
    unsigned char **s0=src->data[0];unsigned char **s1=src->data[1];unsigned char **s2=src->data[2];unsigned char **s3=src->data[3];
    unsigned char **d0=dst->data[0];unsigned char **d1=dst->data[1];unsigned char **d2=dst->data[2];unsigned char **d3=dst->data[3];

    mode = ((mode | MORN_NEAREST) == MORN_NEAREST);
        
    #define INTERPOLATION_CACL0(C) do{\
        d##C[j][i]=((s##C[y][x]*wx+s##C[y][x+1]*(15-wx))*wy+(s##C[y+1][x]*wx+s##C[y+1][x+1]*(15-wx))*(15-wy)+112)/225;\
    }while(0)
    #define INTERPOLATION_CACL1(C) do{\
        d##C[j][i]=s##C[(wy<8)?y+1:y][(wx<8)?x+1:x];\
    }while(0)
    
    // printf("src->channel=%d,height=%d,width=%d\n",src->channel,height,width);
    if((src->channel==1)&&(!mode))
    {
        // #pragma omp parallel for
        for(j=0;j<height;j++)for(i=0;i<width;i++)
        {
            int x = gridx->dataS16[j][i];  if(x<0) {d0[j][i] = 0; continue;}  \
            int y = gridy->dataS16[j][i];/*if(y<0) {d0[j][i] = 0; continue;}*/\
            int wx=(w->dataU8[j][i]>>  4);int wy=(w->dataU8[j][i]&0x0F);\
            INTERPOLATION_CACL0(0);
        }
        return;
    }
    if((src->channel==1)&&(mode))
    {
        // #pragma omp parallel for
        for(j=0;j<height;j++)for(i=0;i<width;i++)
        {
            int x = gridx->dataS16[j][i];  if(x<0) {d0[j][i] = 0; continue;}  \
            int y = gridy->dataS16[j][i];/*if(y<0) {d0[j][i] = 0; continue;}*/\
            int wx=(w->dataU8[j][i]>>  4);int wy=(w->dataU8[j][i]&0x0F);\
            INTERPOLATION_CACL1(0);
        }
        return;
    }
    if((src->channel==2)&&(!mode))
    {
        // #pragma omp parallel for
        for(j=0;j<height;j++)for(i=0;i<width;i++)
        {
            int x = gridx->dataS16[j][i];  if(x<0) {d0[j][i] = 0;d1[j][i] = 0; continue;}  \
            int y = gridy->dataS16[j][i];/*if(y<0) {d0[j][i] = 0;d1[j][i] = 0; continue;}*/\
            int wx=(w->dataU8[j][i]>>  4);int wy=(w->dataU8[j][i]&0x0F);\
            INTERPOLATION_CACL0(0);INTERPOLATION_CACL0(1);
        }
        return;
    }
    if((src->channel==2)&&(mode))
    {
        // #pragma omp parallel for
        for(j=0;j<height;j++)for(i=0;i<width;i++)
        {
            int x = gridx->dataS16[j][i];  if(x<0) {d0[j][i] = 0;d1[j][i] = 0; continue;}  \
            int y = gridy->dataS16[j][i];/*if(y<0) {d0[j][i] = 0;d1[j][i] = 0; continue;}*/\
            int wx=(w->dataU8[j][i]>>  4);int wy=(w->dataU8[j][i]&0x0F);\
            INTERPOLATION_CACL1(0);INTERPOLATION_CACL1(1);
        }
        return;
    }
    if((src->channel==3)&&(!mode))
    {
        // #pragma omp parallel for
        for(j=0;j<height;j++)for(i=0;i<width;i++)
        {
            int x = gridx->dataS16[j][i];  if(x<0) {d0[j][i] = 0;d1[j][i] = 0;d2[j][i] = 0; continue;}
            int y = gridy->dataS16[j][i];/*if(y<0) {d0[j][i] = 0;d1[j][i] = 0;d2[j][i] = 0; continue;}*/
            int wx=(w->dataU8[j][i]>>  4);int wy=(w->dataU8[j][i]&0x0F);
            INTERPOLATION_CACL0(0);INTERPOLATION_CACL0(1);INTERPOLATION_CACL0(2);
        }
        return;
    }
    if((src->channel==3)&&(mode))
    {
        // #pragma omp parallel for
        for(j=0;j<height;j++)for(i=0;i<width;i++)
        {
            int x = gridx->dataS16[j][i];  if(x<0) {d0[j][i] = 0;d1[j][i] = 0;d2[j][i] = 0; continue;}
            int y = gridy->dataS16[j][i];/*if(y<0) {d0[j][i] = 0;d1[j][i] = 0;d2[j][i] = 0; continue;}*/
            int wx=(w->dataU8[j][i]>>  4);int wy=(w->dataU8[j][i]&0x0F);
            INTERPOLATION_CACL1(0);INTERPOLATION_CACL1(1);INTERPOLATION_CACL1(2);
        }
        return;
    }
    if((src->channel==4)&&(!mode))
    {
        // #pragma omp parallel for
        for(j=0;j<height;j++)for(i=0;i<width;i++)
        {
            int x = gridx->dataS16[j][i];  if(x<0) {d0[j][i] = 0;d1[j][i] = 0;d2[j][i] = 0;d3[j][i] = 0; continue;}
            int y = gridy->dataS16[j][i];/*if(y<0) {d0[j][i] = 0;d1[j][i] = 0;d2[j][i] = 0;d3[j][i] = 0; continue;}*/
            int wx=(w->dataU8[j][i]>>  4);int wy=(w->dataU8[j][i]&0x0F);
            INTERPOLATION_CACL0(0);INTERPOLATION_CACL0(1);INTERPOLATION_CACL0(2);INTERPOLATION_CACL0(3);
        }
        return;
    }
    if((src->channel==4)&&(mode))
    {
        // #pragma omp parallel for
        for(j=0;j<height;j++)for(i=0;i<width;i++)
        {
            int x = gridx->dataS16[j][i];  if(x<0) {d0[j][i] = 0;d1[j][i] = 0;d2[j][i] = 0;d3[j][i] = 0; continue;}
            int y = gridy->dataS16[j][i];/*if(y<0) {d0[j][i] = 0;d1[j][i] = 0;d2[j][i] = 0;d3[j][i] = 0; continue;}*/
            int wx=(w->dataU8[j][i]>>  4);int wy=(w->dataU8[j][i]&0x0F);
            INTERPOLATION_CACL1(0);INTERPOLATION_CACL1(1);INTERPOLATION_CACL1(2);INTERPOLATION_CACL1(3);
        }
        return;
    }
}

struct HandleImageCoordinateTransform
{
    int height;
    int width;
    float (*x_func)(int,int,void *);
    float (*y_func)(int,int,void *);
    MTable *lx;
    MTable *ly;
    MTable *w;
};

void endImageCoordinateTransform(void *handle)
{
    struct HandleImageCoordinateTransform *info = (struct HandleImageCoordinateTransform *)handle;

    if(info->lx != NULL) mTableRelease(info->lx);
    if(info->lx != NULL) mTableRelease(info->ly);
    if(info->w  != NULL) mTableRelease(info->w );
}

#define HASH_ImageCoordinateTransform 0x5f44c7bc

void mImageCoordinateTransform(MImage *src,MImage *dst,float (*x_func)(int,int,void *),float (*y_func)(int,int,void *),void *para,int mode)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    
    MImage *p=dst;
    if(INVALID_POINTER(dst)||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    int width;  if(dst->width <=0) width = src->width;  else width = dst->width;
    int height; if(dst->height<=0) height= src->height; else height= dst->height;
    mImageRedefine(dst,src->channel,height,width,dst->data);
    
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));

    MHandle *hdl=mHandle(src,ImageCoordinateTransform);
    struct HandleImageCoordinateTransform *handle = (struct HandleImageCoordinateTransform *)(hdl->handle);
    if((hdl->valid == 0)||(handle->height!=height)||(handle->width!=width)||(handle->x_func!=x_func)||(handle->y_func!=y_func))
    {
        handle->height = height;
        handle->width = width;
        handle->x_func = x_func;
        handle->y_func = y_func;
        
        if(handle->lx==NULL) handle->lx=mTableCreate(height,width,S16,NULL);
        else               mTableRedefine(handle->lx,height,width,S16,NULL);
        if(handle->ly==NULL) handle->ly=mTableCreate(height,width,S16,NULL);
        else               mTableRedefine(handle->ly,height,width,S16,NULL);
        if(handle->w ==NULL) handle->w =mTableCreate(height,width, U8,NULL);
        else               mTableRedefine(handle->w ,height,width, U8,NULL);
        
        TransformGrid(src,x_func,y_func,para,handle->lx,handle->ly,handle->w);

        hdl->valid = 1;
    }

    GridInterpolation(src,dst,handle->lx,handle->ly,handle->w,mode);
    
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
}

void PerspectivePara(MImagePoint *ps,MImagePoint *pd,float *para)
{
    MMatrix *mat = mMatrixCreate(8,9,NULL,DFLT);
    for(int n=0,j=0;n<4;n=n+1,j=j+2)
    {
        mat->data[j][0] = pd[n].x;               mat->data[j+1][0] = 0.0f;
        mat->data[j][1] = pd[n].y;               mat->data[j+1][1] = 0.0f;
        mat->data[j][2] = 1.0f;                  mat->data[j+1][2] = 0.0f;
        mat->data[j][3] = 0.0f;                  mat->data[j+1][3] = pd[n].x;
        mat->data[j][4] = 0.0f;                  mat->data[j+1][4] = pd[n].y;
        mat->data[j][5] = 0.0f;                  mat->data[j+1][5] = 1.0f;
        mat->data[j][6] = 0.0f-pd[n].x*ps[n].x;  mat->data[j+1][6] = 0.0f-pd[n].x*ps[n].y;
        mat->data[j][7] = 0.0f-pd[n].y*ps[n].x;  mat->data[j+1][7] = 0.0f-pd[n].y*ps[n].y;
        mat->data[j][8] = 0.0f-ps[n].x;          mat->data[j+1][8] = 0.0f-ps[n].y;
    }
    mLinearEquation(mat,para);
    mMatrixRelease(mat);
}

float Perspective_x(int u,int v,void *para)
{
    float *p = (float *)para;
    return ((p[0]*u+p[1]*v+p[2])/(p[6]*u+p[7]*v+1.0f));
}

float Perspective_y(int u,int v,void *para)
{
    float *p = (float *)para;
    return ((p[3]*u+p[4]*v+p[5])/(p[6]*u+p[7]*v+1.0f));
}

struct HandleImagePerspectiveCorrection
{
    int height;
    int width;
    MImagePoint ps[4];
    MImagePoint pd[4];
    
    MTable *lx;
    MTable *ly;
    MTable *w;
};

void endImagePerspectiveCorrection(void *handle)
{
    struct HandleImagePerspectiveCorrection *info = (struct HandleImagePerspectiveCorrection *)handle;

    if(info->lx != NULL) mTableRelease(info->lx);
    if(info->lx != NULL) mTableRelease(info->ly);
    if(info->w  != NULL) mTableRelease(info->w );
}

#define HASH_ImagePerspectiveCorrection 0xdd819d48

void mImagePerspectiveCorrection(MImage *src,MImage *dst,MImagePoint *ps,MImagePoint *pd,int mode)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid source image");
    
    MImage *p = dst;
    if(INVALID_POINTER(dst)||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else if((dst->height<=0)||(dst->width<=0))
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    else
        mImageRedefine(dst,src->channel,DFLT,DFLT,dst->data);
    
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    int height = dst->height;
    int width = dst->width;
    
    MHandle *hdl=mHandle(src,ImagePerspectiveCorrection);
    struct HandleImagePerspectiveCorrection *handle = (struct HandleImagePerspectiveCorrection *)(hdl->handle);
    if((hdl->valid == 0)
     ||(memcmp(ps,handle->ps,4*sizeof(MImagePoint))!=0)
     ||(memcmp(pd,handle->pd,4*sizeof(MImagePoint))!=0)
     ||(handle->height != height)
     ||(handle->width  != width))
    {
        float para[8];
        PerspectivePara(ps,pd,para);
        
        // MMatrix *mat = mMatrixCreate(8,9,NULL);
        // for(int n=0,j=0;n<4;n=n+1,j=j+2)
        // {
            // mat->data[j][0] = pd[n].x;               mat->data[j+1][0] = 0.0f;
            // mat->data[j][1] = pd[n].y;               mat->data[j+1][1] = 0.0f;
            // mat->data[j][2] = 1.0f;                  mat->data[j+1][2] = 0.0f;
            // mat->data[j][3] = 0.0f;                  mat->data[j+1][3] = pd[n].x;
            // mat->data[j][4] = 0.0f;                  mat->data[j+1][4] = pd[n].y;
            // mat->data[j][5] = 0.0f;                  mat->data[j+1][5] = 1.0f;
            // mat->data[j][6] = 0.0f-pd[n].x*ps[n].x;  mat->data[j+1][6] = 0.0f-pd[n].x*ps[n].y;
            // mat->data[j][7] = 0.0f-pd[n].y*ps[n].x;  mat->data[j+1][7] = 0.0f-pd[n].y*ps[n].y;
            // mat->data[j][8] = 0.0f-ps[n].x;          mat->data[j+1][8] = 0.0f-ps[n].y;
        // }
        // mLinearEquation(mat,para);
        // mMatrixRelease(mat);
        
        if(handle->lx==NULL) handle->lx=mTableCreate(height,width,S16,NULL);
        else               mTableRedefine(handle->lx,height,width,S16,NULL);
        if(handle->ly==NULL) handle->ly=mTableCreate(height,width,S16,NULL);
        else               mTableRedefine(handle->ly,height,width,S16,NULL);
        if(handle->w ==NULL) handle->w =mTableCreate(height,width, U8,NULL);
        else               mTableRedefine(handle->w ,height,width, U8,NULL);
        
        TransformGrid(src,Perspective_x,Perspective_y,para,handle->lx,handle->ly,handle->w);

        memcpy(handle->ps,ps,4*sizeof(MImagePoint));
        memcpy(handle->pd,pd,4*sizeof(MImagePoint));
        handle->height = height;
        handle->width  = width;
        
        hdl->valid = 1;
    }
    
    GridInterpolation(src,dst,handle->lx,handle->ly,handle->w,mode);
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
}

float Affine_x(int u,int v,void *para)
{
    float *p = (float *)para;
    return (p[0]*u+p[1]*v+p[2]);
}

float Affine_y(int u,int v,void *para)
{
    float *p = (float *)para;
    return (p[3]*u+p[4]*v+p[5]);
}

struct HandleImageAffineCorrection
{
    int height;
    int width;
    MImagePoint ps[3];
    MImagePoint pd[3];
    
    MTable *lx;
    MTable *ly;
    MTable *w;
};

void endImageAffineCorrection(void *handle)
{
    struct HandleImageAffineCorrection *info = (struct HandleImageAffineCorrection *)handle;

    if(info->lx != NULL) mTableRelease(info->lx);
    if(info->lx != NULL) mTableRelease(info->ly);
    if(info->w  != NULL) mTableRelease(info->w );
}

#define HASH_ImageAffineCorrection 0x1670806b

void mImageAffineCorrection(MImage *src,MImage *dst,MImagePoint *ps,MImagePoint *pd,int mode)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid source image");
    
    
    MImage *p = dst;
    if(INVALID_POINTER(dst)||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else if((dst->height<=0)||(dst->width<=0))
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    else
        mImageRedefine(dst,src->channel,DFLT,DFLT,dst->data);
    
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    int height = dst->height;
    int width = dst->width;
    
    MHandle *hdl=mHandle(src,ImageAffineCorrection);
    struct HandleImageAffineCorrection *handle = (struct HandleImageAffineCorrection *)(hdl->handle);
    if((hdl->valid == 0)
     ||(memcmp(ps,handle->ps,3*sizeof(MImagePoint))!=0)
     ||(memcmp(pd,handle->pd,3*sizeof(MImagePoint))!=0)
     ||(handle->height != height)
     ||(handle->width  != width))
    {
        float para[6];
        float a1 = pd[1].x - pd[0].x;
        float a2 = pd[2].x - pd[0].x;
        float b1 = pd[1].y - pd[0].y;
        float b2 = pd[2].y - pd[0].y;
        float c = (a1*b2 -a2*b1);
        mException((c==0.0f),EXIT,"invalid achor point");
        
        float c1,c2;
        
        c1 = ps[1].x-ps[0].x;
        c2 = ps[2].x-ps[0].x;
        para[0] = (c1*b2 - c2*b1)/c;
        para[1] = (c2*a1 - c1*a2)/c;
        para[2] = ps[0].x - (para[0]*pd[0].x + para[1]*pd[0].y);

        c1 = ps[1].y-ps[0].y;
        c2 = ps[2].y-ps[0].y;
        para[3] = (c1*b2 - c2*b1)/c;
        para[4] = (c2*a1 - c1*a2)/c;
        para[5] = ps[0].y - (para[3]*pd[0].x + para[4]*pd[0].y);
        
        if(handle->lx==NULL) handle->lx=mTableCreate(height,width,S16,NULL);
        else mTableRedefine(handle->lx,height,width,S16,NULL);
        if(handle->ly==NULL) handle->ly=mTableCreate(height,width,S16,NULL);
        else  mTableRedefine(handle->ly,height,width,S16,NULL);
        if(handle->w ==NULL) handle->w =mTableCreate(height,width,U8 ,NULL);
        else  mTableRedefine(handle->w ,height,width, U8,NULL);
        
        TransformGrid(src,Affine_x,Affine_y,para,handle->lx,handle->ly,handle->w);

        memcpy(handle->ps,ps,3*sizeof(MImagePoint));
        memcpy(handle->pd,pd,3*sizeof(MImagePoint));
        handle->height = height;
        handle->width  = width;

        hdl->valid = 1;
    }
    GridInterpolation(src,dst,handle->lx,handle->ly,handle->w,mode);
    
    
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
}

void ImageRotate90(MImage *src,MImage *dst)
{
    int i,j,cn;
    mImageRedefine(dst,src->channel,src->width,src->height,dst->data);
    int height = dst->height-1;
    int width = dst->width-1;
    for(cn=0;cn<dst->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        for(j=0;j<=height;j++)
            for(i=0;i<=width;i++)
                ddata[j][i] = sdata[width-i][j];
    }
}
void ImageRotate180(MImage *src,MImage *dst)
{
    int i,j,cn;
    mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    int height = dst->height-1;
    int width = dst->width-1;
    for(cn=0;cn<dst->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        for(j=0;j<=height;j++)
            for(i=0;i<=width;i++)
                ddata[j][i] = sdata[height-j][width-i];
    }
}
void ImageRotate270(MImage *src,MImage *dst)
{
    int i,j,cn;
    mImageRedefine(dst,src->channel,src->width,src->height,dst->data);
    int height = dst->height-1;
    int width = dst->width-1;
    for(cn=0;cn<dst->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        for(j=0;j<=height;j++)
            for(i=0;i<=width;i++)
                ddata[j][i] = sdata[i][height-j];
    }
}


struct HandleImageRotate
{
    int height;
    int width;
    MImagePoint src_hold;
    MImagePoint dst_hold;
    float angle;
    
    MTable *lx;
    MTable *ly;
    MTable *w;
};

void endImageRotate(void *handle)
{
    struct HandleImageRotate *info = (struct HandleImageRotate *)handle;

    if(info->lx != NULL) mTableRelease(info->lx);
    if(info->lx != NULL) mTableRelease(info->ly);
    if(info->w  != NULL) mTableRelease(info->w );
}

#define HASH_ImageRotate 0x35b8aedf

void mImageRotate(MImage *src,MImage *dst,MImagePoint *src_hold,MImagePoint *dst_hold,float angle,int mode)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid source image");
    if(INVALID_POINTER(dst)) dst=src;
    
    float scx,scy,dcx,dcy;
    if(src_hold == NULL) {scx = ((float)(src->width))/2.0f; scy = ((float)(src->height))/2.0f;}
    else                 {scx = src_hold->x;                scy = src_hold->y;                }
    int height=dst->height;int width=dst->width;
    
    if(angle==0.0f) 
    {
        if(dst_hold==NULL) {mImageCopy(src,dst); return;}
        int dx=dst_hold->x-scx;int dy=dst_hold->y-scy;
        mImageCut(src,dst,0,src->width,0,src->height,dx,dy);
        return;
    }
    
    float sn = mSin(angle);
    float cs = mCos(angle);
    
    MImage *p = dst;
    if(dst==src)
    {
        height = (int)(src->height*ABS(cs)+src->width*ABS(sn));
        width  = (int)(src->height*ABS(sn)+src->width*ABS(cs));
        if(dst_hold != NULL)
        {
            float x =((scx+scx-src->width)*cs - (scy+scy-src->height)*sn + height)/2.0f;
            float y =((scx+scx-src->width)*sn + (scy+scy-src->height)*cs + width)/2.0f;
            width = width + (int)(x - dst_hold->x);
            height= height+ (int)(y + dst_hold->y);
        }
        dst = mImageCreate(src->channel,height,width,NULL);
    }
    else if((height<=0)||(width<=0))
    {
        height = (int)(src->height*ABS(cs)+src->width*ABS(sn));
        width  = (int)(src->height*ABS(sn)+src->width*ABS(cs));
        if(dst_hold != NULL)
        {
            float x =((scx+scx-src->width)*cs - (scy+scy-src->height)*sn + height)/2.0f;
            float y =((scx+scx-src->width)*sn + (scy+scy-src->height)*cs + width)/2.0f;
            width = width + (int)(x - dst_hold->x);
            height= height+ (int)(y + dst_hold->y);
        }
        mImageRedefine(dst,src->channel,height,width,dst->data);
    }
    else
        mImageRedefine(dst,src->channel,DFLT,DFLT,dst->data);
    
    mException(INVALID_IMAGE(dst),EXIT,"invalid error");
    
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    if(dst_hold == NULL) {dcx = ((float)width)/2.0f;dcy = ((float)height)/2.0f;}
    else                 {dcx = dst_hold->x;        dcy = dst_hold->y;         }
    
    if(angle == 90.0f)
        if((scx == dcy)&&(scy == dcx))
            if((src->width == height)&&(src->height == width))
                {ImageRotate90(src,dst);if(p!=dst){mImageExchange(src,dst);mImageRelease(dst);}return;}
    if(angle == 180.0f)
        if((scx == dcx)&&(scy == dcy))
            if((src->width == width)&&(src->height == height))
                {ImageRotate180(src,dst);if(p!=dst){mImageExchange(src,dst);mImageRelease(dst);}return;}
    if(angle == 270.0f)
        if((scx == dcy)&&(scy == dcx))
            if((src->width == height)&&(src->height == width))
                {ImageRotate270(src,dst);if(p!=dst){mImageExchange(src,dst);mImageRelease(dst);}return;}
    
    MHandle *hdl=mHandle(src,ImageRotate);
    struct HandleImageRotate *handle = (struct HandleImageRotate *)(hdl->handle);
    if((hdl->valid == 0)
     ||(handle->src_hold.x != scx)||(handle->src_hold.y != scy)
     ||(handle->dst_hold.x != dcx)||(handle->dst_hold.y != dcy)
     ||(handle->angle !=angle)
     ||(handle->height != height)
     ||(handle->width  != width))
    {
        float para[6];
        para[0] = cs;
        para[1] = sn;
        para[2] = scx - dcx*cs - dcy*sn;
        
        para[3] = 0-sn;
        para[4] = cs;
        para[5] = scy + dcx*sn - dcy*cs;

        if(handle->lx==NULL) handle->lx=mTableCreate(height,width,S16,NULL);
        else               mTableRedefine(handle->lx,height,width,S16,NULL);
        if(handle->ly==NULL) handle->ly=mTableCreate(height,width,S16,NULL);
        else               mTableRedefine(handle->ly,height,width,S16,NULL);
        if(handle->w ==NULL) handle->w =mTableCreate(height,width, U8,NULL);
        else               mTableRedefine(handle->w ,height,width, U8,NULL);
        
        TransformGrid(src,Affine_x,Affine_y,para,handle->lx,handle->ly,handle->w);

        handle->src_hold.x = scx; handle->src_hold.y = scy;
        handle->dst_hold.x = dcx; handle->dst_hold.y = dcy;
        handle->angle = angle;
        handle->height = height;
        handle->width  = width;
        
        hdl->valid = 1;
    }
    
    GridInterpolation(src,dst,handle->lx,handle->ly,handle->w,mode);
    
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
}

#define TRANSFORM_VALUE(Src,Sx,Sy,Dst,Dx,Dy) {\
    int Y1,Y2,X1,X2;\
    float Wx,Wy;\
    float W11,W12,W21,W22;\
    int Cn;\
    \
    if((Sx<0)||(Sy<0)||(Sx>Src->width-1)||(Sy>Src->height-1))\
    {\
        for(Cn=0;Cn<dst->channel;Cn++)\
            Dst->data[Cn][Dy][Dx] = 0;\
    }\
    else\
    {\
        Y1 = (int)Sy;\
        Y2 = Y1+1;\
        Wy = Sy-(float)Y1;\
        \
        X1 = (int)Sx;\
        X2 = X1+1;\
        Wx = Sx-(float)X1;\
        \
        W22 = Wx*Wy;\
        W11 = 1.0f-Wx-Wy+W22;\
        W21 = Wx-W22;\
        W12 = Wy-W22;\
        \
        for(Cn=0;Cn<dst->channel;Cn++)\
            Dst->data[Cn][Dy][Dx] = Src->data[Cn][Y1][X1]*W11+Src->data[Cn][Y1][X2]*W21+Src->data[Cn][Y2][X1]*W12+Src->data[Cn][Y2][X2]*W22;\
    }\
}

/*
void mImageCoordinateTransform(MImage *src,MImage *dst,float (*x_func)(int,int),float (*y_func)(int,int))
{
    int i,j,cn;
    
    float lx,ly;
    
    int x1,x2,y1,y2;
    float wx,wy;
    float w11,w12,w21,w22;
    
    MImage *p;
    
    mException(INVALID_IMAGE(src),"invalid input",EXIT);
    
    p=dst;
    if(INVALID_POINTER(dst)||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
    {
        mException(INVALID_IMAGE(dst),"invalid input",EXIT);
        mException((src->channel != dst->channel),"invalid input",EXIT);
    }
    
    for(j=0;j<dst->height;j++)
        for(i=0;i<dst->width;i++)
        {
            lx = x_func(i,j);
            ly = y_func(i,j);
            
            y1 = (int)ly;
            y2 = y1+1;
            wy = ly-(float)y1;
            
            x1 = (int)lx;
            x2 = x1+1;
            wx = lx-(float)x1;
            
            w22 = wx*wy;
            w11 = 1.0f-wx-wy+w22;
            w21 = wx-w22;
            w12 = wy-w22;
            
            for(cn=0;cn<dst->channel;cn++)
                dst->data[cn][j][i] = SRC(cn,x1,y1)*w11+SRC(cn,x2,y1)*w21+SRC(cn,x1,y2)*w12+SRC(cn,x2,y2)*w22;
        }
        
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
}
*/



struct DeformationTemplate {
    int locate_x;
    int locate_y;
    
    MMatrix *x;
    MMatrix *y;
};

void ImageDeformation(MImage *src,MImage *dst,struct DeformationTemplate *temp)
{
    int i,j,cn,m,n;
    float lx,ly;
    int x1,x2,y1,y2;
    float wx,wy;
    float w11,w12,w21,w22;
    
    MImage *p;
    MMatrix *tx,*ty;
    int si,ei,sj,ej;
    
    mException(INVALID_IMAGE(src)||INVALID_POINTER(temp),EXIT,"invalid input");
    mException(((temp->locate_x>=src->width)||(temp->locate_y>=src->height)),EXIT,"invalid temp");
    
    tx = temp->x;
    ty = temp->y;
    
    if(temp->locate_x <0)
    {
        m=0-temp->locate_x;
        si=0;
    }
    else
    {
        m=0;
        si=temp->locate_x;
    }
    ei = MIN(dst->width,(si+tx->col));
    if(temp->locate_y <0)
    {
        n=0-temp->locate_y;
        sj=0;
    }
    else
    {
        n=0;
        sj=temp->locate_y;
    }
    ej = MIN(dst->height,(sj+tx->row));
    
    p=dst;
    if(INVALID_POINTER(dst)||(dst==src))
    {
        dst = src;
        src = mImageCreate(dst->channel,dst->height,dst->width,NULL);
        for(cn=0;cn<dst->channel;cn++)
            for(j=sj;(j<dst->height)&&(n<tx->row);j++,n++)
                memcpy(src->data[cn][j]+si,dst->data[cn][j]+si,(ei-si)*sizeof(unsigned char));
    }
    else
    {
        mException(INVALID_IMAGE(dst),EXIT,"invalid input");
        mException((src->channel != dst->channel)||(src->height!=dst->height)||(src->width!=dst->width),EXIT,"invalid input");
        for(cn=0;cn<dst->channel;cn++)
            for(j=0;j<dst->height;j++)
                memcpy(dst->data[cn][j],src->data[cn][j],src->width*sizeof(unsigned char));
    }
    
    for(j=sj;j<ej;j++,n++)
        for(i=si;i<ei;i++,m++)
        {
            lx = tx->data[n][m];
            ly = ty->data[n][m];
            
            if((lx<=0.0f)||(ly<=0.0f))
                continue;
            
            y1 = (int)ly;
            y2 = y1+1;
            wy = ly-(float)y1;
            
            x1 = (int)lx;
            x2 = x1+1;
            wx = lx-(float)x1;
            
            w22 = wx*wy;
            w11 = 1.0f-wx-wy+w22;
            w21 = wx-w22;
            w12 = wy-w22;
            
            for(cn=0;cn<dst->channel;cn++)
                dst->data[cn][j][i] = (unsigned char)(SRC(cn,x1,y1)*w11+SRC(cn,x2,y1)*w21+SRC(cn,x1,y2)*w12+SRC(cn,x2,y2)*w22);
        }
        
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
}

void TemplateCacuate(int x_in,int y_in,int R,float k,float cx,float cy,int *x_out,int *y_out)
{
    float l;
    float d_x,d_y;
    float d;
        
    d_x = (x_in-cx);
    d_y = (y_in-cy);
    d = d_x*d_x+d_y*d_y;
    if(d>=(float)(R*R))
    {
        *x_out = DFLT;
        *y_out = DFLT;
        return;
    }
    
    d = (float)sqrt(d);
    
    l = (1.0f-(d/((float)R)));
    l = l*l;
    l =(k-1.0f)*l + 1.0f;
    
    *x_out = (int)(cx + d_x*l);
    *y_out = (int)(cy + d_y*l);
}
/*
int GrtTemplate(struct DeformationTemplate *tep,MList *curve,int R,float k)
{
    int height;
    int width;
    
    int i,j;
    int x0,y0,x1,y1;
    
    int n;
    
    MImagePoint **point;
    
    height = tep->x->row;
    width  = tep->x->col;
    
    point = curve->data;
    
    d1 = (point[0]->x)*(point[0]->x)+(point[0]->y)*(point[0]->y);
    d2 = (point[1]->x)*(point[1]->x)+(point[1]->y)*(point[1]->y);
    min = d1+d2;
    min_index=0;
    n=1;
    while(n+1<curve->num)
    {
        d1 = d2;
        d2 = (point[n+1]->x)*(point[n+1]->x)+(point[n+1]->y)*(point[n+1]->y);
        if(d1+d2<min)
        {
            min = d1+d2;
            min_index = n;
        }
        n=n+1;
    }
    TemplateCacuate(0,0,R,k,point[min_index]->x,point[min_index]->y,point[min_index+1]->x,point[min_index+1]->y,tep->x->data[0][0],tep->y->data[0][0]);
    
    for(j=0;j<height;j=j+2)
    {
        for(i=0;i<width;i++)
        {
            while((n>0)&&(n<curve->num))
            {
                d1 = 
*/

void mImageReshapeTemplate(MList *src,MList *dst,MTable *lx,MTable *ly,MTable *w)
{
    int height = w->row;
    int width = w->col;
    
    int x_step = (width+30)/31;
    int y_step = (height+30)/31;
    float area = (float)(x_step*y_step);
    
    int i,j,k,m,n;
    
    float x_locate[32][32];
    float y_locate[32][32];
    
    MImagePoint **ps = (MImagePoint **)(src->data);
    MImagePoint **pd = (MImagePoint **)(dst->data);
    
    #define DISTANCE(x1,y1,x2,y2) (float)(sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)))
    
    for(n=0,j=0;n<32;n++,j+=y_step)
        for(m=0,i=0;m<32;m++,i+=x_step)
        {
            float sum = 0.0f;
            float dx = 0.0f;
            float dy = 0.0f;
            for(k=0;k<dst->num;k++)
            {
                float d = DISTANCE(i,j,pd[k]->x,pd[k]->y);
                if(d == 0.0f)
                {
                    dx = (pd[k]->x - ps[k]->x);
                    dy = (pd[k]->y - ps[k]->y);
                    break;
                }
                
                sum += 1.0f/d;
                dx += (pd[k]->x - ps[k]->x)/d;
                dy += (pd[k]->y - ps[k]->y)/d;
            }
            if(k == dst->num)
            {
                dx = dx/sum;
                dy = dy/sum;
            }
        
            x_locate[n][m] = dx + i;
            y_locate[n][m] = dy + j;
        }
    
    #pragma omp parallel for
    for(j=0;j<height;j++)
    {
        n = j/y_step;
        float wy2 = (float)(j%y_step);
        float wy1 = y_step - wy2;
        for(i=0;i<width;i++)
        {
            m = i/x_step;
            float wx2 = (float)(i%x_step);
            float wx1 = x_step-wx2;
            
            float x =((x_locate[n  ][m]*wx1 + x_locate[n  ][m+1]*wx2)*wy1
                    + (x_locate[n+1][m]*wx1 + x_locate[n+1][m+1]*wx2)*wy2)/area;
            float y =((y_locate[n  ][m]*wx1 + y_locate[n  ][m+1]*wx2)*wy1
                    + (y_locate[n+1][m]*wx1 + y_locate[n+1][m+1]*wx2)*wy2)/area;
            
            int wx = (int)x;
            int wy = (int)y;
            
            lx->dataS16[j][i] = wx;
            ly->dataS16[j][i] = wy;
            
            wx = 16-(int)((x-(float)wx)/0.0625f+0.5f);
            wy = 16-(int)((y-(float)wy)/0.0625f+0.5f);

            w->dataU8[j][i] = (wx<<4)+wy;
        }
    }
}

struct HandleImageReshape
{
    MTable *lx;
    MTable *ly;
    MTable *w;
};
void endImageReshape(void *info)
{
    struct HandleImageReshape *handle = (struct HandleImageReshape *)info;

    if(handle->lx != NULL) mTableRelease(handle->lx);
    if(handle->lx != NULL) mTableRelease(handle->ly);
    if(handle->w  != NULL) mTableRelease(handle->w );
}
#define HASH_ImageReshape 0xe21f102e
void mImageReshape(MImage *src,MImage *dst,MList *src_point,MList *dst_point,int mode)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid source image");
    
    MImage *p = dst;
    if(INVALID_POINTER(dst)||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else if((dst->height<=0)||(dst->width<=0))
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    else
        mImageRedefine(dst,src->channel,DFLT,DFLT,dst->data);
     
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    MHandle *hdl=mHandle(dst,ImageReshape);
    struct HandleImageReshape *handle = (struct HandleImageReshape *)(hdl->handle);
    {
        if(handle->lx == NULL) handle->lx= mTableCreate(dst->height,dst->width,S16,NULL);
        else                  mTableRedefine(handle->lx,dst->height,dst->width,S16,NULL);
        if(handle->ly == NULL) handle->ly= mTableCreate(dst->height,dst->width,S16,NULL);
        else                  mTableRedefine(handle->ly,dst->height,dst->width,S16,NULL);
        if(handle-> w == NULL) handle->w = mTableCreate(dst->height,dst->width, U8,NULL);
        else                  mTableRedefine(handle->w ,dst->height,dst->width, U8,NULL);
        
        hdl->valid = 1;
    }
    
    mImageReshapeTemplate(src_point,dst_point,handle->lx,handle->ly,handle->w);
    
    GridInterpolation(src,dst,handle->lx,handle->ly,handle->w,mode);
    
    if(p!=dst)
    {
        mImageExchange(src,dst);
        mImageRelease(dst);
    }
}



