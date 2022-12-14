/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_image.h"
#include "morn_image_caculate.h"

struct HandleImageCreate
{
    MImage *img;
    MChain *property;
    int64_t reserve[8];
    int writeable;
    
    int cn;
    int height;
    int width;
    unsigned char **index;
    MMemory *memory;

    unsigned char **backup_index;
    MMemory *backup_memory;
    unsigned char **backup_data[MORN_MAX_IMAGE_CN];

    int image_type;
    int border_type;
};
void endImageCreate(struct HandleImageCreate *handle) 
{
    mException((handle->img == NULL),EXIT,"invalid image");
    if(handle->property!=NULL) mChainRelease(handle->property);
    if(handle->index   !=NULL) mFree(handle->index);
    if(handle->memory  !=NULL) mMemoryRelease(handle->memory);

    if(handle->backup_index !=NULL) mFree(handle->backup_index);
    if(handle->backup_memory!=NULL) mMemoryRelease(handle->backup_memory);

    memset(handle->img,0,sizeof(MImage));
}
#define HASH_ImageCreate 0xccb34f86
MImage *ImageCreate(int cn,int height,int width,unsigned char **data[])
{
    if(cn <0) {cn = 0;} if(height <0) {height = 0;} if(width <0) {width = 0;}
    mException((cn>MORN_MAX_IMAGE_CN),EXIT,"invalid input");

    MImage *img = (MImage *)ObjectAlloc(sizeof(MImage));
    img->height = height;img->width = width;img->channel = cn;
    
    MHandle *hdl=mHandle(img,ImageCreate);
    struct HandleImageCreate *handle = (struct HandleImageCreate *)(hdl->handle);
    handle->img = img;

    mPropertyVariate(img,"image_type",&(handle->image_type),sizeof(int));
         if(cn==1) handle->image_type=MORN_IMAGE_GRAY;
    else if(cn==3) handle->image_type=MORN_IMAGE_RGB ;
    else if(cn==4) handle->image_type=MORN_IMAGE_RGBA;
    else           handle->image_type=DFLT;

    mPropertyVariate(img,"border_type",&(handle->border_type),sizeof(int));
    handle->border_type=DFLT;
    if((cn==0)||(height == 0))
    {
        mException((!INVALID_POINTER(data)),EXIT,"invalid input");
        return img;
    }

    if(!INVALID_POINTER(data))
    {
        memcpy(img->data,data,cn*sizeof(unsigned char **));
        handle->border_type = MORN_BORDER_INVALID;
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
        handle->border_type = MORN_BORDER_INVALID;
        return img;
    }
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(1,cn*row*col*sizeof(unsigned char),MORN_HOST);
    mException(handle->memory->num!=1,EXIT,"invalid image memory");
    mMemoryIndex(handle->memory,cn*row,col*sizeof(unsigned char),(void ***)(&(handle->index)),1);
    handle->width = width;
    mPropertyFunction(img,"device",mornMemoryDevice,handle->memory);
    
    for(int i=0;i<cn*row;i++) handle->index[i] = &(handle->index[i][16]);
    for(int k=0;k<cn;k++) img->data[k] = handle->index + k*row+8;

    handle->border_type = MORN_BORDER_UNDEFINED;
    
    return img;
}

void ImageRedefine(MImage *img,int cn,int height,int width,unsigned char **data[])
{
    mException((INVALID_POINTER(img)),EXIT,"invalid input");
    
    if(cn<=0)                 cn    =img->channel;
    if(height <= 0)           height=img->height;
    if(width<=0)              width =img->width;
    
    if((cn!=img->channel)||(height!=img->height)||(width!=img->width)) mHandleReset(img);
    
    int same_size = ((cn<=img->channel)&&(height<=img->height)&&(width<=img->width));
    int reuse = (data==img->data);
    int flag = (img->channel&&img->height&&img->width);
    
    mException((cn>MORN_MAX_IMAGE_CN),EXIT,"invalid input channel");
    struct HandleImageCreate *handle = (struct HandleImageCreate *)(ObjHandle(img,0)->handle);
    if(cn!= img->channel)
    {
             if(cn==1) handle->image_type=MORN_IMAGE_GRAY;
        else if(cn==3) handle->image_type=MORN_IMAGE_RGB ;
        else if(cn==4) handle->image_type=MORN_IMAGE_RGBA;
    }
    img->channel = cn;
    img->height = height;
    img->width = width;
    
    if(same_size&&reuse) return;
    if(same_size&&INVALID_POINTER(data)&&(handle->width >0)) return;
    // 
    mException(reuse&&flag&&(handle->width==0),EXIT,"invalid redefine");
    
    handle->width = 0;
    
    img->border = NULL;
    handle->border_type = MORN_BORDER_UNDEFINED;
    
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
        else handle->border_type = MORN_BORDER_INVALID;
        return;
    }
    
    if(handle->memory == NULL)
    {
        handle->memory = mMemoryCreate(1,cn*row*col*sizeof(unsigned char),MORN_HOST);
        mPropertyFunction(img,"device",mornMemoryDevice,handle->memory);
    }
    else mMemoryRedefine(handle->memory,1,cn*row*col*sizeof(unsigned char),DFLT);
    mException(handle->memory->num!=1,EXIT,"invalid image memory");
    mMemoryIndex(handle->memory,cn*row,col*sizeof(unsigned char),(void ***)(&(handle->index)),1);
    
    for(int i=0;i<cn*row;i++) handle->index[i] = &(handle->index[i][16]);
    handle->width = width;
    
    for(int k=0;k<cn;k++) img->data[k] = handle->index + k*row+8;
}

void mImageRelease(MImage *img)
{
    ObjectFree(img);
}

void MImageDataExchange(MImage *img1,MImage *img2)
{
    struct HandleImageCreate *handle1 = (struct HandleImageCreate *)(ObjHandle(img1,0)->handle);
    struct HandleImageCreate *handle2 = (struct HandleImageCreate *)(ObjHandle(img2,0)->handle);
    int buff;void *p[4];
    buff=img1->channel;img1->channel=img2->channel;img2->channel=buff;
    buff=img1->width  ;img1->width  =img2->width  ;img2->width  =buff;
    buff=img1->height ;img1->height =img2->height ;img2->height =buff;
    memcpy(p         ,img1->data,4*sizeof(uint8_t *));
    memcpy(img1->data,img2->data,4*sizeof(uint8_t *));
    memcpy(img2->data,p         ,4*sizeof(uint8_t *));
    
    buff=handle1->cn    ;handle1->cn    =handle2->cn    ;handle2->cn    =buff;
    buff=handle1->width ;handle1->width =handle2->width ;handle2->width =buff;
    buff=handle1->height;handle1->height=handle2->height;handle2->height=buff;
    p[0]=handle1->index ;handle1->index =handle2->index ;handle2->index =p[0];
    p[0]=handle1->memory;handle1->memory=handle2->memory;handle2->memory=p[0];
    
    buff=handle1-> image_type;handle1-> image_type=handle2-> image_type;handle2-> image_type=buff;
    buff=handle1->border_type;handle1->border_type=handle2->border_type;handle2->border_type=buff;
}

unsigned char ***mImageBackup(MImage *img,int cn,int height,int width)
{
    if(cn    <=0) cn    =img->channel;
    if(height<=0) height=img->height;
    if(width <=0) width =img->width;

    int col = width + 32;
    int row = height+ 16;
    
    struct HandleImageCreate *handle = (struct HandleImageCreate *)(ObjHandle(img,0)->handle);
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
    int *img_border_type = (int *)mPropertyRead(img,"border_type");
    mException((*img_border_type == MORN_BORDER_INVALID),EXIT,"image expand is invalid");
    
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
    
    if(*img_border_type == MORN_BORDER_IMAGE)
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

    mPropertyWrite(img,"border_type",&border_type,sizeof(int));
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


void mImageCopy(MImage *src,MImage *dst)
{
    int i;
    if(src==dst) return;

    mException((INVALID_IMAGE(src)||INVALID_POINTER(dst)),EXIT,"invalid input");
    mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    dst->border = src->border;
    // dst->info = src->info;
    
    for(int cn=0;cn<src->channel;cn++)
    {
        #pragma omp parallel for
        for(i=0;i<src->height;i++)
            memcpy(dst->data[cn][i],src->data[cn][i],src->width*sizeof(unsigned char));
    }
}

// void m_ImageWipe(MImage *img,int channel)
// {
//     char f[MORN_MAX_IMAGE_CN];
//     if(channel<0) {memset(f,1,MORN_MAX_IMAGE_CN*sizeof(char));}
//     else {mException(channel>=img->channel,EXIT,"invalid input");memset(f,0,MORN_MAX_IMAGE_CN*sizeof(char));f[channel]=1;}
    
//     for(int c=0;c<img->channel;c++)
//     {
//         if(f[c]==0) continue;
//         for(int j=0;j<img->height;j++)
//             memset(img->data[c][j],0,img->width*sizeof(unsigned char));
//     }
// }

void m_ImageWipe(MImage *img,int channel,MImageRect *rect)
{
    char f[MORN_MAX_IMAGE_CN];
    if(channel<0) {memset(f,1,MORN_MAX_IMAGE_CN*sizeof(char));}
    else {mException(channel>=img->channel,EXIT,"invalid input");memset(f,0,MORN_MAX_IMAGE_CN*sizeof(char));f[channel]=1;}

    int x,y,width,height;
    if(rect==NULL) {x=0;y=0;width=img->width;height=img->height;}
    else           
    {
        x=MIN(rect->x1,rect->x2);y=MIN(rect->y1,rect->y2);width=ABS(rect->x2-rect->x1);height=ABS(rect->y2-rect->y1);
        if(x>=img->width ) {return;} if(x<0) {width =width +x;x=0;}
        if(y>=img->height) {return;} if(y<0) {height=height+y;y=0;}
        if(width >=img->width ) width =img->width -x;
        if(height>=img->height) height=img->height-y;
    }
    
    for(int c=0;c<img->channel;c++)
    {
        if(f[c]==0) continue;
        for(int j=y;j<y+height;j++)
            memset(img->data[c][j]+x,0,width*sizeof(unsigned char));
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
       
    for(int k=0;k<dst->channel;k++)
    {
        unsigned char **data1 = src1->data[k];
        unsigned char **data2 = (k>=src2->channel)?src2->data[0]:src2->data[k];
        
        int j;
        #pragma omp parallel for
        for(j=ImageY1(dst);j<ImageY2(dst);j++)
        {
            #define ImageAdd(i,o) {dst->data[k][j][i+o] = MIN(data1[j][i+o]-data2[j][i+o],255);}
            RowCalculate(ImageAdd,dst->width);
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
    
    for(int k=0;k<dst->channel;k++)
    {
        unsigned char **data1 = src1->data[k];
        unsigned char **data2 = (k>=src2->channel)?src2->data[0]:src2->data[k];
        
        int j;
        #pragma omp parallel for
        for(j=0;j<dst->height;j++)
        {
            #define ImageWeightAdd(i,o) {\
                int rst=v1[data1[j][i]]+v2[data2[j][i]];\
                     if(rst<  0)dst->data[k][j][i]=0  ;\
                else if(rst>255)dst->data[k][j][i]=255;\
                else            dst->data[k][j][i]=rst;\
            }
            RowCalculate(ImageWeightAdd,dst->width);
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
    
    for(int k=0;k<dst->channel;k++)
    {
        unsigned char **data1 = src1->data[k];
        unsigned char **data2 = (k>=src2->channel)?src2->data[0]:src2->data[k];
        
        int j;
        #pragma omp parallel for
        for(j=0;j<dst->height;j++)
        {
            #define ImageSub(i,o) {dst->data[k][j][i+o] = MAX(data1[j][i+o]-data2[j][i+o],0);}
            RowCalculate(ImageSub,dst->width);
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
    
    for(int k=0;k<dst->channel;k++)
    {
        unsigned char **data1 = src1->data[k];
        unsigned char **data2 = (k>=src2->channel)?src2->data[0]:src2->data[k];
        
        int j;
        #pragma omp parallel for
        for(j=0;j<dst->height;j++)
        {
            #define ImageAnd(i,o) {dst->data[k][j][i+o] = data1[j][i+o]&data2[j][i+o];}
            RowCalculate(ImageAnd,dst->width);
        }
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
    
    for(int k=0;k<dst->channel;k++)
    {
        unsigned char **data1 = src1->data[k];
        unsigned char **data2 = (k>=src2->channel)?src2->data[0]:src2->data[k];
        
        int j;
        #pragma omp parallel for
        for(j=0;j<dst->height;j++)
        {
            #define ImageOr(i,o) {dst->data[k][j][i+o] = data1[j][i+o]|data2[j][i+o];}
            RowCalculate(ImageOr,dst->width);
        }
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
        for(j=0;j<dst->height;j++)
        {
            #define ImageInvert(i,o) {dst->data[k][j][i+o] = 255-src->data[k][j][i+o];}
            RowCalculate(ImageInvert,dst->width);
        }
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
        for(j=0;j<dst->height;j++)
        {
            #define ImageLinearMap(i,o) {dst->data[cn][j][i+o] = data[src->data[cn][j][i+o]];}
            RowCalculate(ImageLinearMap,dst->width);
        }
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
    for(j=0;j<dst->height;j++)for(int i=ImageX1(dst,j);i<ImageX2(dst,j);i++)
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
void endImageChannelSplit(struct HandleImageChannelSplit *handle)
{
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

void mImageMask(MImage *in1,MImage *in2,MImage *mask,MImage *out)
{
    if(in2==NULL) in2=mask;
    if(mask==NULL) mask=in2;
    mException(INVALID_IMAGE(in1)||INVALID_IMAGE(in2),EXIT,"invalid input image");
    int height=in1->height;int width=in1->width;
    mException((in1->channel!=in2->channel)||(height!=in2->height)||(height!=mask->height)||(width!=in2->width)||(width!=mask->width),EXIT,"invalid image size");
    if(out==NULL) out=in1;
    if((out!=in1)&&(out!=in2)) mImageRedefine(out,in1->channel,height,width);
    
    for(int c=0;c<in1->channel;c++)
    {
        uint8_t **m=(mask->channel>=c)?mask->data[0]:mask->data[c];
        for(int j=0;j<height;j++)
        {
            uint8_t *p1=in1->data[c][j];uint8_t *p2=in2->data[c][j];uint8_t *pout=out->data[c][j];
            #define ImageMask(i,o) {pout[i+o]=m[j][i+o]?p1[i+o]:p2[i+o];}
            RowCalculate(ImageMask,width);
        }
    }
}

struct HandleImageROI
{
    int x0,y0,x1,y1;
    MImage *roi;
    int img_height;
    uint8_t **data[4];
    uint8_t ***img_data;
};
void endImageROI(struct HandleImageROI *handle)
{
    if(handle->roi!=NULL) mImageRelease(handle->roi);
    if(handle->data[0]!=NULL) mFree(handle->data[0]);
}
#define HASH_ImageROI 0xf83d8c22
MImage *m_ImageROI(MImage *img,int x0,int y0,int x1,int y1)
{
    mException(INVALID_IMAGE(img),EXIT,"invalid image");
    mException((x0<0)||(x1>=img->width)||(x0>=x1)||(y0<0)||(y1>=img->height)||(y0>=y1),EXIT,"invalid ROI region");
    MHandle *hdl = mHandle(img,ImageROI);
    struct HandleImageROI *handle=hdl->handle;
    if(handle->img_height>img->height) {mFree(handle->data[0]);handle->data[0]=NULL;hdl->valid=0;}
    if(!mHandleValid(hdl))
    {
        if(handle->roi==NULL) handle->roi=mImageCreate();
        if(handle->data[0]==NULL) handle->data[0]=mMalloc(img->height*4*sizeof(uint8_t *));
        handle->data[1]=handle->data[0]+img->height;
        handle->data[2]=handle->data[1]+img->height;
        handle->data[3]=handle->data[2]+img->height;
        handle->img_height=img->height;
        hdl->valid=1;
    }
    int valid=1;
    if(handle->img_data!=img->data) valid=0;
    else if((handle->x0!=x0)||(handle->y0!=y0)||(handle->x1!=x1)||(handle->y1!=y1)) valid=0;
    if(valid==1) return handle->roi;
    
    for(int c=0;c<img->channel;c++)
    {
        for(int j=0;j<y1-y0;j++) handle->data[c][j]=img->data[c][y0+j]+x0;
    }
    mImageRedefine(handle->roi,img->channel,y1-y0,x1-x0,handle->data);
    handle->x0=x0;handle->y0=y0;handle->x1=x1;handle->y1=y1;handle->img_data=img->data;
    return handle->roi;
}

struct HandleMornImage
{
    MImage *buff0;
    MImage *buff1;
    MImage *buff2;
};
void endMornImage(struct HandleMornImage *handle)
{
    if(handle->buff0!=NULL) mImageRelease(handle->buff0);
    if(handle->buff1!=NULL) mImageRelease(handle->buff1);
    if(handle->buff2!=NULL) mImageRelease(handle->buff2);
}
#define HASH_MornImage 0xbc9d0150
struct HandleMornImage *morn_Morn_image_handle = NULL;
struct HandleMornImage *mornImageInit()
{
    if(morn_Morn_image_handle!=NULL) return morn_Morn_image_handle;
    MHandle *hdl = mHandle("Morn",MornImage);
    morn_Morn_image_handle = hdl->handle;
    hdl->valid=1;
    return morn_Morn_image_handle;
}
MImage *mornImageBuff(int channel,int height,int width)
{
    struct HandleMornImage *handle = mornImageInit();
    if(handle->buff0==NULL) handle->buff0 = mImageCreate(channel,height,width);
    else                    mImageRedefine(handle->buff0,channel,height,width);
    return handle->buff0;
}
MImage *mornImageBuff1(int channel,int height,int width)
{
    struct HandleMornImage *handle = mornImageInit();
    if(handle->buff1==NULL) handle->buff1 = mImageCreate(channel,height,width);
    else                    mImageRedefine(handle->buff1,channel,height,width);
    return handle->buff1;
}
MImage *mornImageBuff2(int channel,int height,int width)
{
    struct HandleMornImage *handle = mornImageInit();
    if(handle->buff2==NULL) handle->buff2 = mImageCreate(channel,height,width);
    else                    mImageRedefine(handle->buff2,channel,height,width);
    return handle->buff2;
}




