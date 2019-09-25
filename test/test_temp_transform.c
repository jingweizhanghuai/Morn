// gcc -O2 -fopenmp test_temp_transform.c -I ..\include\ -L ..\lib\x64\mingw -lmorn -ljpeg -lpng -lz -o test_test_Transform.exe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Image.h"

int main()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("../doc/test2.jpg",src);
    
    MImage *dst = mImageCreate(DFLT,DFLT,DFLT,NULL);
    MObject *temp = mObjectCreate(NULL);
    
    mImageLensTemplate(temp,-0.5f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out1.jpg");
    
    mImageLensTemplate(temp,-0.3f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out2.jpg");
    
    mImageLensTemplate(temp,-0.1f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out3.jpg");
    
    mImageLensTemplate(temp,0.1f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out4.jpg");
    
    mImageLensTemplate(temp,0.3f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out5.jpg");
    
    mImageLensTemplate(temp,0.5f,80);
    mImageTemplateTransform(src,dst,temp,120,205);
    mImageSave(dst,"./test_Transform_out6.jpg");
    
    mImageRelease(src);
    mImageRelease(dst);
    mObjectRelease(temp);
    
    return 0;
}
    