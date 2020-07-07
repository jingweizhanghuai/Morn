/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_color_convert.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -ljpeg -lpng -lz -o test_color_convert.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_Image.h"

int main1()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    MImage *img = mImageCreate(1,DFLT,DFLT,NULL);
    mImageLoad(src,"../doc/test2.jpg");
    
    MImage *hsv = mImageCreate(3,src->height,src->width,NULL);
    mImageRGBToHSV(src,hsv);
    
    mImageRedefine(img,1,src->height,src->width,&(hsv->data[0]));
    mImageSave(img,"./test_color_convert_hsv_h.png");
    
    mImageRedefine(img,1,src->height,src->width,&(hsv->data[1]));
    mImageSave(img,"./test_color_convert_hsv_s.png");
    
    mImageRedefine(img,1,src->height,src->width,&(hsv->data[2]));
    mImageSave(img,"./test_color_convert_hsv_v.png");
    
    MImage *rgb1 = mImageCreate(3,src->height,src->width,NULL);
    mImageHSVToRGB(hsv,rgb1);
    mImageSave(rgb1,"./test_ColorConvert_rgb1.png");
    
    MImage *yuv = mImageCreate(3,src->height,src->width,NULL);
    mImageRGBToYUV(src,yuv);
    
    mImageRedefine(img,1,src->height,src->width,&(yuv->data[0]));
    mImageSave(img,"./test_color_convert_yuv_y.png");
    
    mImageRedefine(img,1,src->height,src->width,&(yuv->data[1]));
    mImageSave(img,"./test_color_convert_yuv_u.png");
    
    mImageRedefine(img,1,src->height,src->width,&(yuv->data[2]));
    mImageSave(img,"./test_color_convert_yuv_v.png");
    
    MImage *rgb2 = mImageCreate(3,src->height,src->width,NULL);
    mImageYUVToRGB(yuv,rgb2);
    mImageSave(rgb2,"./test_ColorConvert_rgb2.png");
    
    mImageRelease(src);
    mImageRelease(img);
    mImageRelease(hsv);
    mImageRelease(yuv);
    mImageRelease(rgb1);
    mImageRelease(rgb2);
    
    return 0;
}

int main4()
{
    int rx[256],gx[256],bx[256],ry[256],gy[256],by[256],rz[256],gz[256],bz[256];
    for(int i=0;i<256;i++)
    {
        // =0.412453*i/0.950456+0.5;gx[i]=0.357580*i/0.950456+0.5;bx[i]=0.180423*i/0.950456+0.5;
        // =0.212671*i+0.5;gy[i]=0.715160*i+0.5;by[i]=0.072169*i+0.5;
        // =0.019334*i/1.088754+0.5;gz[i]=0.119193*i/1.088754+0.5;bz[i]=0.950227*i/1.088754+0.5;
        rx[i]= 0.2126 * i+0.5;gx[i]= + 0.7152 * i+0.5;bx[i]= + 0.0722 * i+0.5;
        ry[i]= 0.3264 *2.55* i+0.5;gy[i]= - 0.5000 *2.55* i+0.5;by[i]= + 0.1736 *2.55* i+0.5;
        rz[i]= 0.12173*2.55* i+0.5;gz[i]= + 0.37827*2.55* i+0.5;bz[i]= - 0.5000 *2.55* i+0.5;
    }
    printf("\b};\nunsigned char r2l[256]={");for(int i=0;i<256;i++)printf("%d,",rx[i]);
    printf("\b};\nunsigned char g2l[256]={");for(int i=0;i<256;i++)printf("%d,",gx[i]);
    printf("\b};\nunsigned char b2l[256]={");for(int i=0;i<256;i++)printf("%d,",bx[i]);
    printf("\b};\nunsigned char r2a[256]={");for(int i=0;i<256;i++)printf("%d,",ry[i]);
    printf("\b};\nunsigned char g2a[256]={");for(int i=0;i<256;i++)printf("%d,",gy[i]);
    printf("\b};\nunsigned char b2a[256]={");for(int i=0;i<256;i++)printf("%d,",by[i]);
    printf("\b};\nunsigned char r2b[256]={");for(int i=0;i<256;i++)printf("%d,",rz[i]);
    printf("\b};\nunsigned char g2b[256]={");for(int i=0;i<256;i++)printf("%d,",gz[i]);
    printf("\b};\nunsigned char b2b[256]={");for(int i=0;i<256;i++)printf("%d,",bz[i]);
}

// int main()
// {
//     for(int i=0;i<256;i++)
//     {
//         float x=(float)i/255.0;
//         x = (x<0.0088564516790356308)?pow(x,1.0f/3.0f):(7.787037*x +0.137931);
//         printf("%f,",x);
//     }
// }

void mImageRGBToLAB(MImage *src,MImage *dst);
int maiwn()
{
    MImage *img = mImageCreate();
    mImageLoad(img,"./traffic_line/test45_rsz.png");
    img->data[0][100][100]=255;
    img->data[1][100][100]=0;
    img->data[2][100][100]=255;

    MImage *lab = mImageCreate();
    mImageRGBToHSV(img,lab);
    mImageSave(lab,"./traffic_line/test45_lab.png");

    MImage *l=mImageCreate(1,lab->height,lab->width,lab->data+2);
    mImageSave(l,"./traffic_line/test45_l.png");

    printf("eeeeeeeeeeeeee\n");
    mImageRelease(img);
    mImageRelease(lab);
    mImageRelease(l);
}

int main3()
{
    float step[24];
    int idx[24];idx[0]=0;
    for(int v=0;v<24;v++)
    {
        int n=v+1;
        step[v]=240.0/(float)n;
        int num=n*n;printf("%d,",num);
        idx[n]=idx[v]+num;
    }
    
    printf("\b}\nfloat hs_step[24]={");
    for(int v=0;v<24;v++) printf("%f,",step[v]);

    printf("\b}\nfloat hs_idx[24]={");
    for(int v=0;v<24;v++) printf("%d,",idx[v]);
}

int main()
{
    MImage *img = mImageCreate();
    // mImageLoad(img,"./traffic_line/test45_rsz.png");
    mImageLoad(img,"D:/data/data48/test_img0188.png");

    MImage *rst = mImageCreate(3,img->height,img->width);
    for(int j=0;j<img->height;j++)for(int i=0;i<img->width;i++)
    {
        int b=img->data[0][j][i];
        int g=img->data[1][j][i];
        int r=img->data[2][j][i];
        float k = 256.0/(float)(b+g+r);

        int d1=(b-g+128)*256.0/(b+g+r);
        int d2=(g-r+128)*256.0/(b+g+r);
        int d3=(r-b+128)*256.0/(b+g+r);

        rst->data[0][j][i]=255-b;//MAX(MIN(255,d3),0);
        rst->data[1][j][i]=255-g;//MAX(MIN(255,d1),0);
        rst->data[2][j][i]=255-r;//MAX(MIN(255,d2),0);

        rst->data[0][j][i]=rst->data[0][j][i]/10*10;
        rst->data[1][j][i]=rst->data[1][j][i]/10*10;
        rst->data[2][j][i]=rst->data[2][j][i]/10*10;
        
        // float X = r* 0.4124 + g* 0.3576 + b* 0.1805;
        // float Y = r* 0.2126 + g* 0.7152 + b* 0.0722;
        // float Z = r* 0.0193 + g* 0.1192 + b* 0.9505;

        // rst->data[0][j][i]=(X-Y+128)*256.0/(X+Y+Z);
        // rst->data[1][j][i]=(Y-Z+128)*256.0/(X+Y+Z);
        // rst->data[2][j][i]=(Z-X+128)*256.0/(X+Y+Z);
        
    }
    printf("eeeeeeeeeeeeee\n");
    mImageSave(rst,"./traffic_line/test45_rst.png");
    mImageRelease(img);
    mImageRelease(rst);
}