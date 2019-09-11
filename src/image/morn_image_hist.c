#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_Image.h"

/*
MHist *mHistCreate(int num,int *data)
{
    MHist *hist;
    
    hist = (MHist *)mMalloc(sizeof(MHist));
    
    if(num<0)
        num = 0;
    
    hist->num = num;
    hist->handle = NULL;
    
    if(num==0)
    {
        hist->data = NULL;
        hist->memory = NULL;
    }
    else if(!INVALID_POINTER(data))
    {
        hist->data = data;
        hist->memory = NULL;
    }
    else
    {
        hist->memory = mMemoryBlockCreate(num*sizeof(int));
        hist->data = (int *)(hist->memory->memory);
    }
    
    return hist;
}

void mHistRelease(MHist *hist)
{   
    mException(INVALID_POINTER(hist),"invalid input",EXIT);
    
    if(!INVALID_POINTER(hist->memory))
        mMemoryRelease(hist->memory);
    
    if(!INVALID_POINTER(hist->handle))
        HandleRelease(hist->handle);
    
    mFree(hist);
}

void mHistRedefine(MHist *hist,int num)
{
    mException(INVALID_POINTER(hist),"invalid input",EXIT);
    
    hist->num = num;
    if(hist->num>=num)
        return;
    
    if(INVALID_POINTER(hist->memory))
        hist->memory = mMemoryBlockCreate(num*sizeof(int));
    else
        mMemoryBlockRedefine(hist->memory,num*sizeof(int));
    
    hist->data = (int *)((hist->memory)->memory);
}
*/

void mImageHist(MImage*img,int cn,MArray *hist)
{
    int hist_num;
    int i,j,k;
    int n;
    int value;
    
    mException(INVALID_IMAGE(img),EXIT,"invalid input");
    mException((cn>=(img->channel)),EXIT,"invalid input");
    
    if(img->channel == 1)
        cn = 0;
    
    if(hist->num <= 0)
    {
        if(cn >= 0)
            mArrayRedefine(hist,256,S32,hist->data);
        else if(img->channel <= 2)
            mArrayRedefine(hist,4096,S32,hist->data);
        else
            mArrayRedefine(hist,((img->channel)<<4),S32,hist->data);
    }
    
    memset(hist->data,0,sizeof(int)*(hist->num));
    
    if((cn >= 0)&&(hist->num == 256))
    {
        for(j=ImageY1(img);j<ImageY2(img);j++)
            for(i=ImageX1(img,j);i<ImageX2(img,j);i++)
            {
                n = img->data[cn][j][i];
                hist->dataS32[n] = hist->dataS32[n] + 1;
            }
    }
    else if(cn >= 0)
    {
        for(j=ImageY1(img);j<ImageY2(img);j++)
            for(i=ImageX1(img,j);i<ImageX2(img,j);i++)
            {
                value = img->data[0][j][i];
                n = value*(hist->num)/256;
                hist->dataS32[n] = hist->dataS32[n] + 1;
            }
    }
    else
    {
        if((img->channel == 3)&&(hist->num == 4096))
            hist_num = 16;
        else if((img->channel == 2)&&(hist->num == 4096))
            hist_num = 64;
        else if(img->channel == 2)
            hist_num = (int)(sqrt((double)(hist->num)));
        else
            hist_num = (int)(pow(((double)(hist->num)),(1.0/((double)(img->channel)))));
        
        for(j=ImageY1(img);j<ImageY2(img);j++)
            for(i=ImageX1(img,j);i<ImageX2(img,j);i++)
            {
                n = 0;
                for(k=0;k<img->channel;k++)
                {
                    n = n*hist_num;
                    value = img->data[k][j][i];
                    n = n + (value*hist_num/256);
                }
                hist->dataS32[n] = hist->dataS32[n] + 1;
            }
    }
}

void mHistNormlize(MArray *src,MArray *dst,int norm_data)
{
    int count;
    float k;
    int i;
    
    mException(INVALID_POINTER(src),EXIT,"invalid input");
    
    count = 0;
    for(i=0;i<src->num;i++)
        count = count + src->dataS32[i];
    
    k = ((float)norm_data)/((float)count);
    
    mArrayRedefine(dst,src->num,S32,dst->data);
    
    for(i=0;i<src->num;i++)
        dst->dataS32[i] = (int)(((float)src->dataS32[i]) * k + 0.5f);
}
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
