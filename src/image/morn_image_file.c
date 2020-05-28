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

#define fread(Data,Size,Num,Fl) mException(((int)fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error");
#define fwrite(Data,Size,Num,Fl) mException(((int)fwrite(Data,Size,Num,Fl)!=Num),EXIT,"write file error");

struct BMPHeader
{
    int bmpsize;
    int bmpreserved;    
    int bmpoffbits;    
    
    int imginfosize;
    int imgwidth;        
    int imgheight;    
    short imgplanes;    
    short imgbitcount;    
    int imgcompression;    
    int imgsize;        
    int imgxpelspermeter;    
    int imgypelspermeter;
    int imgclrused;    
    int imgclrimportant;
};

struct colorlist
{
    unsigned char color_blue;
    unsigned char color_green;
    unsigned char color_red;
    char reserved;
};

void BMPGraySave(MImage *src,const char *filename)
{
    int i;
    
    int data0 = 0;
    
    mException(INVALID_IMAGE(src),EXIT,"invlid input");
    int image_type = (int)mInfoGet(&(src->info),"image_type");
    mException((image_type != MORN_IMAGE_GRAY),EXIT,"invlid input");
    
    FILE *f = fopen(filename,"wb");
    // printf("src is %p,f is %p,&bmptype is %p\n",src,f,&bmptype);
    mException((f == NULL),EXIT,"cannot open file");
    short bmptype = 0x4d42;fwrite(&bmptype,1,2,f);
    
    int img_width = src->width;
    int img_height = src->height;
    int data_width = ((img_width-1)&0xFFFFFFFC)+4;
    int img_size = img_height*data_width;
    
    struct BMPHeader my_bmp;
    my_bmp.bmpsize = 1078 + img_size;
    my_bmp.bmpreserved = 0;
    my_bmp.bmpoffbits = 1078;
    my_bmp.imginfosize = 40;
    my_bmp.imgwidth = img_width;
    my_bmp.imgheight = img_height;
    my_bmp.imgplanes = 1;
    my_bmp.imgbitcount = 8;
    my_bmp.imgcompression =0;
    my_bmp.imgsize = img_size;
    my_bmp.imgxpelspermeter = 11811;
    my_bmp.imgypelspermeter = 11811;
    my_bmp.imgclrused = 0;
    my_bmp.imgclrimportant = 0;
    
    fwrite(&my_bmp,1,52,f);

    struct colorlist color_gray[256];
    for(i=0;i<256;i++)
    {
        color_gray[i].color_blue  = (unsigned char)i;
        color_gray[i].color_green = (unsigned char)i;
        color_gray[i].color_red   = (unsigned char)i;
        color_gray[i].reserved    = 0;
    }
    fwrite(color_gray,1,1024,f);
    
    if(data_width==img_width)
    {
        for(i=img_height-1;i>=0;i--)           
            fwrite(src->data[0][i],1,img_width,f);
    }
    else
    {
        for(i=img_height-1;i>=0;i--)
        {
            fwrite(src->data[0][i],1,img_width,f);
            fwrite(&data0,1,data_width-img_width,f);
        }
    }
    
    fclose(f);
}

void BMPRGBSave(MImage *src,const char *filename)
{
    int i,j;
    mException(INVALID_IMAGE(src),EXIT,"invlid input");
    int image_type = (int)mInfoGet(&(src->info),"image_type");
    mException((image_type != MORN_IMAGE_RGB)||(src->channel<3),EXIT,"invlid input");
    
    FILE *f = fopen(filename,"wb");
    mException((f == NULL),EXIT,"cannot open file");
    short bmptype = 0x4d42;fwrite(&bmptype,1,2,f);

    int img_width = src->width;
    int img_height = src->height;
    int data_width = ((img_width*3+3)>>2)<<2;
    int data_size = data_width * img_height;
    
    struct BMPHeader my_bmp;
    my_bmp.bmpsize = 54 + data_size;
    my_bmp.bmpreserved = 0;
    my_bmp.bmpoffbits = 54;
    my_bmp.imginfosize = 40;
    my_bmp.imgwidth = img_width;
    my_bmp.imgheight = img_height;
    my_bmp.imgplanes = 1;
    my_bmp.imgbitcount = 24;
    my_bmp.imgcompression =0;
    my_bmp.imgsize = data_size;
    my_bmp.imgxpelspermeter = 11811;
    my_bmp.imgypelspermeter = 11811;
    my_bmp.imgclrused = 0;
    my_bmp.imgclrimportant = 0;
    
    fwrite(&my_bmp,1,52,f);
    
    if(data_width==img_width*3)
    {
        for(j=img_height-1;j>=0;j--)            
            for(i=0;i<img_width;i++)
            {
                fwrite(src->data[0][j]+i,1,1,f);
                fwrite(src->data[1][j]+i,1,1,f);
                fwrite(src->data[2][j]+i,1,1,f);
            }
    }
    else
    {        
        for(j=img_height-1;j>=0;j--)
        {
            for(i=0;i<img_width;i++)
            {                    
                fwrite(src->data[0][j]+i,1,1,f);
                fwrite(src->data[1][j]+i,1,1,f);
                fwrite(src->data[2][j]+i,1,1,f);
            }
            int data0[3] = {0,0,0};
            fwrite(data0,1,data_width-img_width*3,f);  
        }
    }
    fclose(f);
    
}

void BMPRGBASave(MImage *src,const char *filename)
{
    FILE *f;
    
    int img_width;
    int img_height;
    int data_width;
    int data_size;
    
    int i,j;
    
    short bmptype = 0x4d42;
    
    struct BMPHeader my_bmp;
    
    mException(INVALID_IMAGE(src),EXIT,"invlid input");
    int image_type = (int)mInfoGet(&(src->info),"image_type");
    mException((image_type != MORN_IMAGE_RGBA)||(src->channel<4),EXIT,"invlid input");
    
    f = fopen(filename,"wb");
    mException((f == NULL),EXIT,"cannot open file");
    
    img_width = src->width;
    img_height = src->height;
    
    data_width = img_width*4;
    data_size = data_width * img_height;

    fwrite(&bmptype,1,2,f);
    
    my_bmp.bmpsize = 54 + data_size;
    my_bmp.bmpreserved = 0;
    my_bmp.bmpoffbits = 54;
    my_bmp.imginfosize = 40;
    my_bmp.imgwidth = img_width;
    my_bmp.imgheight = img_height;
    my_bmp.imgplanes = 1;
    my_bmp.imgbitcount = 32;
    my_bmp.imgcompression =0;
    my_bmp.imgsize = data_size;
    my_bmp.imgxpelspermeter = 11811;
    my_bmp.imgypelspermeter = 11811;
    my_bmp.imgclrused = 0;
    my_bmp.imgclrimportant = 1;
    
    fwrite(&my_bmp,1,52,f);
    
    for(j=img_height-1;j>=0;j--)
        for(i=0;i<img_width;i++)
        {
            fwrite(src->data[0][j]+i,1,1,f);
            fwrite(src->data[1][j]+i,1,1,f);
            fwrite(src->data[2][j]+i,1,1,f);
            fwrite(src->data[3][j]+i,1,1,f);
        }
        
    fclose(f);
}

void mBMPSave(MImage *src,const char *filename)
{
    mException(INVALID_IMAGE(src),EXIT,"invlid input");
    
    int image_type = (int)mInfoGet(&(src->info),"image_type");
    
    if(image_type == MORN_IMAGE_GRAY)
        BMPGraySave(src,filename);
    else if(image_type == MORN_IMAGE_RGB)
        BMPRGBSave(src,filename);
    else if(image_type == MORN_IMAGE_RGBA)
        BMPRGBASave(src,filename);
    else
        mException(1,EXIT,"invalid image type");
}

struct HandleImageLoad
{
    FILE *f;
};
#define HASH_ImageLoad 0x5c139120
void endImageLoad(void *info) {}
void mBMPLoad(MImage *dst,const char *filename)
{
    int i,j;
    mException(INVALID_POINTER(dst),EXIT,"invalid input");
    
    FILE *pf=NULL; FILE *f;
    MHandle *hdl=mHandle(dst,ImageLoad);
    struct HandleImageLoad *handle = (struct HandleImageLoad *)(hdl->handle);
    if(handle->f!=NULL) f=handle->f;
    else
    {
        pf = fopen(filename, "rb");
        mException((pf == NULL),EXIT,"file %s cannot open",filename);
        f = pf;
    }
    
    short filetype;
    fread(&filetype,1,sizeof(short),f);
    mException((filetype != 0x4d42),EXIT,"invalid BMP format");
    
    struct BMPHeader my_bmp;
    fread(&my_bmp,1,52,f);
    
    int img_width = my_bmp.imgwidth;
    int img_height = my_bmp.imgheight;
    int cn = my_bmp.imgbitcount>>3;
    
    mImageRedefine(dst,cn,img_height,img_width,dst->data);
    
    int data_width = ((img_width*cn-1)&0xFFFFFFFC)+4;
    
    int pos = my_bmp.bmpoffbits;
    
    if(cn==1)
    {
        mInfoSet(&(dst->info),"image_type",MORN_IMAGE_GRAY);
        for(j=img_height-1;j>=0;j--)
        {
            fseek(f,pos,SEEK_SET);
            fread(dst->data[0][j],1,img_width,f);
                        
            pos = pos+data_width;
        }
    }
    else if(cn == 3)
    {
        mInfoSet(&(dst->info),"image_type",MORN_IMAGE_RGB);
        for(j=img_height-1;j>=0;j--)
        {
            fseek(f,pos,SEEK_SET);
            
            for(i=0;i<img_width;i++)
            {    
                fread(dst->data[0][j]+i,1,1,f);
                fread(dst->data[1][j]+i,1,1,f);
                fread(dst->data[2][j]+i,1,1,f);
            }
                        
            pos = pos+data_width;
        }
    }
    else if(cn == 4)
    {
        mInfoSet(&(dst->info),"image_type",MORN_IMAGE_RGBA);
        for(j=img_height-1;j>=0;j--)
        {
            fseek(f,pos,SEEK_SET);
            
            for(i=0;i<img_width;i++)
            {    
                fread(dst->data[0][j]+i,1,1,f);
                fread(dst->data[1][j]+i,1,1,f);
                fread(dst->data[2][j]+i,1,1,f);
                fread(dst->data[3][j]+i,1,1,f);
            }
                        
            pos = pos+data_width;
        }
    }
    else
        mException(1,EXIT,"invalid BMP format");
 
    if(pf!=NULL) fclose(pf);
}
