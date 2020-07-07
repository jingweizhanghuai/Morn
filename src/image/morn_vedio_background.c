/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_image.h"

struct HandleVideoBackground
{
    MTable *value[3][MORN_MAX_IMAGE_CN];
    MTable *cnt[3];
    
    MImage *hst;
    MImage *bgd;
};

void endVideoBackground(void *handle)
{
    int cn;
    struct HandleVideoBackground *info = (struct HandleVideoBackground *)handle;

    for(cn=0;cn<MORN_MAX_IMAGE_CN;cn++)
    {
        if(info->value[0][cn]!=NULL)   mTableRelease(info->value[0][cn]);
        if(info->value[1][cn]!=NULL)   mTableRelease(info->value[1][cn]);
        if(info->value[2][cn]!=NULL)   mTableRelease(info->value[2][cn]);
    }
    if(info->cnt[0]!=NULL)   mTableRelease(info->cnt[0]);
    if(info->cnt[1]!=NULL)   mTableRelease(info->cnt[1]);
    if(info->cnt[2]!=NULL)   mTableRelease(info->cnt[2]);
    
    if(info->hst!=NULL)      mImageRelease(info->hst);
}

#define HASH_VideoBackground 0x19ba7fbe
void mVideoBackground(MImage *src,MImage *bgd,int time_thresh,int diff_thresh)
{
    int flag;
    
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    mException((diff_thresh > 128),EXIT,"invalid input");
    mException(INVALID_POINTER(bgd),EXIT,"invalid input");
    int height = src->height;
    int width = src->width;
    int src_cn = src->channel;
    
    mException((INVALID_POINTER(bgd)||(bgd==src)),EXIT,"invalid input");
    mImageRedefine(bgd,src_cn,height,width,bgd->data);
    
    MHandle *hdl=mHandle(src,VideoBackground);
    struct HandleVideoBackground *handle = (struct HandleVideoBackground *)(hdl->handle);
    if(hdl->valid == 0)
    {
        for(int k=0;k<3;k++)
        {
            for(int cn=0;cn<src_cn;cn++)
            {
                if(handle->value[k][cn]==NULL) handle->value[k][cn]=mTableCreate(height,width,S16,NULL);
                else                         mTableRedefine(handle->value[k][cn],height,width,S16,NULL);
                
            }
            if(handle->cnt[k]==NULL) handle->cnt[k]=mTableCreate(height,width,S16,NULL);
            else                   mTableRedefine(handle->cnt[k],height,width,S16,NULL);
            for(int j=0;j<height;j++)
                memset(handle->cnt[k]->dataS16[j],0,width*sizeof(S16));
        }
        
        if(handle->hst==NULL) handle->hst = mImageCreate(src_cn,height,width,NULL);
        else                  mImageRedefine(handle->hst,src_cn,height,width,NULL);
        
        hdl->valid = 1;        
    }
    
    if(bgd!=handle->bgd)
    {
        mImageCopy(src,bgd);
        handle->bgd=bgd;
    }
    
    if(time_thresh <= 0) time_thresh = 20;
    if(diff_thresh <= 0) diff_thresh = 10;
    
    MTable **cnt = handle->cnt;
    MTable **value[3];
    value[0] = &(handle->value[0][0]);
    value[1] = &(handle->value[1][0]);
    value[2] = &(handle->value[2][0]);
    MImage *hst = handle->hst;
    
    #define BACKGROUND_CHECK(Src,Hst,Flag) {\
        for(int cn=0;cn<src_cn;cn++)\
        {\
            Flag = (ABS(Src->data[cn][j][i] - Hst->data[cn][j][i]) > diff_thresh/2);\
            if(Flag==1) break;\
        }\
    }
    #define BACKGROUND_DIFFERENT(Src,Value,Flag) {\
        for(int cn=0;cn<src_cn;cn++)\
        {\
            Flag = (ABS(Src->data[cn][j][i] - (((Value)[cn]->dataS16[j][i])>>7)) > diff_thresh);\
            if(Flag==1) break;\
        }\
    }
    #define BACKGROUND_COPY(Src,Hst) {\
         for(int cn=0;cn<src_cn;cn++)\
             Hst->data[cn][j][i] = Src->data[cn][j][i];\
    }
    #define BACKGROUND_UPDATE(Src,Value) {\
        for(int cn=0;cn<src_cn;cn++)\
            Value[cn]->dataS16[j][i] = (short)(((float)(Src->data[cn][j][i]))*12.8 + ((float)(Value[cn]->dataS16[j][i]))*0.9f);\
    }
    #define BACKGROUND_INIT(Src,Value) {\
        for(int cn=0;cn<src_cn;cn++)\
            Value[cn]->dataS16[j][i] = ((short)(Src->data[cn][j][i]))<<7;\
    }
    #define BACKGROUND_GET(Value,Dst) {\
        for(int cn=0;cn<src_cn;cn++)\
            Dst->data[cn][j][i] = (Value[cn]->dataS16[j][i])>>7;\
    }
    
    for(int j=0;j<height;j++)for(int i=0;i<width;i++)
    {
        int k,l;
        BACKGROUND_CHECK(src,hst,flag);
        BACKGROUND_COPY(src,hst);
        if(flag) continue;
        
        for(k=0;k<3;k++)
        {
            if(cnt[k]->dataS16[j][k]>0) cnt[k]->dataS16[j][k] -= 1;
        }
        
        for(k=0;k<3;k++)
        {
            if(cnt[k]->dataS16[j][k]<0) continue;
            BACKGROUND_DIFFERENT(src,value[k],flag);
            if(!flag)
            {
                BACKGROUND_UPDATE(src,value[k]);
                if(cnt[k]->dataS16[j][i]<time_thresh+time_thresh)
                    cnt[k]->dataS16[j][i] += 2;
                break;
            }
        }

        if(k==3)
        {
            for(k=0;k<3;k++)
            {
                if(cnt[k]->dataS16[j][k]==0)
                {
                    BACKGROUND_INIT(src,value[k]);
                    cnt[k]->dataS16[j][i] = 1;
                    break;
                }
            }
        }
        else
        {
            for(k=0;k<3;k++)
            {
                if(cnt[k]->dataS16[j][i] > time_thresh)
                {
                    for(l=0;l<3;l++)
                    {
                        if(l==k) continue;
                        if(cnt[k]->dataS16[j][i]<cnt[l]->dataS16[j][i])
                            break;
                    }
                    if(l==3)
                        BACKGROUND_GET(value[k],bgd);
                }
            }
        }
    }
}



