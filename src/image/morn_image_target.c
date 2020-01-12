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

struct HandleImageTarget
{
    int *color_num;
};
void endImageTarget(void *info)
{
    struct HandleImageTarget *handle = (struct HandleImageTarget *)info;
    if(handle->color_num != NULL) mFree(handle->color_num);
}
#define HASH_ImageTarget 0xcc6db083
void mImageTarget(MImage *src,MImage *dst,MImageRect *rect)
{
    int i,j;
    
    MHandle *hdl; ObjectHandle(src,ImageTarget,hdl);
    struct HandleImageTarget *handle = (struct HandleImageTarget *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->color_num != NULL) mFree(handle->color_num);
        handle->color_num = (int *)mMalloc(32768*sizeof(int));
        hdl->valid = 1;
    }
    int *color_num = handle->color_num;
    
    int rect_height= ((float)(rect->y2-rect->y1))*1.41421356f/2.0f;
    int rect_width = ((float)(rect->x2-rect->x1))*1.41421356f/2.0f;
    float cy = ((float)(rect->y2+rect->y1))/2.0f;
    float cx = ((float)(rect->x2+rect->x1))/2.0f;
    
    int x1 = (int)(cx - rect_width +0.5);if(x1<0)           x1 = 0;
    int x2 = (int)(cx + rect_width +0.5);if(x2>src->width)  x2 = src->width;
    int y1 = (int)(cy - rect_height+0.5);if(y1<0)           y1 = 0;
    int y2 = (int)(cy + rect_height+0.5);if(y2>src->height) y2 = src->height;
    
    memset(color_num,0,32768*sizeof(int));
    for(j=y1;j<y2;j++)
    {
        int flag = ((j>=rect->y1)&&(j<rect->y2));
        for(i=x1;i<x2;i++)
        {
            if(flag==1) flag = (flag&&(i>=rect->x1)&&(i<rect->x2));
            
            int b = (src->data[0][j][i])>>3;
            int g = (src->data[1][j][i])>>3;
            int r = (src->data[2][j][i])>>3;
            int n = (b<<10)+(g<<5)+r;
            
            if(flag==1) color_num[n] += 1;
            else        color_num[n] -= 1;
        }
    }
    
    for(j=rect->y1;j<rect->y2;j++)
        for(i=rect->x1;i<rect->x2;i++)
        {
            int b = (src->data[0][j][i])>>3;
            int g = (src->data[1][j][i])>>3;
            int r = (src->data[2][j][i])>>3;
            int n = (b<<10)+(g<<5)+r;
            
            if(color_num[n]>0)
            {
                dst->data[0][j][i] = src->data[0][j][i];
                dst->data[1][j][i] = src->data[1][j][i];
                dst->data[2][j][i] = src->data[2][j][i];
            }
            else
            {
                dst->data[0][j][i] = 0;
                dst->data[1][j][i] = 0;
                dst->data[2][j][i] = 0;
            }
        }
}








