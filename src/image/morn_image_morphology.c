#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Image.h"
#include "morn_ImageCaculate.h"

void mImageDilation(MImage *src,MImage *dst,int r,int stride)
{
    if(r<=0) r=1;if(stride <=0)stride=1;
    mException((r>7),EXIT,"invalid region size");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
   
    
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        
        #define ImageDilation(X,Y) {\
            unsigned char Max = 0;\
            for(int M=Y-r;M<=Y+r;M+=stride)\
                for(int N=X-r;N<=X+r;N+=stride)\
                    if(sdata[M][N] > Max)\
                        Max = sdata[M][N];\
            ddata[Y][X] = Max;\
        }
        
        mImageRegion(src,r,ImageDilation);
    }
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageErosion(MImage *src,MImage *dst,int r,int stride)
{
    if(r<=0) r=1;if(stride <=0)stride=1;
    mException((r>7),EXIT,"invalid region size");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        
        #define ImageErosion(X,Y) {\
            unsigned char Min = 255;\
            for(int M=Y-r;M<=Y+r;M+=stride)\
                for(int N=X-r;N<=X+r;N+=stride)\
                    if(sdata[M][N] < Min)\
                        Min = sdata[M][N];\
            ddata[Y][X] = Min;\
        }
        
        mImageRegion(src,r,ImageErosion);
    }
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageMorphologySmooth(MImage *src,MImage *dst,int r,int stride)
{
    if(r<=0) r=1;if(stride <=0)stride=1;
    mException((r>7),EXIT,"invalid region size");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    int num = (r+r+1)/stride;num=num*num;
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        
        #define MorphologySmooth(X,Y) {\
            unsigned char Min = 255;\
            unsigned char Max = 0;\
            int Sum = 0;\
            for(int M=Y-r;M<=Y+r;M+=stride)\
                for(int N=X-r;N<=X+r;N+=stride)\
                {\
                    Sum += sdata[M][N];\
                    Min = MIN(sdata[M][N],Min);\
                    Max = MIN(sdata[M][N],Min);\
                }\
            ddata[Y][X]=(Sum/num > sdata[Y][X])?Max:Min;\
        }
        
        mImageRegion(src,r,MorphologySmooth);
    }
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageMorphologySharp(MImage *src,MImage *dst,int r,int stride)
{
    if(r<=0) r=1;if(stride <=0)stride=1;
    mException((r>7),EXIT,"invalid region size");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    int num = (r+r+1)/stride;num=num*num;
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        
        #define MorphologySharp(X,Y) {\
            unsigned char Min = 255;\
            unsigned char Max = 0;\
            int Sum = 0;\
            for(int M=Y-r;M<=Y+r;M+=stride)\
                for(int N=X-r;N<=X+r;N+=stride)\
                {\
                    Sum += sdata[M][N];\
                    Min = MIN(sdata[M][N],Min);\
                    Max = MIN(sdata[M][N],Min);\
                }\
            ddata[Y][X]=(Sum/num > sdata[Y][X])?Min:Max;\
        }
        
        mImageRegion(src,r,MorphologySharp);
    }
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}





// void mImageDilation(MImage *src,MImage *dst,int r)
// {
    // MImage *p;
    
    // mException(INVALID_IMAGE(src),"invalid input",EXIT);
    // mException((r>16),"invalid input",EXIT);
   
    // p = dst;
    // if((INVALID_POINTER(dst))||(dst==src))
        // dst = mImageCreate(src->channel,src->height,src->width,NULL);
    // else
        // mImageRedefine(dst,src->channel,src->height,src->width);
   
    // #define DILATION(Src,Rst0,Rst1) {\
        // int i,j;\
        // unsigned char Max=0;\
        // for(i=0;i<r+1+r;i++)\
            // for(j=0;j<r+1+r;j++)\
                // Max = ((Src)[i][j]>Max)?(Src)[i][j]:Max;\
        // Rst0 = Max;\
    // }\
   
    // if(r==1)      {IMAGE_ZONE1(src,dst->data[CN],dst->data[CN],DILATION);}
    // else if(r==2) {IMAGE_ZONE2(src,dst->data[CN],dst->data[CN],DILATION);}
    // else          {IMAGE_ZONE(src,dst->data[CN],dst->data[CN],r,DILATION);}
    
    // if(p!=dst)
    // {
        // mImageExchange(src,dst);
        // mImageRelease(dst);
    // }
// }

// void mImageErosion(MImage *src,MImage *dst,int r)
// {
    // MImage *p;
    
    // mException(INVALID_IMAGE(src),"invalid input",EXIT);
    // mException((r>16),"invalid input",EXIT);
   
    // p = dst;
    // if((INVALID_POINTER(dst))||(dst==src))
        // dst = mImageCreate(src->channel,src->height,src->width,NULL);
    // else
        // mImageRedefine(dst,src->channel,src->height,src->width);
   
    // #define EROSION(Src,Rst0,Rst1) {\
        // int i,j;\
        // unsigned char Min=255;\
        // for(i=0;i<r+1+r;i++)\
            // for(j=0;j<r+1+r;j++)\
                // Min = ((Src)[i][j]>Min)?(Src)[i][j]:Min;\
        // Rst0 = Min;\
    // }\
   
    // if(r==1)      {IMAGE_ZONE1(src,dst->data[CN],dst->data[CN],EROSION); }
    // else if(r==2) {IMAGE_ZONE2(src,dst->data[CN],dst->data[CN],EROSION); }
    // else          {IMAGE_ZONE(src,dst->data[CN],dst->data[CN],r,EROSION);}
    
    // if(p!=dst)
    // {
        // mImageExchange(src,dst);
        // mImageRelease(dst);
    // }
// }



