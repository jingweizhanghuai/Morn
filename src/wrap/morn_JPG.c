/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_Image.h"

#include <jpeglib.h> 
#define JPEG_QUALITY 100 //图片质量

#define fread(Data,Size,Num,Fl) mException((fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error");
          
void JPGRGBSave(MImage *src,const char *filename)
{
    mException(INVALID_IMAGE(src),EXIT,"invlid input");
    int image_type = (int)mInfoGet(&(src->info),"image_type");
    mException(((image_type != MORN_IMAGE_RGB)&&(image_type != MORN_IMAGE_RGBA))||(src->channel<3),EXIT,"invlid input");

    FILE *f = fopen(filename,"wb");
    mException((f==NULL),EXIT,"file cannot open");

    int img_width = src->width;
    int img_height = src->height;
    
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;  
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);    
    jpeg_stdio_dest(&cinfo,f);
    
    cinfo.image_width = img_width;     
    cinfo.image_height = img_height;   
    cinfo.input_components = 3;  
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo,JPEG_QUALITY,1);
    
    jpeg_start_compress(&cinfo,TRUE);
    
    JSAMPROW data_buff = mMalloc(img_width*3);
    for(int j=0;j<img_height;j++)
    {
        unsigned char *p0_src = src->data[0][j];
        unsigned char *p1_src = src->data[1][j];
        unsigned char *p2_src = src->data[2][j];
        
        JSAMPROW p_buff = data_buff;
        for(int i=0;i<img_width;i++)
        {
            p_buff[0] = p2_src[i];
            p_buff[1] = p1_src[i];
            p_buff[2] = p0_src[i];
            
            p_buff = p_buff+3;
        }
        
        jpeg_write_scanlines(&cinfo,&data_buff,1);
    }
    
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    mFree(data_buff);
    fclose(f);
}

void JPGGraySave(MImage *src,const char *filename)
{
    mException(INVALID_IMAGE(src),EXIT,"invlid input");
    int image_type = (int)mInfoGet(&(src->info),"image_type");
    mException((image_type != MORN_IMAGE_GRAY),EXIT,"invlid input");

    FILE *f = fopen(filename,"wb");
    mException((f==NULL),EXIT,"file cannot open");
    
    int img_width = src->width;
    int img_height = src->height;
    
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;  
    cinfo.err = jpeg_std_error(&jerr);
    
    jpeg_create_compress(&cinfo);    
    jpeg_stdio_dest(&cinfo,f);
    
    cinfo.image_width = img_width;   //* image width and height, in pixels   
    cinfo.image_height = img_height;   
    cinfo.input_components = 1;   //* # of color components per pixel   
    cinfo.in_color_space = JCS_GRAYSCALE;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo,JPEG_QUALITY,1);
    
    jpeg_start_compress(&cinfo,TRUE);
    
    for(int j=0;j<img_height;j++)
        jpeg_write_scanlines(&cinfo,&(src->data[0][j]),1);
    
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    fclose(f);
}

void mJPGSave(MImage *src,const char *filename)
{
    mException(INVALID_IMAGE(src),EXIT,"invlid input");
    
    int image_type = (int)mInfoGet(&(src->info),"image_type");
    if(image_type == MORN_IMAGE_GRAY)
        JPGGraySave(src,filename);
    else if(image_type == MORN_IMAGE_RGB)
        JPGRGBSave(src,filename);
    else if(image_type == MORN_IMAGE_RGBA)
        JPGRGBSave(src,filename);
    else
        mException(1,EXIT,"invlid image format");
}

struct HandleImageLoad
{
    FILE *f;
};
#define HASH_ImageLoad 0x5c139120
void endImageLoad(void *info);

void mJPGLoad(const char *filename,MImage *dst)
{
    mException(INVALID_POINTER(dst),EXIT,"invalid input");
    
    FILE *pf=NULL; FILE *f;
    MHandle *hdl; ObjectHandle(dst,ImageLoad,hdl);
    struct HandleImageLoad *handle = hdl->handle;
    if(handle->f!=NULL) f=handle->f;
    else
    {
        pf = fopen(filename, "rb");
        mException((f == NULL),EXIT,"file %s cannot open",filename);
        f = pf;
    }
    
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo,f);
    jpeg_read_header(&cinfo,TRUE);         
    jpeg_start_decompress(&cinfo);
    
    int img_width = cinfo.output_width;
    int img_height = cinfo.output_height;
    int cn = cinfo.output_components;
    
    mImageRedefine(dst,cn,img_height,img_width,dst->data);
    if(cn==1)
    {
        mInfoSet(&(dst->info),"image_type",MORN_IMAGE_GRAY);
        for(int j=0;j<img_height;j++)
            jpeg_read_scanlines(&cinfo,&dst->data[0][j],1);
    }
    else if(cn == 3)
    {
        mInfoSet(&(dst->info),"image_type",MORN_IMAGE_RGB);
        JSAMPROW data_buff = mMalloc(img_width*cn);
        
        for(int j=0;j<img_height;j++)
        {
            jpeg_read_scanlines(&cinfo,&data_buff,1);
            
            unsigned char *p0_dst = dst->data[0][j];
            unsigned char *p1_dst = dst->data[1][j];
            unsigned char *p2_dst = dst->data[2][j];
            
            JSAMPROW p_buff = data_buff;
            for(int i=0;i<img_width;i++)
            {
                p2_dst[i] = p_buff[0];
                p1_dst[i] = p_buff[1];
                p0_dst[i] = p_buff[2];
                p_buff = p_buff +3;
            }
        }
        mFree(data_buff);
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    
    if(pf!=NULL) fclose(pf);
}
