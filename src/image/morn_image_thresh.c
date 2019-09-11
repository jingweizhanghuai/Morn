#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Image.h"

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
    
    
    MHandle *hdl; ObjectHandle(src,ImageAdaptThreshold,hdl);
    struct HandleImageAdaptThreshold *handle = hdl->handle;
    if(hdl->valid==0)
    {
        for(int cn=0;cn<src->channel;cn++)
        {
            if(handle->itg[cn]==NULL) handle->itg[cn]=mTableCreate(height+1,width+1,S32,NULL);
            else                    mTableRedefine(handle->itg[cn],height+1,width+1,S32,NULL);
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


