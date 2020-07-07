/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
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

















