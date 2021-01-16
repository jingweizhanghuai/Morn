/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_image.h"

#define fread(Data,Size,Num,Fl) mException((fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error")
     
struct HandleImageLoad
{
    FILE *f;
};
#define HASH_ImageLoad 0x5c139120
void endImageLoad(void *info);

#if defined MORN_USE_PNG
    
#include <png.h>
void mPNGLoad(MImage *dst,const char *filename)
{
    int i,j,k;
    mException(INVALID_POINTER(dst),EXIT,"invalid input");
    
    FILE *pf=NULL; FILE *f=NULL;
    MHandle *hdl=mHandle(dst,ImageLoad);
    struct HandleImageLoad *handle = (struct HandleImageLoad *)(hdl->handle);
    if(handle->f!=NULL) f=handle->f;
    else
    {
        pf = fopen(filename, "rb");
        mException((f == NULL),EXIT,"file %s cannot open",filename);
        f = pf;
    }
    
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    png_infop info_ptr = png_create_info_struct(png_ptr);

    char buf[4];fread(buf,1,4,f); 
    mException((png_sig_cmp((unsigned char*)buf, (png_size_t)0,4)!=0),EXIT,"PNG format error");
    
    fseek(f,0,SEEK_SET);
    png_init_io(png_ptr,f);
    png_read_png(png_ptr,info_ptr,PNG_TRANSFORM_EXPAND,0);
   
    int img_width = png_get_image_width(png_ptr, info_ptr);     
    int img_height = png_get_image_height(png_ptr, info_ptr);
    int cn = png_get_channels(png_ptr,info_ptr);
    
    mImageRedefine(dst,cn,img_height,img_width,dst->data);
    
    unsigned char **p_data = png_get_rows(png_ptr,info_ptr);

    if(cn==1)
    {
        mInfoSet(&(dst->info),"image_type",MORN_IMAGE_GRAY);
        for(j=0;j<img_height;j++)
            memcpy(dst->data[0][j],p_data[j],img_width*sizeof(char));
    }
    else if(cn == 3)
    {
        mInfoSet(&(dst->info),"image_type",MORN_IMAGE_RGB);
        for(j=0;j<img_height;j++)
            for(i=0,k=0;i<img_width*3;i=i+3,k=k+1)
            {
                dst->data[2][j][k] = p_data[j][i];
                dst->data[1][j][k] = p_data[j][i+1];
                dst->data[0][j][k] = p_data[j][i+2];
            }
    }
    else if(cn == 4)
    {
        mInfoSet(&(dst->info),"image_type",MORN_IMAGE_RGBA);
        for(j=0;j<img_height;j++)
            for(i=0,k=0;i<img_width*4;i=i+4,k=k+1)
            {
                dst->data[2][j][k] = p_data[j][i];
                dst->data[1][j][k] = p_data[j][i+1];
                dst->data[0][j][k] = p_data[j][i+2];
                dst->data[3][j][k] = p_data[j][i+3];
            }
    }
    png_destroy_read_struct(&png_ptr,&info_ptr,0);    
    if(pf!=NULL) fclose(pf);
}

void mPNGSave(MImage *src,const char *filename)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input image");
    mException(src->channel==2,EXIT,"save image error");
    int img_width = src->width;
    int img_height = src->height;
    
    FILE *f = fopen(filename,"wb");
    mException((f==NULL),EXIT,"file %s cannot open",filename);

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,NULL,NULL,NULL);
    mException((png_ptr==NULL),EXIT,"save image error");
    
    png_infop info_ptr = png_create_info_struct(png_ptr);
    mException((info_ptr==NULL),EXIT,"save image error");

    png_init_io(png_ptr,f);
    
    int color_type=PNG_COLOR_TYPE_RGB;
         if(src->channel==4) color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    else if(src->channel==3) color_type = PNG_COLOR_TYPE_RGB;
    else if(src->channel==1) color_type = PNG_COLOR_TYPE_GRAY;
    png_set_IHDR(png_ptr,info_ptr,img_width,img_height,8,color_type,PNG_INTERLACE_NONE,  PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr,info_ptr);
    
    unsigned char **p_data;
    unsigned char *buff=NULL;
    
    if(src->channel==1)
        p_data = src->data[0];
    else
    {
        unsigned char **p;
        p = src->data[0];
        src->data[0] = src->data[2];
        src->data[2] = p;
                
        buff = (unsigned char *)mMalloc(src->channel*img_height*img_width*sizeof(unsigned char));
        mImageDataOutputU8(src,buff,DFLT,NULL,NULL);
        
        p = src->data[0];
        src->data[0] = src->data[2];
        src->data[2] = p;
        
        p_data = (png_bytep *)mMalloc(img_height*sizeof(png_bytep));
        p_data[0]=buff;
        for(int j=1;j<img_height;j++)
            p_data[j]=p_data[j-1]+src->channel*img_width;
    }
    
    png_write_image(png_ptr,p_data); 
    png_write_end(png_ptr,info_ptr);
    
    if(buff!=NULL) mFree(buff);
   
    png_destroy_write_struct(&png_ptr,&info_ptr); 
    fclose(f);
}
#endif

void ImageLoad(MImage *img,const char *filename)
{
    
    MHandle *hdl=mHandle(img,ImageLoad);
    struct HandleImageLoad *handle = (struct HandleImageLoad *)(hdl->handle);
    mException((handle->f!=NULL),EXIT,"invalid operate");
    handle->f = fopen(filename,"rb");
    mException((handle->f == NULL),EXIT,"open file %s error",filename);
    hdl->valid = 1;
    
    unsigned char flag[8];
    fread(flag,8,1,handle->f);
    
    unsigned char png_flag[8] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
    unsigned char bmp_flag [2] = {0x42,0x4d};
    unsigned char jpg_flag1[2] = {0xFF,0xD8};
    unsigned char jpg_flag2[2] = {0xFF,0xD9};

    #if defined MORN_USE_PNG
    if(memcmp(flag,png_flag,8)==0)
    {
        fseek(handle->f,0,SEEK_SET);
        mPNGLoad(img,filename);
        goto ImageLoad_Next;
    }
    #endif
    
    if(memcmp(flag,bmp_flag,2)==0)
    {
        fseek(handle->f,0,SEEK_SET);
        mBMPLoad(img,filename);
        goto ImageLoad_Next;
    }

    #if defined MORN_USE_JPEG
    if(memcmp(flag,jpg_flag1,2)==0)
    {
        fseek(handle->f,-2,SEEK_END);
        fread(flag,2,1,handle->f);
        if(memcmp(flag,jpg_flag2,2)==0)
        {
            fseek(handle->f,0,SEEK_SET);
            mJPGLoad(img,filename);
            goto ImageLoad_Next;
        }
    }
    #endif
    
    mException(1,EXIT,"invalid image format");

    ImageLoad_Next:
    fclose(handle->f);
    handle->f = NULL;
}

#ifdef __GNUC__
#define stricmp strcasecmp
#endif
void ImageSave(MImage *img,const char *filename)
{
    char *type;
    int len = strlen(filename);
    for(type=(char *)filename+len;type>filename;type--)
        if(type[-1]=='.') break;
    mException((type==filename),EXIT,"unknown type");

         if(stricmp(type,"bmp" )==0) mBMPSave(img,filename);
    #if defined MORN_USE_JPEG
    else if(stricmp(type,"jpg" )==0) mJPGSave(img,filename);
    else if(stricmp(type,"jpeg")==0) mJPGSave(img,filename);
    #endif
    #if defined MORN_USE_PNG
    else if(stricmp(type,"png" )==0) mPNGSave(img,filename);
    #endif
    
    else mException((type==filename),EXIT,"unknown type");
}











