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

void ImageRandResize(MImage *src,MImage *dst)
{
    // int j;
    int height=dst->height;
    int width =dst->width;
    float kx = (float)(src->width )/(float)width;
    float ky = (float)(src->height)/(float)height;
    int s = (int)(MAX(kx,ky)+0.5);
    // printf("s is %d\n",s);
    // printf("kx is %f,ky is %f\n",kx,ky);
    int j;
    #pragma omp parallel for
    for(j=0;j<height;j++)
    {
        float ly=ky*j;int y=(int)(ly+0.5);
        for(int i=0;i<width;i++)
        {
            float lx=kx*i;int x=(int)(lx+0.5);
            int rx = x+mRand(0,s);
            int ry = y+mRand(0,s);
            for(int cn=0;cn<src->channel;cn++)
                dst->data[cn][j][i] = src->data[cn][ry][rx];
        }
    }
}

















