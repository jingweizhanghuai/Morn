/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// gcc -O2 -fopenmp test_temp_transform.c -I ..\include\ -L ..\lib\x64\mingw -lmorn -ljpeg -lpng -lz -o test_test_Transform.exe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_image.h"

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
    