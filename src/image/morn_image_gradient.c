/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_image.h"
#include "morn_image_caculate.h"

#define GRADIENT1(X,Y) ABS((sdata[CN][Y-r][X  -1]-sdata[CN][Y+r][X  -1])*2+(sdata[CN][Y-r][X  ]-sdata[CN][Y+r][X  ])*3+(sdata[CN][Y-r  ][X+1]-sdata[CN][Y+r  ][X+1])*2)
#define GRADIENT2(X,Y) ABS((sdata[CN][Y-r][X+r-1]-sdata[CN][Y+r][X-r+1])*2+(sdata[CN][Y-r][X+r]-sdata[CN][Y+r][X-r])*3+(sdata[CN][Y-r+1][X+r]-sdata[CN][Y+r-1][X-r])*2)
#define GRADIENT3(X,Y) ABS((sdata[CN][Y-1][X-r  ]-sdata[CN][Y-1][X+r  ])*2+(sdata[CN][Y  ][X-r]-sdata[CN][Y  ][X+r])*3+(sdata[CN][Y  +1][X-r]-sdata[CN][Y  +1][X+r])*2)
#define GRADIENT4(X,Y) ABS((sdata[CN][Y-r][X-r+1]-sdata[CN][Y+r][X+r-1])*2+(sdata[CN][Y-r][X-r]-sdata[CN][Y+r][X+r])*3+(sdata[CN][Y-r+1][X-r]-sdata[CN][Y+r-1][X+r])*2)

void mImageDirection(MImage *src,MImage *dst,int r,int thresh)
{
    mException((r>4),EXIT,"invalid region size");
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(1,src->height,src->width,NULL);
    else
        mImageRedefine(dst,1,src->height,src->width,dst->data);
    
    thresh=thresh*7;

    unsigned char ***sdata = src->data;
    
    #define ImageDirection(X,Y) {\
        register int Diff;\
        int Max = 0;\
        int Dir = 0;\
        for(int CN=0;CN<src->channel;CN++)\
        {\
            Diff = GRADIENT1(X,Y); if(Diff > thresh) {if(Diff>Max) {Max = Diff;  Dir = 1;}}\
            Diff = GRADIENT2(X,Y); if(Diff > thresh) {if(Diff>Max) {Max = Diff;  Dir = 2;}}\
            Diff = GRADIENT3(X,Y); if(Diff > thresh) {if(Diff>Max) {Max = Diff;  Dir = 3;}}\
            Diff = GRADIENT4(X,Y); if(Diff > thresh) {if(Diff>Max) {Max = Diff;  Dir = 4;}}\
        }\
        dst->data[0][Y][X] = Dir;\
    }
    
    mImageRegion(src,r,ImageDirection);
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageDirectionGradient(MImage *src,MImage *dst,int direction,int r,int thresh)
{
    mException((r>4),EXIT,"invalid region size");
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(1,src->height,src->width,NULL);
    else
        mImageRedefine(dst,1,src->height,src->width,dst->data);
    
    thresh=thresh*7;
  
    int src_cn = src->channel;
    unsigned char ***sdata = src->data;
    
    #define ImageDirectionGradient(X,Y) {\
        register int Diff;\
        int Rst = 0;\
             if(direction == 1) for(int CN=0;CN<src_cn;CN++) {Diff = GRADIENT1(X,Y);  Rst = MAX(Rst,Diff);}\
        else if(direction == 2) for(int CN=0;CN<src_cn;CN++) {Diff = GRADIENT2(X,Y);  Rst = MAX(Rst,Diff);}\
        else if(direction == 3) for(int CN=0;CN<src_cn;CN++) {Diff = GRADIENT3(X,Y);  Rst = MAX(Rst,Diff);}\
        else if(direction == 4) for(int CN=0;CN<src_cn;CN++) {Diff = GRADIENT4(X,Y);  Rst = MAX(Rst,Diff);}\
        dst->data[0][Y][X] = (Rst > thresh)?(Rst/7):0;\
    }\
    
    mImageRegion(src,r,ImageDirectionGradient);
            
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageGradientValue(MImage *src,MImage *dst,int r,int thresh)
{
    mException((r>4),EXIT,"invalid region size");
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(1,src->height,src->width,NULL);
    else
        mImageRedefine(dst,1,src->height,src->width,dst->data);
    
    thresh=thresh*7;
    
    int src_cn = src->channel;
    unsigned char ***sdata = src->data;
    
    #define ImageGradientValue(X,Y) {\
        register int Diff;\
        int Max = 0;\
        for(int CN=0;CN<src_cn;CN++)\
        {\
            Diff = GRADIENT1(X,Y); if(Diff>Max) Max = Diff;\
            Diff = GRADIENT2(X,Y); if(Diff>Max) Max = Diff;\
            Diff = GRADIENT3(X,Y); if(Diff>Max) Max = Diff;\
            Diff = GRADIENT4(X,Y); if(Diff>Max) Max = Diff;\
        }\
        dst->data[0][Y][X] = (Max>thresh)?(Max/7):0;\
    }
    
    mImageRegion(src,r,ImageGradientValue);
            
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageGradientCheck(MImage *src,MImage *dst,int r,int thresh)
{
    mException((r>4),EXIT,"invalid region size");
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(1,src->height,src->width,NULL);
    else
        mImageRedefine(dst,1,src->height,src->width,dst->data);
    
    thresh=thresh*7;

    int src_cn = src->channel;
    unsigned char ***sdata = src->data;
                
    #define ImageGradientCheck(X,Y) {\
        register int Diff;\
        dst->data[0][Y][X] = 0;\
        for(int CN=0;CN<src_cn;CN++)\
        {\
            Diff = GRADIENT1(X,Y); if(Diff>thresh) {dst->data[0][Y][X] = 255; break;}\
            Diff = GRADIENT2(X,Y); if(Diff>thresh) {dst->data[0][Y][X] = 255; break;}\
            Diff = GRADIENT3(X,Y); if(Diff>thresh) {dst->data[0][Y][X] = 255; break;}\
            Diff = GRADIENT4(X,Y); if(Diff>thresh) {dst->data[0][Y][X] = 255; break;}\
        }\
    }
    
    mImageRegion(src,r,ImageGradientCheck);
            
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageGradientAdaptCheck(MImage *src,MImage *dst,int r,float thresh)
{
    mException((r>4),EXIT,"invalid region size");
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(1,src->height,src->width,NULL);
    else
        mImageRedefine(dst,1,src->height,src->width,dst->data);
    
    unsigned char t[256];
    for(int i=0;i<256;i++)
    {
        t[i] = (int)(((float)i)*thresh+0.5);
        if(t[i]<5) t[i] = 5;
    }
    
    int src_cn = src->channel;
    unsigned char ***sdata = src->data;
    
    #define ImageGradientAdaptCheck(X,Y) {\
        register int Diff;\
        dst->data[0][Y][X] = 0;\
        for(int CN=0;CN<src_cn;CN++)\
        {\
            unsigned char data = sdata[CN][Y][X];\
            Diff = GRADIENT1(X,Y); if(Diff>t[data]) {dst->data[0][Y][X] = 255; break;}\
            Diff = GRADIENT2(X,Y); if(Diff>t[data]) {dst->data[0][Y][X] = 255; break;}\
            Diff = GRADIENT3(X,Y); if(Diff>t[data]) {dst->data[0][Y][X] = 255; break;}\
            Diff = GRADIENT4(X,Y); if(Diff>t[data]) {dst->data[0][Y][X] = 255; break;}\
        }\
    }
       
    mImageRegion(src,r,ImageGradientAdaptCheck);
            
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageGradient(MImage *src,MImage *direction,MImage *value,int r,int thresh)
{
    mException((r>4),EXIT,"invalid region size");
    mException((direction == NULL)&&(value==NULL),EXIT,"invalid input");
    if(value == NULL)
    {
        mImageDirection(src,direction,r,thresh);
        return;
    }
    if(direction == NULL)
    {
        mImageGradientValue(src,value,r,thresh);
        return;
    }
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    MImage *p_dir = direction;
    if((INVALID_POINTER(direction))||(direction==src))
        direction = mImageCreate(1,src->height,src->width,NULL);
    else
        mImageRedefine(direction,1,src->height,src->width,direction->data);
    
    MImage *p_value = value;
    if((INVALID_POINTER(value))||(value==src))
        value = mImageCreate(1,src->height,src->width,NULL);
    else
        mImageRedefine(value,1,src->height,src->width,value->data);
    
    thresh=thresh+thresh+thresh;
    
    int src_cn = src->channel;
    unsigned char ***sdata = src->data;
    
    #define ImageGradient(X,Y) {\
        register int Diff;\
        int Max = 0;\
        int Dir = 0;\
        for(int CN=0;CN<src_cn;CN++)\
        {\
            Diff = GRADIENT1(X,Y); if(Diff > thresh) {if(Diff>Max) {Max = Diff;  Dir = 1;}}\
            Diff = GRADIENT2(X,Y); if(Diff > thresh) {if(Diff>Max) {Max = Diff;  Dir = 2;}}\
            Diff = GRADIENT3(X,Y); if(Diff > thresh) {if(Diff>Max) {Max = Diff;  Dir = 3;}}\
            Diff = GRADIENT4(X,Y); if(Diff > thresh) {if(Diff>Max) {Max = Diff;  Dir = 4;}}\
        }\
        direction->data[0][Y][X] = Dir;\
        value ->data[0][Y][X] = Max/3;\
    }
    
    mImageRegion(src,r,ImageGradient);
    
    if(p_dir!=direction) { mImageExchange(src,direction); mImageRelease(direction);}
    if(p_value!=value) { mImageExchange(src,value); mImageRelease(value);}
}


void mImageGradientFilter(MImage *dir,MImage *value,MImage *ddst,MImage *vdst,int r)
{
    mException((r>4),EXIT,"invalid region size");
    
    mImageExpand(dir,r,MORN_BORDER_REFLECT);
    mImageExpand(value,r,MORN_BORDER_REFLECT);
    
    MImage *p_dir = ddst;
    if((INVALID_POINTER(ddst))||(ddst==dir))
        ddst = mImageCreate(1,dir->height,dir->width,NULL);
    else
        mImageRedefine(ddst,1,dir->height,dir->width,ddst->data);
    
    MImage *p_value = vdst;
    if((INVALID_POINTER(vdst))||(vdst==value))
        vdst = mImageCreate(1,value->height,value->width,NULL);
    else
        mImageRedefine(vdst,1,value->height,value->width,vdst->data);
    
    unsigned char **ddata = dir->data[0];
    unsigned char **vdata = value->data[0];
    
    #define ImageGradientFilter(X,Y) {\
        int Value1 = 0;int Value2 = 0;int Value3 = 0;int Value4 = 0;\
        int Max;\
        for(int N=-r;N<=r;N++)\
        {\
                 if(ddata[Y  ][X+N] == 1) {Value1 +=vdata[Y  ][X+N];}\
            else if(ddata[Y+N][X+N] == 2) {Value2 +=vdata[Y+N][X+N];}\
            else if(ddata[Y+N][X  ] == 3) {Value3 +=vdata[Y+N][X  ];}\
            else if(ddata[Y+N][X-N] == 4) {Value4 +=vdata[Y+N][X-N];}\
        }\
        Max = MAX(Value1,Value2);Max = MAX(Max,Value3);Max = MAX(Max,Value4);\
             if(Max==Value1) {ddata[Y][X] = 1;vdata[Y][X] = Value1/(r+1+r);}\
        else if(Max==Value2) {ddata[Y][X] = 2;vdata[Y][X] = Value2/(r+1+r);}\
        else if(Max==Value3) {ddata[Y][X] = 3;vdata[Y][X] = Value3/(r+1+r);}\
        else if(Max==Value4) {ddata[Y][X] = 4;vdata[Y][X] = Value4/(r+1+r);}\
    }
               
    mImageRegion(value,r,ImageGradientFilter);
            
    if(p_dir!=dir) { mImageExchange(dir,ddst); mImageRelease(ddst);}
    if(p_value!=value) { mImageExchange(value,vdst); mImageRelease(vdst);}
}

void mImageGradientSuppression(MImage *dir,MImage *value,MImage *dst,int r)
{
    mException((r>4),EXIT,"invalid region size");
    
    mImageExpand(dir,r,MORN_BORDER_REFLECT);
    mImageExpand(value,r,MORN_BORDER_REFLECT);
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==value))
        dst = mImageCreate(1,value->height,value->width,NULL);
    else
        mImageRedefine(dst,1,value->height,value->width,dst->data);
    
    unsigned char **ddata = dir->data[0];
    unsigned char **vdata = value->data[0];
    unsigned char **dst_data = dst->data[0];
                
    #define ImageGradientSuppression(X,Y) {\
        if(ddata[Y][X] == 0) {dst_data[Y][X] = 0;continue;}\
        register int Data = vdata[Y][X]; int N;\
             if(ddata[Y][X] == 1) for(N=1;N<r+1;N++) {if(Data<vdata[Y-N][X  ]) break;if(Data<vdata[Y+N][X  ]) break;}\
        else if(ddata[Y][X] == 2) for(N=1;N<r+1;N++) {if(Data<vdata[Y-N][X+N]) break;if(Data<vdata[Y+N][X-N]) break;}\
        else if(ddata[Y][X] == 3) for(N=1;N<r+1;N++) {if(Data<vdata[Y  ][X-N]) break;if(Data<vdata[Y  ][X+N]) break;}\
        else /*ddata[Y][X] == 4*/ for(N=1;N<r+1;N++) {if(Data<vdata[Y-N][X-N]) break;if(Data<vdata[Y+N][X+N]) break;}\
        dst_data[Y][X] = (N==r+1)?255:0;\
    }
    mImageRegion(value,r,ImageGradientSuppression);
    
    if(p!=dst) { mImageExchange(value,dst); mImageRelease(dst);}
}
    
void mImageCanny(MImage *src,MImage *dst,int r,int thresh)
{
    int height,width;
    MImage *dir;
    MImage *value;
    
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    width = src->width;
    height = src->height;
    
    if(dst == NULL)
        dst = src;
    
    dir = mImageCreate(1,height,width,NULL);
    value = mImageCreate(1,height,width,NULL);
    // dflt = mImageCreate(1,height,width,NULL);
    // vflt = mImageCreate(1,height,width,NULL);
    
    mImageGradient(src,dir,value,r,thresh);
    // mImageGradientFilter(gdt,flt,2);
    // printf("gdt->data[819][1023] is %d,%d\n",gdt->data[0][819][1023],gdt->data[1][819][1023]);
    // printf("gdt->data[819][1023] %d,%d,%d,%d,%d,%d\n",gdt->data[0][816][1020]
                                                    // ,gdt->data[0][817][1021]
                                                    // ,gdt->data[0][818][1022]
                                                    // ,gdt->data[0][820][1024]
                                                    // ,gdt->data[0][821][1025]
                                                    // ,gdt->data[0][822][1026]);
   
    // gdt->channel = 1;
    // gdt->info.image_type = MORN_IMAGE_GRAY;
    // BMPSave(gdt,"./test13_out2.bmp");
    // gdt->channel = 2;
    
    mImageRedefine(dst,1,height,width,dst->data);
    mImageGradientSuppression(dir,value,dst,r+1);
    
    // printf("dst->data[819][1023] is %d\n",dst->data[0][819][1023]);
    
    mImageRelease(dir);
    mImageRelease(value);
    // mImageRelease(dflt);
    // mImageRelease(vflt);
}
