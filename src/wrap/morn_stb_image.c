#include "morn_image.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

void mImageLoad(MImage *img,const char *imgname,...)
{
    char filename[256];
    va_list val;va_start(val,imgname);vsprintf(filename,imgname,val);va_end(val);
    
    int width,height,channel;
    uint8_t *data = stbi_load(imgname,&width,&height,&channel,0);
    mImageRedefine(img,channel,height,width);
    
    uint8_t **buff;
    if(channel>=3) {buff=img->data[0];img->data[0]=img->data[2];img->data[2]=buff;}
    mImageDataInput(img,data);
    if(channel>=3) {buff=img->data[0];img->data[0]=img->data[2];img->data[2]=buff;}
}

#define JPG 1
#define PNG 2
#define BMP -1
#ifdef __GNUC__
#define stricmp strcasecmp
#endif
void mImageSave(MImage *img,const char *imgname,...)
{
    char filename[256];
    va_list val;va_start(val,imgname);vsprintf(filename,imgname,val);va_end(val);
    
    char *p=(char *)filename+strlen(filename);
    while(*p!='.') {if(p==filename) {break;} p--;}
    int type=BMP;
         if(stricmp(p+1,"jpg" )==0) type=JPG;
    else if(stricmp(p+1,"jpeg")==0) type=JPG;
    else if(stricmp(p+1,"png" )==0) type=PNG;
    
    if(type==BMP) {mBMPSave(img,filename);return;}
    
    uint8_t *data = malloc(img->channel*img->width*img->height);
    
    int image_type = DFLT;mPropertyRead(img,"image_type",&image_type);
    mException((image_type!=MORN_IMAGE_GRAY)&&(image_type!=MORN_IMAGE_RGB)&&(image_type!=MORN_IMAGE_RGBA),EXIT,"invalid image color type");
    
    uint8_t **buff;
    if(img->channel>=3) {buff=img->data[0];img->data[0]=img->data[2];img->data[2]=buff;}
    mImageDataOutput(img,data);
    if(img->channel>=3) {buff=img->data[0];img->data[0]=img->data[2];img->data[2]=buff;}
    
         if(type==PNG) stbi_write_png(filename,img->width,img->height,img->channel,data,img->width*img->channel);
    else if(type==JPG) stbi_write_jpg(filename,img->width,img->height,img->channel,data,100);
    
    free(data);
}
