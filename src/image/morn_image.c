/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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
    
    // int backup_cn;
    // int backup_height;
    // int backup_width;
    unsigned char **backup_index;
    MMemory *backup_memory;
    unsigned char **backup_data[MORN_MAX_IMAGE_CN];
};
void endImageCreate(void *info) 
{
    struct HandleImageCreate *handle = (struct HandleImageCreate *)info;
    mException((handle->img == NULL),EXIT,"invalid image");
   
    if(!INVALID_POINTER(handle->index )) mFree(handle->index);
    if(!INVALID_POINTER(handle->memory)) mMemoryRelease(handle->memory);

    if(!INVALID_POINTER(handle->backup_index )) mFree(handle->backup_index);
    if(!INVALID_POINTER(handle->backup_memory)) mMemoryRelease(handle->backup_memory);

    memset(handle->img,0,sizeof(MImage));
    mFree(handle->img);
}
#define HASH_ImageCreate 0xccb34f86
MImage *ImageCreate(int cn,int height,int width,unsigned char **data[])
{
    if(cn <0) {cn = 0;} if(height <0) {height = 0;} if(width <0) {width = 0;}
    mException((cn>MORN_MAX_IMAGE_CN),EXIT,"invalid input");
   
    MImage *img = (MImage *)mMalloc(sizeof(MImage));
    memset(img,0,sizeof(MImage));
    img->height = height;img->width = width;img->channel = cn;
    
         if(cn==1) mInfoSet(&(img->info),"image_type",MORN_IMAGE_GRAY);
    else if(cn==3) mInfoSet(&(img->info),"image_type",MORN_IMAGE_RGB);
    else if(cn==4) mInfoSet(&(img->info),"image_type",MORN_IMAGE_RGBA);

    img->handle = mHandleCreate();
    MHandle *hdl=mHandle(img,ImageCreate);
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
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(1,cn*row*col*sizeof(unsigned char),MORN_HOST);
    mException(handle->memory->num!=1,EXIT,"invalid image memory");
    mMemoryIndex(handle->memory,cn*row,col*sizeof(unsigned char),(void ***)(&(handle->index)),1);
    handle->width = width;
    
    for(int i=0;i<cn*row;i++) handle->index[i] = &(handle->index[i][16]);
    for(int k=0;k<cn;k++) img->data[k] = handle->index + k*row+8;
    
    mInfoSet(&(img->info),"border_type",MORN_BORDER_UNDEFINED);
    
    return img;
}

void ImageRedefine(MImage *img,int cn,int height,int width,unsigned char **data[])
{
    mException((INVALID_POINTER(img)),EXIT,"invalid input");
    
    if(cn<=0)                 cn    =img->channel;
    if(height <= 0)           height=img->height;
    if(width<=0)              width =img->width;
    
    if((cn!=img->channel)||(height!=img->height)||(width!=img->width)) mHandleReset(img->handle);
    
    int same_size = ((cn<=img->channel)&&(height<=img->height)&&(width<=img->width));
    int reuse = (data==img->data);
    int flag = (img->channel&&img->height&&img->width);
    
    if(cn!= img->channel)
    {
             if(cn==1) mInfoSet(&(img->info),"image_type",MORN_IMAGE_GRAY);
        else if(cn==3) mInfoSet(&(img->info),"image_type",MORN_IMAGE_RGB);
        else if(cn==4) mInfoSet(&(img->info),"image_type",MORN_IMAGE_RGBA);
    }
    
    mException((cn>MORN_MAX_IMAGE_CN),EXIT,"invalid input channel");
    img->channel = cn;
    img->height = height;
    img->width = width;
    
    if(same_size&&reuse) return;
    struct HandleImageCreate *handle = (struct HandleImageCreate *)(((MHandle *)(img->handle->data[0]))->handle);
    if(same_size&&INVALID_POINTER(data)&&(handle->width >0)) return;
    // 
    mException(reuse&&flag&&(handle->width==0),EXIT,"invalid redefine");
    
    handle->width = 0;
    
    img->border = NULL;
    mInfoSet(&(img->info),"border_type",MORN_BORDER_UNDEFINED);
    
    if((height<=0)||(cn<=0)||(width<=0))
    {
        mException((!INVALID_POINTER(data))&&(!reuse),EXIT,"invalid input");
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
    
    if(!INVALID_POINTER(data))
    {
        memcpy(img->data,data,cn*sizeof(unsigned char **));
        if(data == handle->backup_data)
        {
            if(!INVALID_POINTER(handle->backup_index )) mFree(handle->backup_index);
            if(!INVALID_POINTER(handle->backup_memory)) mMemoryRelease(handle->backup_memory);
            handle->backup_index=NULL;
            handle->backup_memory=NULL;
        }
        else mInfoSet(&(img->info),"border_type",MORN_BORDER_INVALID);
        return;
    }

    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(1,cn*row*col*sizeof(unsigned char),MORN_HOST);
    else mMemoryRedefine(handle->memory,1,cn*row*col*sizeof(unsigned char),DFLT);
    mException(handle->memory->num!=1,EXIT,"invalid image memory");
    mMemoryIndex(handle->memory,cn*row,col*sizeof(unsigned char),(void ***)(&(handle->index)),1);
    
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

unsigned char ***mImageBackup(MImage *img,int cn,int height,int width)
{
    if(cn    <=0) cn    =img->channel;
    if(height<=0) height=img->height;
    if(width <=0) width =img->width;

    int col = width + 32;
    int row = height+ 16;
    
    struct HandleImageCreate *handle = (struct HandleImageCreate *)(((MHandle *)(img->handle->data[0]))->handle);
    if(handle->backup_index!=NULL) mFree(handle->backup_index);
    handle->backup_index = (unsigned char **)mMalloc(cn*row*sizeof(unsigned char *));
    
    if(handle->backup_memory == NULL) handle->backup_memory = mMemoryCreate(1,cn*row*col*sizeof(unsigned char),MORN_HOST);
    else mMemoryRedefine(handle->backup_memory,1,cn*row*col*sizeof(unsigned char),DFLT);
    mException(handle->backup_memory->num!=1,EXIT,"invalid image backup memory");
    mMemoryIndex(handle->backup_memory,cn*row,col*sizeof(unsigned char),(void ***)(&(handle->backup_index)),1);

    for(int i=0;i<cn*row;i++) handle->backup_index[i] = &(handle->backup_index[i][16]);
    for(int k=0;k<cn;k++) handle->backup_data[k] = handle->backup_index + k*row+8;
    return (handle->backup_data);
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

void m_ImageCut(MImage *img,MImage *dst,MImageRect *rect,MImagePoint *locate)
{
    mException(INVALID_IMAGE(img)||INVALID_POINTER(rect),EXIT,"invalid input");
    int x1=rect->x1;int x2=rect->x2;int y1=rect->y1;int y2=rect->y2;
    int lx,ly;if(INVALID_POINTER(locate)) {lx=0,ly=0;}else {lx=locate->x;ly=locate->y;}
    
    int height = ABS(y1-y2);int width = ABS(x1-x2);
    mException((height==0)||(width<=0),EXIT,"invalid input");

    if(lx<0) {x1=(x1<x2)?(x1-lx):(x1+lx); lx=0;} else if(lx>0) {x2=(x1<x2)?(x2-lx):(x2+lx);}
    if(ly<0) {y1=(y1<y2)?(y1-ly):(y1+ly); ly=0;} else if(ly>0) {y2=(y1<y2)?(y2-ly):(y2+ly);}
    
    if((y1<0)&&(y2<0)) {return;} if((y1>=img->height)&&(y2>=img->height)) {return;}
    if((x1<0)&&(x2<0)) {return;} if((x1>=img->width )&&(x2>=img->width )) {return;}
    if(y1<0) {ly=ly-y1; y1=0;} else if(y2<0) {ly=ly-y2; y2=0;}
    if(x1<0) {lx=lx-x1; x1=0;} else if(x2<0) {lx=lx-x2; x2=0;}
    y1=MIN(y1,img->height);y2=MIN(y2,img->height);
    x1=MIN(x1,img->width );x2=MIN(x2,img->width );

    int h = ABS(y1-y2);int w = ABS(x1-x2);
    if((h==0)||(w==0)) return;
    // printf("x1=%d,x2=%d,width=%d,w=%d\n",x1,x2,width,w);

    if(INVALID_POINTER(dst)) dst=img;
    unsigned char ***dst_data;
    if(dst!=img)
    {
        mImageRedefine(dst,img->channel,height,width);
        dst_data=dst->data;
    }
    else
    {
        if(( ly   *img->width+lx <= y1*img->width+x1)&&( ly   *img->width+lx+w <= y1*img->width+x2)
         &&((ly+h)*img->width+lx <= y2*img->width+x1)&&((ly+h)*img->width+lx+w <= y1*img->width+x2))
            dst_data=img->data;
        else
            dst_data=mImageBackup(img,DFLT,height,width);
    }
    
    printf("x1 is %d,x2 is %d,y1 is %d,y2 is %d,lx=%d,ly=%d,height=%d,width=%d,h=%d,w=%d\n",x1,x2,y1,y2,lx,ly,height,width,h,w);
    if(x1<x2)
    {
        for(int c=0;c<img->channel;c++)
            for(int j=ly,y=y1;j<ly+h;j++,y+=((y2>y1)?1:-1))
                memcpy(dst->data[c][j]+lx,img->data[c][y]+x1,w*sizeof(unsigned char));
    }
    else
    {
        for(int c=0;c<img->channel;c++)
            for(int j=ly,y=y1;j<ly+h;j++,y+=((y2>y1)?1:-1))
                for(int i=lx,x=x1;i<lx+w;i++,x--) 
                    dst->data[c][j][i]=img->data[c][y][x];
    }

    if(img==dst) mImageRedefine(dst,DFLT,height,width,dst_data);
}

// void mImageCut(MImage *img,MImage *ROI,int x1,int x2,int y1,int y2,int lx,int ly)
// {
//     mException(INVALID_IMAGE(img),EXIT,"invalid input");

//     int flag = (x1<0)||(x2>=img->width)||(x1>=x2)||(y1<0)||(y2>=img->height)||(y1>=y2)||((img==ROI)&&(ly<y1));
//     if(flag){ImageCut(img,ROI,x1,x2,y1,y2,lx,ly);return;}

//     int height=y2-y1;int width =x2-x1;
//     if(INVALID_POINTER(ROI)) ROI=img;
//     else mImageRedefine(ROI,img->channel,height,width);
    
//     if(lx<0) {x1=x1-lx;lx=0;} else if(lx>0) {x2=x2-lx;}
//     if(ly<0) {y1=y1-ly;ly=0;} else if(ly>0) {y2=y2-ly;}
    
//     // printf("x1 is %d,x2 is %d,y1 is %d,y2 is %d\n",x1,x2,y1,y2);
//     for(int j=y1;j<y2;j++)for(int cn=0;cn<ROI->channel;cn++)
//         memmove(ROI->data[cn][ly+j-y1]+lx,img->data[cn][j]+x1,MIN((ROI->width-lx),(x2-x1))*sizeof(unsigned char));
// }

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

void mImageWipe(MImage *img)
{
    for(int c=0;c<img->channel;c++)for(int j=0;j<img->height;j++)
        memset(img->data[c][j],0,img->width*sizeof(unsigned char));
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

void mImageThreshold(MImage *src,MImage *dst,int *thresh,int value1,int value2)
{
    mException((INVALID_IMAGE(src)),EXIT,"invalid input");
    if(INVALID_POINTER(dst)) dst=src;
    if(value1<0) value1=0;
    if(value2<0) value2=255;
    
    for(int j=ImageY1(dst);j<ImageY2(dst);j++)
        for(int i=ImageX1(dst,j);i<ImageX2(dst,j);i++)
        {
            int c;for(c=0;c<src->channel;c++) if(src->data[c][j][i]<thresh[c]) break;
            dst->data[0][j][i]=(c==src->channel)?value2:value1;
        }
    dst->channel = 1;
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

void mImageDataWeightAdd(MImage *src1,MImage *src2,MImage *dst,float w1,float w2)
{
    mException((INVALID_IMAGE(src1)||INVALID_IMAGE(src2)),EXIT,"invalid input");
    mException(((src1->height != src2->height)||(src1->width != src2->width)),EXIT,"invalid input");
    mException((src2->channel!=src1->channel)&&(src2->channel!=1),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) {dst = src1;}
    else {mImageRedefine(dst,src1->channel,src1->height,src1->width,dst->data);}
    
         if(!INVALID_POINTER(src1->border)) dst->border = src1->border;
    else if(!INVALID_POINTER(src2->border)) dst->border = src2->border;

    int v1[256];int v2[256];
    for(int i=0;i<256;i++){v1[i]=(int)((float)i*w1); v2[i]=(int)((float)i*w2);}
    
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
                register int data = v1[data1[j][i]]+v2[data2[j][i]];
                dst->data[k][j][i] = (data>255)?255:((data<0)?0:data);
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
    // #pragma omp parallel for
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
    int flag[64];
    MImage *img[64];
    int n;
};
void endImageChannelSplit(void *info)
{
    struct HandleImageChannelSplit *handle = (struct HandleImageChannelSplit *)info;
    for(int i=0;i<64;i++)
    {
        if(handle->img[i]!=NULL)
            mImageRelease(handle->img[i]);
    }
}
#define HASH_ImageChannelSplit 0x41e09e5b
MImage *mImageChannelSplit(MImage *src,int num,...)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input image");
    mException((num>=4),EXIT,"invalid input");
     
    MHandle *hdl=mHandle(src,ImageChannelSplit);
    struct HandleImageChannelSplit *handle = (struct HandleImageChannelSplit *)(hdl->handle);
    hdl->valid = 1;

    int cn[4]={0,0,0,0}; unsigned char **data[4];
    va_list para;
    va_start(para,num);
    for(int i=0;i<num;i++) {cn[i]=va_arg(para,int);mException((cn[i]>=4),EXIT,"invalid input channel");}
    va_end(para);
    
    int flag = ((cn[0]+1)<<24)+((cn[1]+1)<<16)+((cn[2]+1)<<8)+(cn[3]+1);
    for(int i=0;i<handle->n;i++) if(flag==handle->flag[i]) return handle->img[i];

    int n=handle->n;
    handle->img [n]=mImageCreate(num,src->height,src->width,data);
    handle->flag[n]=flag;
    handle->n=n+1;
    return handle->img[n];
}

