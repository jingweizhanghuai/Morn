/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_image.h"

void m_ImageThresh(MImage *in,MImage *out,int cn,int thresh,int *left_value,int *right_value)
{
    mException(INVALID_IMAGE(in),EXIT,"invalid input");
    if(INVALID_IMAGE(out)) out = in;
    if(cn<0) cn=(in->channel>2)?1:0;
    unsigned char **data_in=in->data[cn];
    
    if(thresh==DFLT) thresh=128;
    mException(thresh>255,EXIT,"invalid input");

    unsigned char v[MORN_MAX_IMAGE_CN][256];
    for(int c=0;c<in->channel;c++)
    {
        unsigned char lv;if( left_value==NULL) {lv=DFLT;} else {lv=MIN( left_value[c],255);}
        unsigned char rv;if(right_value==NULL) {lv=DFLT;} else {lv=MIN(right_value[c],255);}
        for(int i=0; i<thresh; i++) v[c][i]=(lv<0)?i:lv;
        for(int i=thresh;i<256;i++) v[c][i]=(rv<0)?i:rv;
    }

    for(int j=0;j<in->height;j++)for(int i=0;i<in->width;i++)
    {
        int ref = data_in[j][i];
        for(int c=0;c<in->channel;c++)
            out->data[c][j][i]=v[c][ref];
    }
}

struct HandleImageAdaptThreshold
{
    MTable *itg[MORN_MAX_IMAGE_CN];
};
void endImageAdaptThreshold(void *info)
{
    struct HandleImageAdaptThreshold *handle = (struct HandleImageAdaptThreshold *)info;
    for(int cn=0;cn<MORN_MAX_IMAGE_CN;cn++)
        if(handle->itg[cn]!=NULL)
            mTableRelease(handle->itg[cn]);
}
#define HASH_ImageAdaptThreshold 0xac8318d6
void mImageAdaptThreshold(MImage *src,MImage *dst,int r,int thresh)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input source image");
 
    if(dst==NULL)
        dst = src;
    
    int height = src->height;
    int width = src->width;
    
    MHandle *hdl=mHandle(src,ImageAdaptThreshold);
    struct HandleImageAdaptThreshold *handle = (struct HandleImageAdaptThreshold *)(hdl->handle);
    if(hdl->valid==0)
    {
        for(int cn=0;cn<src->channel;cn++)
        {
            if(handle->itg[cn]==NULL) handle->itg[cn]=mTableCreate(height+1,width+1,sizeof(int),NULL);
            else                    mTableRedefine(handle->itg[cn],height+1,width+1,sizeof(int),NULL);
        }
        hdl->valid = 1;
    }
    MTable **itg = handle->itg;
    
    mImageIntegration(src,itg,NULL);
    
    int area = (r+r)*(r+r);
    
    for(int j=r;j<height-r;j++)
        for(int i=r;i<width-r;i++)
        {
            dst->data[0][j][i] = 0;
            for(int cn=0;cn<src->channel;cn++)
            {
                int mean = (itg[cn]->dataS32[j-r][i-r]-itg[cn]->dataS32[j-r][i+r]
                           -itg[cn]->dataS32[j+r][i-r]+itg[cn]->dataS32[j+r][i+r])/area;
                
                if((j==869)&&(i==250))
                    printf("mean is %d,src->data[cn][j][i] is %d\n",mean,src->data[cn][j][i]);
                if((j==869)&&(i==300))
                    printf("mean is %d,src->data[cn][j][i] is %d\n",mean,src->data[cn][j][i]);
                
                if(ABS(src->data[cn][j][i]-mean)>thresh)
                {
                    dst->data[0][j][i] = 255;
                    break;
                }
            }
        }
        
    dst->channel = 1;
    mInfoSet(&(dst->info),"image_type",MORN_IMAGE_GRAY);
}


