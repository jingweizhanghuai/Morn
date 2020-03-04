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

struct HandleImageCreate
{
    MImage *img;
    int cn;
    int height;
    int width;
    unsigned char **index;
    MMemory *memory;
};
void endImageCreate(void *info) 
{
    struct HandleImageCreate *handle = (struct HandleImageCreate *)info;
    mException((handle->img == NULL),EXIT,"invalid image");
   
    if(!INVALID_POINTER(handle->index))
        mFree(handle->index);
    
    if(!INVALID_POINTER(handle->memory))
        mMemoryRelease(handle->memory);
  
    mFree(handle->img);
}
#define HASH_ImageCreate 0xccb34f86
MImage *mImageCreate(int cn,int height,int width,unsigned char **data[])
{
    if(cn <0) {cn = 0;} if(height <0) {height = 0;} if(width <0) {width = 0;}
    mException((cn>MORN_MAX_IMAGE_CN),EXIT,"invalid input");
   
    MImage *img = (MImage *)mMalloc(sizeof(MImage));
    memset(img,0,sizeof(MImage));
    img->height = height;img->width = width;img->channel = cn;
    
         if(cn==1) mInfoSet(&(img->info),"image_type",MORN_IMAGE_GRAY);
    else if(cn==3) mInfoSet(&(img->info),"image_type",MORN_IMAGE_RGB);
    else if(cn==4) mInfoSet(&(img->info),"image_type",MORN_IMAGE_RGBA);
   
    MHandle *hdl; ObjectHandle(img,ImageCreate,hdl);
    struct HandleImageCreate *handle = (struct HandleImageCreate *)(hdl->handle);
    handle->img = img;
    
    if((cn==0)||(height == 0))
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        return img;
    }
    
    if(!INVALID_POINTER(data))
    {
        memcpy(img->data,data,cn*sizeof(unsigned char **));
        mInfoSet(&(img->info),"border_type",MORN_BORDER_INVALID);
        return img;
    }
    
    int col = width + 32;
    int row = height+ 16;
    handle->index = (unsigned char **)mMalloc(row*cn*sizeof(unsigned char *));
    handle->height = height;
    handle->cn = cn;
    
    if(width==0)
    {
        mException(!INVALID_POINTER(data),EXIT,"invalid input");
        memset(handle->index,0,row*cn*sizeof(unsigned char *));
        mInfoSet(&(img->info),"border_type",MORN_BORDER_INVALID);
        return img;
    }
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(cn*row,col*sizeof(unsigned char));
    mMemoryIndex(handle->memory,cn*row,col*sizeof(unsigned char),(void **)(handle->index));
    handle->width = width;
    
    for(int i=0;i<cn*row;i++) handle->index[i] = &(handle->index[i][16]);
    for(int k=0;k<cn;k++) img->data[k] = handle->index + k*row+8;
    
    mInfoSet(&(img->info),"border_type",MORN_BORDER_UNDEFINED);
    
    return img;
}

void mImageRedefine(MImage *img,int cn,int height,int width,unsigned char **data[])
{
    mException((INVALID_POINTER(img)),EXIT,"invalid input");
    
    if(cn<=0)       cn     = img->channel;
    if(height <= 0) height = img->height;
    if(width<=0)    width  = img->width;
    
    if((cn!=img->channel)||(height!=img->height)||(width!=img->width)) mHandleReset(img->handle);
    
    int same_size = ((cn<=img->channel)&&(height<=img->height)&&(width<=img->width));
    int reuse = (data==img->data);
    int flag = (img->channel&&img->height&&img->width);
    
    mException((cn>MORN_MAX_IMAGE_CN),EXIT,"invalid input channel");
    if(cn!= img->channel)
    {
             if(cn==1) mInfoSet(&(img->info),"image_type",MORN_IMAGE_GRAY);
        else if(cn==3) mInfoSet(&(img->info),"image_type",MORN_IMAGE_RGB);
        else if(cn==4) mInfoSet(&(img->info),"image_type",MORN_IMAGE_RGBA);
    }
    
    img->channel = cn;
    img->height = height;
    img->width = width;
    
    if(same_size&&reuse) return;
    struct HandleImageCreate *handle = (struct HandleImageCreate *)(((MHandle *)(img->handle->data[0]))->handle);
    if(same_size&&(data==NULL)&&(handle->width >0)) return;
    mException(reuse&&flag&&(handle->width==0),EXIT,"invalid redefine");
    
    handle->width = 0;
    
    img->border = NULL;
    mInfoSet(&(img->info),"border_type",MORN_BORDER_UNDEFINED);
    
    if((height<=0)||(cn<=0)||(width<=0))
    {
        mException((data!=NULL)&&(!reuse),EXIT,"invalid input");
        memset(img->data,0,MORN_MAX_IMAGE_CN*sizeof(unsigned char*));
        return;
    }
        
    if(reuse) data=NULL;
    
    int col = width + 32;
    int row = height+ 16;
    if(height*cn>handle->height*handle->cn)
    {
        if(handle->index!=NULL) mFree(handle->index); 
        handle->index = NULL;
    }
    if(handle->index == NULL)
    {
        handle->index = (unsigned char **)mMalloc(row*cn*sizeof(unsigned char *));
        handle->cn = cn;
        handle->height = height;
    }
    
    if(data!=NULL)
    {
        memcpy(img->data,data,cn*sizeof(unsigned char **));
        mInfoSet(&(img->info),"border_type",MORN_BORDER_INVALID);
        return;
    }
   
    if(handle->memory == NULL) handle->memory = mMemoryCreate(cn*row,col*sizeof(unsigned char));
    mMemoryIndex(handle->memory,cn*row,col*sizeof(unsigned char),(void **)(handle->index));
   
    for(int i=0;i<cn*row;i++) handle->index[i] = &(handle->index[i][16]);
    handle->width = width;
    
    for(int k=0;k<cn;k++) img->data[k] = handle->index + k*row+8;
}

void mImageRelease(MImage *img)
{
    mException(INVALID_POINTER(img),EXIT,"invalid input");
    
    if(!INVALID_POINTER(img->handle))
        mHandleRelease(img->handle);
}

void mImageExpand(MImage *img,int r,int border_type) 
{
    mException((r>8)||(border_type < MORN_BORDER_BLACK)||(border_type > MORN_BORDER_REFLECT),EXIT,"invalid border type");
    int img_border_type = (int)mInfoGet(&(img->info),"border_type");
    mException((img_border_type == MORN_BORDER_INVALID),EXIT,"image expand is invalid");
    
    #define EXPEND_LEFT(Y,X1) {\
        if(border_type == MORN_BORDER_BLACK)\
            memset(img->data[cn][Y]+X1-r,0,r*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_WHITE)\
            memset(img->data[cn][Y]+X1-r,255,r*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_REPLICATE)\
            memset(img->data[cn][Y]+X1-r,img->data[cn][Y][X1],r*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_REFLECT)\
            for(int i=1;i<=r;i++) img->data[cn][Y][X1-i]=img->data[cn][Y][X1+i];\
    }
    #define EXPEND_RIGHT(Y,X2) {\
        if(border_type == MORN_BORDER_BLACK)\
            memset(img->data[cn][Y]+X2+1,0,r*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_WHITE)\
            memset(img->data[cn][Y]+X2+1,255,r*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_REPLICATE)\
            memset(img->data[cn][Y]+X2+1,img->data[cn][Y][X2],r*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_REFLECT)\
            for(int i=1;i<=r;i++) img->data[cn][Y][X2+i]=img->data[cn][Y][X2-i];\
    }
    #define EXPEND_TOP(Y,Y1,X1,X2) {\
        if(border_type == MORN_BORDER_BLACK)\
            memset(img->data[cn][Y]+X1-r,0,(X2-X1+r+r)*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_WHITE)\
            memset(img->data[cn][Y]+X1-r,255,(X2-X1+r+r)*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_REPLICATE)\
            memcpy(img->data[cn][Y]+X1-r,img->data[cn][Y1]+X1-r,(X2-X1+r+r)*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_REFLECT)\
            memcpy(img->data[cn][Y]+X1-r,img->data[cn][Y1+Y1-Y]+X1-r,(X2-X1+r+r)*sizeof(unsigned char));\
    }
    #define EXPEND_BUTTOM(Y,Y2,X1,X2) {\
        if(border_type == MORN_BORDER_BLACK)\
            memset(img->data[cn][Y]+X1-r,0,(X2-X1+r+r)*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_WHITE)\
            memset(img->data[cn][Y]+X1-r,255,(X2-X1+r+r)*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_REPLICATE)\
            memcpy(img->data[cn][Y]+X1-r,img->data[cn][Y2]+X1-r,(X2-X1+r+r)*sizeof(unsigned char));\
        else if(border_type == MORN_BORDER_REFLECT)\
            memcpy(img->data[cn][Y]+X1-r,img->data[cn][Y2+Y2-Y]+X1-r,(X2-X1+r+r)*sizeof(unsigned char));\
    }
    
    int height = img->height;
    int width = img->width;
    
    int y1 = ImageY1(img);
    int y2 = ImageY2(img)-1;
    
    if(img_border_type == MORN_BORDER_IMAGE)
    {
        for(int cn=0;cn<img->channel;cn++)
        {
            int j;
            for(j=MAX(y1-r,0);j<y1;j++)        {int x1 = ImageX1(img,y1); if(x1<r) EXPEND_LEFT(j,0); int x2 =ImageX2(img,y1); if(x2>width-r) EXPEND_RIGHT(j,width);}
            #pragma omp parallel for
            for(j=y1;j<=y2;j++)                {int x1 = ImageX1(img,j ); if(x1<r) EXPEND_LEFT(j,0); int x2 =ImageX2(img,j ); if(x2>width-r) EXPEND_RIGHT(j,width);}
            for(j=y2+1;j<MIN(y2+r,height);j++) {int x1 = ImageX1(img,y2); if(x1<r) EXPEND_LEFT(j,0); int x2 =ImageX2(img,y2); if(x2>width-r) EXPEND_RIGHT(j,width);}
            
            int x1,x2;
            x1 = ImageX1(img,y1); x2 = ImageX2(img,y1); for(j=y1-r;j<0;j++)          EXPEND_TOP(j,         0,x1,x2);
            x1 = ImageX1(img,y2); x2 = ImageX2(img,y2); for(j=height;j<=y2+r;j++) EXPEND_BUTTOM(j,(height-1),x1,x2);
        }
        return;
    }
    
    for(int cn=0;cn<img->channel;cn++)
    {
        int j;
        #pragma omp parallel for
        for(j=y1;j<=y2;j++)
        {
            int x1 = ImageX1(img,j);
            int x2 = ImageX2(img,j)-1;
            EXPEND_LEFT(j,x1);
            EXPEND_RIGHT(j,x2);
        }
        
        int x1,x2;
        x1 = ImageX1(img,y1);
        x2 = ImageX2(img,y1)-1;
        for(j=y1-r;j<y1;j++)
            EXPEND_TOP(j,y1,x1,x2);
        
        x1 = ImageX1(img,y2);
        x2 = ImageX2(img,y2)-1;
        for(j=y2+1;j<=y2+r;j++)
            EXPEND_BUTTOM(j,y2,x1,x2);
    }
    
    mInfoSet(&(img->info),"border_type",border_type);
}

void ImageCut(MImage *img,MImage *dst,int x1,int x2,int y1,int y2)
{
    if((x1==0)&&(x2==img->width)&&(y1==0)&&(y2==img->height)) {mImageCopy(img,dst);return;}
        
    MImage *p=dst;
    if(dst==NULL) dst = mImageCreate(img->channel,ABS(y2-y1),ABS(x2-x1),NULL);
    else          mImageRedefine(dst,img->channel,ABS(y2-y1),ABS(x2-x1),NULL);
    int cn=img->channel;
    
    for(int j=0,y=y1;j<dst->height;j++,y+=((y2>y1)?1:-1))
    for(int i=0,x=x1;i<dst-> width;i++,x+=((x2>x1)?1:-1))
    {
             if((y<0)||(y>=img->height)) {for(int c=0;c<cn;c++) dst->data[c][j][i]=0;}
        else if((x<0)||(x>=img-> width)) {for(int c=0;c<cn;c++) dst->data[c][j][i]=0;}
        else                             {for(int c=0;c<cn;c++) dst->data[c][j][i]=img->data[c][y][x];}
    }
    
    if(p!=dst)
    {
        mImageExchange(img,dst);
        mImageRelease(dst);
    }
}

void mImageCut(MImage *img,MImage *ROI,int x1,int x2,int y1,int y2)
{
    mException(INVALID_IMAGE(img),EXIT,"invalid input");
    if((x1<0)||(x2>=img->width)||(x1>=x2)||(y1<0)||(y2>=img->height)||(y1>=y2))
        {ImageCut(img,ROI,x1,x2,y1,y2);return;}
    
    if(INVALID_POINTER(ROI)) ROI=img;
    else mImageRedefine(ROI,img->channel,y2-y1,x2-x1,ROI->data);
    
    // printf("x1 is %d,x2 is %d,y1 is %d,y2 is %d\n",x1,x2,y1,y2);
    int i, j;
    for(int cn=0;cn<ROI->channel;cn++)
    {
        for(j=y1;j<y2;j++)
        {
            i = j-y1;
            memmove(ROI->data[cn][i],img->data[cn][j]+x1,(x2-x1)*sizeof(unsigned char));
        }
    }
    
    ROI->border = NULL;
    mInfoSet(&(ROI->info),"image_type",mInfoGet(&(img->info),"image_type"));
    ROI->width = x2-x1;
    ROI->height = y2-y1;
}

void mImageCopy(MImage *src,MImage *dst)
{
    int i;
    if(src==dst) return;

    mException((INVALID_IMAGE(src)||INVALID_POINTER(dst)),EXIT,"invalid input");
    mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    dst->border = src->border;
    dst->info = src->info;
    
    for(int cn=0;cn<src->channel;cn++)
    {
        #pragma omp parallel for
        for(i=0;i<src->height;i++)
            memcpy(dst->data[cn][i],src->data[cn][i],src->width*sizeof(unsigned char));
    }
}

void mImageDiff(MImage *src1,MImage *src2,MImage *diff)
{
    mException((INVALID_IMAGE(src1)||INVALID_IMAGE(src2)),EXIT,"invalid input");
    mException(((src1->channel!=src2->channel)||(src1->height != src2->height)||(src1->width != src2->width)),EXIT,"invalid input");
    
    if(INVALID_POINTER(diff)) diff = src1;
    else mImageRedefine(diff,src1->channel,src1->height,src1->width,diff->data);
    
    if(!INVALID_POINTER(src1->border))      diff->border = src1->border;
    else if(!INVALID_POINTER(src2->border)) diff->border = src2->border;
    
    for(int k=0;k<diff->channel;k++)
    {
        int j;
        #pragma omp parallel for
        for(j=ImageY1(diff);j<ImageY2(diff);j++)
            for(int i=ImageX1(diff,j);i<ImageX2(diff,j);i++)
                diff->data[k][j][i] = ABS(src1->data[k][j][i]-src2->data[k][j][i]);
    }
}

void mImageDiffThreshold(MImage *src1,MImage *src2,MImage *diff,int thresh)
{
    mException((INVALID_IMAGE(src1)||INVALID_IMAGE(src2)),EXIT,"invalid input");
    mException(((src1->channel!=src2->channel)||(src1->height != src2->height)||(src1->width != src2->width)),EXIT,"invalid input");
    
    if(INVALID_POINTER(diff)) diff = src1;
    else mImageRedefine(diff,1,src1->height,src1->width,diff->data);
       
    if(!INVALID_POINTER(src1->border))      diff->border = src1->border;
    else if(!INVALID_POINTER(src2->border)) diff->border = src2->border;
    
    int j;
    #pragma omp parallel for
    for(j=ImageY1(diff);j<ImageY2(diff);j++)
        for(int i=ImageX1(diff,j);i<ImageX2(diff,j);i++)
        {
            diff->data[0][j][i] = 0;
            for(int k=0;k<src1->channel;k++)
            {
                if(ABS(src1->data[k][j][i]-src2->data[k][j][i])>thresh)
                {
                    diff->data[0][j][i] = 255;
                    break;
                }
            }
        }
    
    diff->channel = 1;
}

void mImageDataAdd(MImage *src1,MImage *src2,MImage *dst)
{
    mException((INVALID_IMAGE(src1)||INVALID_IMAGE(src2)),EXIT,"invalid input");
    mException(((src1->height != src2->height)||(src1->width != src2->width)),EXIT,"invalid input");
    mException((src2->channel!=src1->channel)&&(src2->channel!=1),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) {dst = src1;}
    else {mImageRedefine(dst,src1->channel,src1->height,src1->width,dst->data);}
    
         if(!INVALID_POINTER(src1->border)) dst->border = src1->border;
    else if(!INVALID_POINTER(src2->border)) dst->border = src2->border;
       
    unsigned char **data1,**data2;
    for(int k=0;k<dst->channel;k++)
    {
        data1 = src1->data[k];
        data2 = (k>=src2->channel)?src2->data[0]:src2->data[k];
        
        int j;
        #pragma omp parallel for
        for(j=ImageY1(dst);j<ImageY2(dst);j++)
            for(int i=ImageX1(dst,j);i<ImageX2(dst,j);i++)
            {
                register int data = data1[j][i]+data2[j][i];
                dst->data[k][j][i] = (data>255)?255:data;
            }
    }
}

void mImageDataSub(MImage *src1,MImage *src2,MImage *dst)
{
    mException((INVALID_IMAGE(src1)||INVALID_IMAGE(src2)),EXIT,"invalid input");
    mException(((src1->height != src2->height)||(src1->width != src2->width)),EXIT,"invalid input");
    mException((src2->channel!=src1->channel)&&(src2->channel!=1),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) {dst = src1;}
    else {mImageRedefine(dst,src1->channel,src1->height,src1->width,dst->data);}
    
         if(!INVALID_POINTER(src1->border)) dst->border = src1->border;
    else if(!INVALID_POINTER(src2->border)) dst->border = src2->border;
    
    unsigned char **data1,**data2;
    for(int k=0;k<dst->channel;k++)
    {
        data1 = src1->data[k];
        data2 = (k>=src2->channel)?src2->data[0]:src2->data[k];
        
        int j;
        #pragma omp parallel for
        for(j=ImageY1(dst);j<ImageY2(dst);j++)
            for(int i=ImageX1(dst,j);i<ImageX2(dst,j);i++)
            {
                register int data = data1[j][i]-data2[j][i];
                dst->data[k][j][i] = (data<0)?0:data;
            }
    }
}

void mImageDataAnd(MImage *src1,MImage *src2,MImage *dst)
{
    mException((INVALID_IMAGE(src1)||INVALID_IMAGE(src2)),EXIT,"invalid input");
    mException(((src1->height != src2->height)||(src1->width != src2->width)),EXIT,"invalid input");
    mException((src2->channel!=src1->channel)&&(src2->channel!=1),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) {dst = src1;}
    if(dst!=src1) {mImageRedefine(dst,src1->channel,src1->height,src1->width,dst->data);}
   
         if(!INVALID_POINTER(src1->border)) dst->border = src1->border;
    else if(!INVALID_POINTER(src2->border)) dst->border = src2->border;
    
    unsigned char **data1,**data2;
    for(int k=0;k<dst->channel;k++)
    {
        data1 = src1->data[k];
        data2 = (k>=src2->channel)?src2->data[0]:src2->data[k];
        
        int j;
        #pragma omp parallel for
        for(j=ImageY1(dst);j<ImageY2(dst);j++)
            for(int i=ImageX1(dst,j);i<ImageX2(dst,j);i++)
                dst->data[k][j][i] = data1[j][i]&data2[j][i];
    }
}

void mImageDataOr(MImage *src1,MImage *src2,MImage *dst)
{
    mException((INVALID_IMAGE(src1)||INVALID_IMAGE(src2)),EXIT,"invalid input");
    mException(((src1->height != src2->height)||(src1->width != src2->width)),EXIT,"invalid input");
    mException((src2->channel!=src1->channel)&&(src2->channel!=1),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) {dst = src1;}
    else {mImageRedefine(dst,src1->channel,src1->height,src1->width,dst->data);}
    
         if(!INVALID_POINTER(src1->border)) dst->border = src1->border;
    else if(!INVALID_POINTER(src2->border)) dst->border = src2->border;
    
    unsigned char **data1,**data2;
    for(int k=0;k<dst->channel;k++)
    {
        data1 = src1->data[k];
        data2 = (k>=src2->channel)?src2->data[0]:src2->data[k];
        
        int j;
        #pragma omp parallel for
        for(j=ImageY1(dst);j<ImageY2(dst);j++)
            for(int i=ImageX1(dst,j);i<ImageX2(dst,j);i++)
                dst->data[k][j][i] = data1[j][i]|data2[j][i];
    }
}

void mImageInvert(MImage *src,MImage *dst)
{
    mException((INVALID_IMAGE(src)),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) {dst = src;}
    else {mImageRedefine(dst,src->channel,src->height,src->width,dst->data);}
    
    if(!INVALID_POINTER(src->border)) dst->border = src->border;
    
    for(int k=0;k<dst->channel;k++)
    {
        int j;
        #pragma omp parallel for
        for(j=ImageY1(dst);j<ImageY2(dst);j++)
            for(int i=ImageX1(dst,j);i<ImageX2(dst,j);i++)
                dst->data[k][j][i] = 255-src->data[k][j][i];
    }
    dst->border = src->border;
}

void mImageLinearMap(MImage *src,MImage *dst,float k,float b)
{
    mException((INVALID_IMAGE(src)),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) {dst = src;}
    else {mImageRedefine(dst,src->channel,src->height,src->width,dst->data);}
    
    if(!INVALID_POINTER(src->border)) dst->border = src->border;
    
    unsigned char data[256];
    for(int i=0;i<256;i++)
    {
        float rst = ((float)i)*k+b;
        if(rst>255.0f)      data[i] = 255;
        else if(rst<0.0f)   data[i] = 0;
        else                data[i] = (unsigned char)(rst+0.5f);
    }
    
    for(int cn=0;cn<dst->channel;cn++)
    {
        int j;
        #pragma omp parallel for
        for(j=ImageY1(dst);j<ImageY2(dst);j++)
            for(int i=ImageX1(dst,j);i<ImageX2(dst,j);i++)
                dst->data[cn][j][i] = data[src->data[cn][j][i]];
    }
    dst->border = src->border;
}

void mImageOperate(MImage *src,MImage *dst,void (*func)(unsigned char *,unsigned char *,void *),void *para)
{
    mException((INVALID_IMAGE(src)),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) {dst = src;}
    else 
    {
        mException((dst->channel<=0),EXIT,"invalid input dst channel");
        mImageRedefine(dst,dst->channel,src->height,src->width,dst->data);
    }
       
    if(!INVALID_POINTER(src->border)) dst->border = src->border;
    
    unsigned char idata[MORN_MAX_IMAGE_CN];
    unsigned char odata[MORN_MAX_IMAGE_CN];
    int j;
    #pragma omp parallel for
    for(j=ImageY1(dst);j<ImageY2(dst);j++)for(int i=ImageX1(dst,j);i<ImageX2(dst,j);i++)
    {
        for(int cn=0;cn<src->channel;cn++) idata[cn]=src->data[cn][j][i];
        
        func(idata,odata,para);
        
        for(int cn=0;cn<dst->channel;cn++) dst->data[cn][j][i]=odata[cn];
    }
    dst->border = src->border;
}


struct HandleImageChannelSplit
{
    MImage *img[MORN_MAX_IMAGE_CN];
};
void endImageChannelSplit(void *info)
{
    struct HandleImageChannelSplit *handle = (struct HandleImageChannelSplit *)info;
    for(int i=0;i<MORN_MAX_IMAGE_CN;i++)
    {
        if(handle->img[i]!=NULL)
            mImageRelease(handle->img[i]);
    }
}
#define HASH_ImageChannelSplit 0x41e09e5b
MImage *mImageChannelSplit(MImage *src,int cn)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input image");
    mException((cn>=src->channel),EXIT,"invalid input");
     
    MHandle *hdl; ObjectHandle(src,ImageChannelSplit,hdl);
    struct HandleImageChannelSplit *handle = (struct HandleImageChannelSplit *)(hdl->handle);
    hdl->valid = 1;
    
    if(handle->img[cn]==NULL)
        handle->img[cn] = mImageCreate(1,DFLT,DFLT,NULL);
    handle->img[cn]->height = src->height;
    handle->img[cn]->width  = src->width ;
    handle->img[cn]->channel= 1;
    handle->img[cn]->data[0]= src->data[cn];
    return handle->img[cn];
}
    
    
