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
/*
#define MID(X1,X2,X3) (((X1>X2)==(X2>=X3))?X2:(((X1>X2)==(X3>=X1))?X1:X3))
#define MID_FILTER(j,i)\
{\
    int j1=j-5;int j2=j+5; if(j1<0) j1=0; if(j2>=src->height) j2 = src->height-1;\
    int i1=i-5;int i2=i+5; if(i1<0) i1=0; if(i2>=src->width ) i2 = src->width -1;\
    for(int cn=0;cn<src->channel;cn++)\
    {\
        data1 = data[cn][j1][i1];\
        data2 = data[cn][j ][i2];\
        data3 = data[cn][j2][i ];mid1 = MID(data1,data2,data3);\
        data1 = data[cn][j1][i2];\
        data2 = data[cn][j ][i1];\
        data3 = data[cn][j2][i2];mid2 = MID(data1,data2,data3);\
        data1 = data[cn][j1][i ];\
        data2 = data[cn][j ][i ];\
        data3 = data[cn][j2][i1];mid3 = MID(data1,data2,data3);\
        img->data[cn][n][m]=MID(mid1,mid2,mid3);\
    }\
}

void SeperateImage2(MImage *src,MImage *img,MImagePoint *point)
{
    int height= src->height;
    int width = src->width;
    
    unsigned char ***data = src->data;
    int data1,data2,data3;
    int mid1,mid2,mid3;
    
    int i,j,m,n;
    for(j=0,n=0;j<src->height;j+=16,n++)
        for(i=0,m=0;i<src->width;i+=16,m++)
        {
            MID_FILTER(j,i)
        }
        
    i = (int)(point->x);
    j = (int)(point->y);
    m = (int)(point->x/16.0f+0.5);
    n = (int)(point->y/16.0f+0.5);
    MID_FILTER(j,i);
}



void SeperateTable(MImage *img,MImage *info,MImagePoint *point,int thresh)
{
    int i,j;
    int channel = img->channel;
    unsigned char ***data=img->data;
    
    unsigned char **tab = info->data[0];
    unsigned char **flag= info->data[1];
    for(j=0;j<img->height;j++)
    {
        memset(tab[j] ,  0,img->width*sizeof(unsigned char));
        memset(flag[j],128,img->width*sizeof(unsigned char));
    }
    
    if(channel >1) thresh = thresh *thresh;
    
    
    
    #define DIFF(N1,M1,N2,M2,Diff) {\
        Diff = ABS(data[0][N1][M1]-data[0][N2][M2]);\
        if(channel > 1) Diff = Diff*Diff + (data[1][N1][M1]-data[1][N2][M2])*(data[1][N1][M1]-data[1][N2][M2]);\
        if(channel > 2) Diff+=             (data[2][N1][M1]-data[2][N2][M2])*(data[2][N1][M1]-data[2][N2][M2]);\
    }
    
    int diff;
    for(j=0;j<img->height;j++)for(i=0;i<img->width;i++)
    {
        DIFF(j,i,j  ,i+1,diff);if(diff<thresh) {tab[j][i]|=0x10;tab[j  ][i-1]|=0x08;}
        DIFF(j,i,j+1,i-1,diff);if(diff<thresh) {tab[j][i]|=0x20;tab[j-1][i+1]|=0x04;}
        DIFF(j,i,j+1,i  ,diff);if(diff<thresh) {tab[j][i]|=0x40;tab[j-1][i  ]|=0x02;}
        DIFF(j,i,j+1,i+1,diff);if(diff<thresh) {tab[j][i]|=0x80;tab[j-1][i-1]|=0x01;}
    }
    
    i = (int)(point->x/16.0f+0.5);
    j = (int)(point->y/16.0f+0.5);
    flag[j][i] = 255;
    flag[j-1][i-1] = (tab[j][i]&0x01!=0)?255:0;
    flag[j-1][i  ] = (tab[j][i]&0x02!=0)?255:0;
    flag[j-1][i+1] = (tab[j][i]&0x04!=0)?255:0;
    flag[j  ][i-1] = (tab[j][i]&0x08!=0)?255:0;
    flag[j  ][i+1] = (tab[j][i]&0x10!=0)?255:0;
    flag[j+1][i-1] = (tab[j][i]&0x20!=0)?255:0;
    flag[j+1][i  ] = (tab[j][i]&0x40!=0)?255:0;
    flag[j+1][i+1] = (tab[j][i]&0x80!=0)?255:0;
    
    printf("tab[%d][%d] is %d\n",j,i,tab[j][i]);
    
    // return;
    
    #define FLAG(N,M,tdata) {\
             if(flag[N][M]==  0) {if(tab[j][i]&tdata!=0) sum1++;else sum2++;}\
        else if(flag[N][M]==255) {if(tab[j][i]&tdata!=0) sum3++;else sum4++;}\
    }
    
    int sum1,sum2,sum3,sum4;
    for(int n=0;n<4;n++)
    {
        for(j=1;j<img->height-1;j++)for(i=1;i<img->width-1;i++)
        {
            sum1=0;sum2=0;sum3=0;sum4=0;
            FLAG(j-1,i-1,0x01);FLAG(j-1,i,0x02);FLAG(j-1,i+1,0x04);
            FLAG(j  ,i-1,0x08);                 FLAG(j  ,i+1,0x10);
            FLAG(j+1,i-1,0x20);FLAG(j+1,i,0x40);FLAG(j+1,i+1,0x80);
            
            if((j==72)&&(i==49))
                printf("flag is %d,sum1 is %d,sum2 is %d,sum3 is %d,sum4 is %d\n",flag[j][i],sum1,sum2,sum3,sum4);
            
            if((sum1>1)&&(sum1>sum2)&&(sum1>sum3)&&(sum1>sum4))
            {
                if(flag[j][i]==255) flag[j][i] =128;
                if(flag[j][i]==128) flag[j][i] =  0;
                if(flag[j][i]==  0) flag[j][i] =  0;
            }
            else if((sum2>1)&&(sum2>=sum1)&&(sum2>=sum3)&&(sum2>=sum4))
            {
                if(flag[j][i]==255) flag[j][i] =255;
                if(flag[j][i]==128) flag[j][i] =128;
                if(flag[j][i]==  0) flag[j][i] =  0;
            }
            else if((sum3>1)&&(sum3>sum1)&&(sum3>sum2)&&(sum3>sum4))
            {
                if(flag[j][i]==255) flag[j][i] =255;
                if(flag[j][i]==128) flag[j][i] =255;
                if(flag[j][i]==  0) flag[j][i] =128;
            }
            else if((sum4>1)&&(sum4>=sum1)&&(sum4>=sum2)&&(sum4>=sum3))
            {
                if(flag[j][i]==255) flag[j][i] =128;
                if(flag[j][i]==128) flag[j][i] =  0;
                if(flag[j][i]==  0) flag[j][i] =  0;
            }
        }
        
        for(j=img->height-2;j>0;j--)for(i=img->width-2;i>0;i--)
        {
            sum1=0;sum2=0;sum3=0;sum4=0;
            FLAG(j-1,i-1,0x01);FLAG(j-1,i,0x02);FLAG(j-1,i+1,0x04);
            FLAG(j  ,i-1,0x08);                 FLAG(j  ,i+1,0x10);
            FLAG(j+1,i-1,0x20);FLAG(j+1,i,0x40);FLAG(j+1,i+1,0x80);
            
            if((j==72)&&(i==49))
                printf("flag is %d,sum1 is %d,sum2 is %d,sum3 is %d,sum4 is %d\n",flag[j][i],sum1,sum2,sum3,sum4);
            
            if((sum1>1)&&(sum1>sum2)&&(sum1>sum3)&&(sum1>sum4))
            {
                if(flag[j][i]==255) flag[j][i] =128;
                if(flag[j][i]==128) flag[j][i] =128;
                if(flag[j][i]==  0) flag[j][i] =  0;
            }
            else if((sum2>1)&&(sum2>=sum1)&&(sum2>=sum3)&&(sum2>=sum4))
            {
                if(flag[j][i]==255) flag[j][i] =255;
                if(flag[j][i]==128) flag[j][i] =128;
                if(flag[j][i]==  0) flag[j][i] =  0;
            }
            else if((sum3>1)&&(sum3>sum1)&&(sum3>sum2)&&(sum3>sum4))
            {
                if(flag[j][i]==255) flag[j][i] =255;
                if(flag[j][i]==128) flag[j][i] =255;
                if(flag[j][i]==  0) flag[j][i] =128;
            }
            else if((sum4>1)&&(sum4>=sum1)&&(sum4>=sum2)&&(sum4>=sum3))
            {
                if(flag[j][i]==255) flag[j][i] =128;
                if(flag[j][i]==128) flag[j][i] =  0;
                if(flag[j][i]==  0) flag[j][i] =  0;
            }
        }
    }
}


struct HandleImageSeperate
{
    MImage *img;
    MImage *info;
};
void endImageSeperate(void *info)
{
    struct HandleImageSeperate *handle = info;
    if(handle->img!= NULL) mImageRelease(handle->img);
    if(handle->info!=NULL) mImageRelease(handle->info);
} 
#define HASH_ImageSeperate 0x50c2063
int mImageSeperate2(MImage *src,MImage *dst,MImagePoint *point,int thresh)
{
    int i,j,cn;
    
    MHandle *hdl; ObjectHandle(src,ImageSeperate,hdl);
    struct HandleImageSeperate *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        if(handle->img == NULL) handle->img = mImageCreate(src->channel,src->height/16,src->width/16,NULL);
        else                    mImageRedefine(handle->img,src->channel,src->height/16,src->width/16,NULL);
        
        if(handle->info== NULL) handle->info = mImageCreate(src->channel,src->height/16,src->width/16,NULL);
        else                    mImageRedefine(handle->info,src->channel,src->height/16,src->width/16,NULL);
        
        hdl->valid = 1;
    }
   
    SeperateImage2(src,handle->img,point);
    mImageMidValueFilter(handle->img,handle->img);
    mImageSave(handle->img ,"./test_ImageSeperate1.bmp");
    
    SeperateTable(handle->img,handle->info,point,thresh);
    
    MImage *tab = mImageChannelSplit(handle->info,0);
    mImageSave(tab,"./test_ImageSeperate2.bmp");
    
    MImage *flag = mImageChannelSplit(handle->info,1);
    mImageSave(flag,"./test_ImageSeperate3.bmp");
}
*/
    