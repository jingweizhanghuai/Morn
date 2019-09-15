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

void mImageMeanFilter(MImage *src,MImage *dst,int r)
{
    mException((r>7),EXIT,"invalid region size");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    
    int area = (r+1+r)*(r+1+r);
    
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
       
        #define MeanFilter(X,Y) {\
            int Sum = 0;\
            for(int M=Y-r;M<=Y+r;M++)\
                for(int N=X-r;N<=X+r;N++)\
                    Sum += sdata[M][N];\
            ddata[Y][X] = Sum/area;\
        }
        
        mImageRegion(src,r,MeanFilter);
    }
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageMidValueFilter(MImage *src,MImage *dst)
{
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    mImageExpand(src,1,MORN_BORDER_REPLICATE);
    
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        
        #define MID(X1,X2,X3) (((X1>X2)==(X2>=X3))?X2:(((X1>X2)==(X3>=X1))?X1:X3))
        
        #define MidValueFilter(X,Y) {\
            int mid1 = MID(sdata[Y-1][X-1],sdata[Y-1][X],sdata[Y-1][X+1]);\
            int mid2 = MID(sdata[Y  ][X-1],sdata[Y  ][X],sdata[Y  ][X+1]);\
            int mid3 = MID(sdata[Y+1][X-1],sdata[Y+1][X],sdata[Y+1][X+1]);\
            ddata[Y][X] = MID(mid1,mid2,mid3);\
        }
        
        mImageRegion(src,1,MidValueFilter);
    }
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageMidValueFilter2(MImage *src,MImage *dst,int r)
{
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
        
        #define MID(X1,X2,X3) (((X1>X2)==(X2>=X3))?X2:(((X1>X2)==(X3>=X1))?X1:X3))
        
        #define MidValueFilter2(X,Y) {\
            int mid1 = MID(sdata[Y-r][X-r],sdata[Y-r][X],sdata[Y-r][X+r]);\
            int mid2 = MID(sdata[Y  ][X-r],sdata[Y  ][X],sdata[Y  ][X+r]);\
            int mid3 = MID(sdata[Y+r][X-r],sdata[Y+r][X],sdata[Y+r][X+r]);\
            ddata[Y][X] = MID(mid1,mid2,mid3);\
        }
        
        mImageRegion(src,r,MidValueFilter2);
    }
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}

void mImageConverlotion(MImage *src,MImage *dst,float *kernel,int r)
{
     mException((r>7),EXIT,"invalid region size");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(src->channel,src->height,src->width,NULL);
    else
        mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    float *knl[15];for(int i=0;i<r+r+1;i++) knl[i]=kernel+i*(r+r+1);
    
    mImageExpand(src,r,MORN_BORDER_REFLECT);
    for(int cn=0;cn<src->channel;cn++)
    {
        unsigned char **sdata = src->data[cn];
        unsigned char **ddata = dst->data[cn];
        
        #define Converlotion(X,Y) {\
            int Sum = 0;\
            for(int M=0-r;M<=r;M++)\
                for(int N=0-r;N<=r;N++)\
                    Sum += sdata[Y+M][X+N]*knl[r+M][r+N];\
                   if(Sum<0) ddata[Y][X]=0;\
            else if(Sum>255) ddata[Y][X]=255;\
            else             ddata[Y][X]=Sum;\
        }
        
        mImageRegion(src,r,Converlotion);
    }
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}
    