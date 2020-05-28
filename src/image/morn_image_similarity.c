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

extern unsigned char r_to_y[256];
extern unsigned char g_to_y[256];
extern unsigned char b_to_y[256];
struct HandleSimilarityFeature
{
    uint32_t feature[4][63];
};
void endSimilarityFeature(void *info) {}
#define HASH_SimilarityFeature 0xae6941e4

uint32_t *mImageSimilarityFeature(MImage *src)
{
    MHandle *hdl=mHandle(src,SimilarityFeature);
    struct HandleSimilarityFeature *handle = (struct HandleSimilarityFeature *)(hdl->handle);
    if(hdl->valid == 1) return &(handle->feature[0][0]);
    hdl->valid =1;
        
    int image_type = (int)mInfoGet(&(src->info),"image_type");
    mException((image_type != MORN_IMAGE_GRAY)&&(image_type != MORN_IMAGE_RGB),EXIT,"invalid input");
    MTable *tab = mTableCreate(src->height+1,src->width+1,S32,NULL);
    memset(tab->dataS32[0],0,(src->width+1)*sizeof(float));
    for(int j=0;j<src->height;j++)
    {
        int sum = 0;
        for(int i=0;i<src->width;i++)
        {
            int data;
            if(src->channel==MORN_IMAGE_RGB)
            {
                unsigned char r = src->data[2][j][i];
                unsigned char g = src->data[1][j][i];
                unsigned char b = src->data[0][j][i];
                data = r_to_y[r] + g_to_y[g] + b_to_y[b];
            }
            else data = src->data[0][j][i];
            sum = sum + data;
            tab->dataS32[j+1][i+1] = tab->dataS32[j][i+1]+sum;
        }
    }
    
    float height = src->height;float width = src->width;
    float cx = width/2.0f;float cy = height/2.0f;
    
    for(int m=0;m<4;m++)
    {
        float y_step = height/8.0f; float x_step = width/8.0f;
        height = height/1.1f;width = width/1.1f;
        float x = cx - x_step*4.0f;float y = cy - y_step*4.0f;
        int mean[8][8];
        
        for(int j=0;j<8;j++)
            for(int i=0;i<8;i++)
            {
                int x0 = x+i*x_step;int x1 = x0+x_step;
                int y0 = y+i*y_step;int y1 = y0+y_step;
                mean[j][i] = tab->dataS32[y0][x0]+tab->dataS32[y1][x1]
                            -tab->dataS32[y0][x1]-tab->dataS32[y1][x0];
                mean[j][i] = mean[j][i]/((y1-y0)*(x1-x0));
            }
            
        char feature[2016];
        int n=0;
        for(int j=1;j<64;j++)
            for(int i=0;i<j;i++)
            {
                feature[n] = (ABS(mean[j]-mean[i])>32);
                n=n+1;
            }
        n=0;
        for(int i=0;i<2016;i+=32)
        {
            handle->feature[m][n] = 0;
            for(int j=31;j>=0;j--)
                handle->feature[m][n]+=(feature[i]<<j);
            n=n+1;
        }
    }
    mTableRelease(tab);
    return &(handle->feature[0][0]);
}

char morn_hanming[256]={0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,
                        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
                        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
                        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
                        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
                        1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
                        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
                        2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
                        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
                        3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
                        4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8};
int SimilarityFeatureDistance(uint32_t *feature1,uint32_t *feature2)
{
    int min = 2016;
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
        {
            uint32_t *p1 = feature1+i*63;
            uint32_t *p2 = feature2+j*63;
            int d=0;
            for(int n=0;n<63;n++)
            {
                int result = p1[n]^p2[n];
                unsigned char *p = (unsigned char *)(&result);
                d += morn_hanming[p[0]]+morn_hanming[p[1]]+morn_hanming[p[2]]+morn_hanming[p[3]];
            }
            min = MIN(d,min);
        }
    return min;
}

int mImageSimilarity(MImage *src1,MImage *src2)
{
    uint32_t *feature1 = mImageSimilarityFeature(src1);
    uint32_t *feature2 = mImageSimilarityFeature(src2);
    return SimilarityFeatureDistance(feature1,feature2);
}

// void mImageSearch(MImage *src,MList *list,int *out,int n)
// {
    // uint32_t *feature = mImageSimilarityFeature(src);
    // for(int i=0;i<list->num;i++)
    // {
        // int d = SimilarityFeatureDistance(feature,list->data[i]);
        // mSequenceMinSubsetS32(proc,d,i,NULL,NULL,n);
    // }
// }










