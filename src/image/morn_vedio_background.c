#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_image.h"

struct HandleImageBackground
{
    MTable *cnt;
    MImage *buff;
    MImage *rst;
    int count;
}HandleImageBackground;

unsigned char morn_update9[256]={0,1,2,3,4,5,5,6,7,8,9,10,11,12,13,14,14,15,16,17,18,19,20,21,22,23,23,24,25,26,27,28,29,30,31,32,32,33,34,35,36,37,38,39,40,41,41,42,43,44,45,46,47,48,49,50,50,51,52,53,54,55,56,57,58,59,59,60,61,62,63,64,65,66,67,68,68,69,70,71,72,73,74,75,76,77,77,78,79,80,81,82,83,84,85,86,86,87,88,89,90,91,92,93,94,95,95,96,97,98,99,100,101,102,103,104,104,105,106,107,108,109,110,111,112,113,113,114,115,116,117,118,119,120,121,122,122,123,124,125,126,127,128,129,130,131,131,132,133,134,135,136,137,138,139,140,140,141,142,143,144,145,146,147,148,149,149,150,151,152,153,154,155,156,157,158,158,159,160,161,162,163,164,165,166,167,167,168,169,170,171,172,173,174,175,176,176,177,178,179,180,181,182,183,184,185,185,186,187,188,189,190,191,192,193,194,194,195,196,197,198,199,200,201,202,203,203,204,205,206,207,208,209,210,211,212,212,213,214,215,216,217,218,219,220,221,221,222,223,224,225,226,227,228,229,229};
unsigned char morn_update1[256]={0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,20,20,21,21,21,21,21,21,21,21,21,21,22,22,22,22,22,22,22,22,22,22,23,23,23,23,23,23,23,23,23,23,24,24,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,25,25,25,26};
void endImageBackground(void *info)
{
    struct HandleImageBackground *handle = info;
    if(handle->cnt !=NULL) mTableRelease(handle->cnt); 
    if(handle->buff!=NULL) mImageRelease(handle->buff);
    if(handle->rst !=NULL) mImageRelease(handle->rst);
}

#define HASH_ImageBackground 0x9f8230f8

#define DIFFERENT(Src1,Src2,flag) {\
    for(cn=0;cn<src_cn;cn++)\
    {\
        flag = (ABS(Src1->data[cn][j][i] - Src2->data[cn][j][i]) > diff_thresh);\
        if(flag == 1) break;\
    }\
}
#define POINT_COPY(Src,Dst) {\
    for(cn=0;cn<src_cn;cn++)\
        Dst->data[cn][j][i] = Src->data[cn][j][i];\
}
#define POINT_CACULATE(Src,Dst) {\
    for(cn=0;cn<src_cn;cn++)\
        Dst->data[cn][j][i] = morn_update9[Dst->data[cn][j][i]] + morn_update1[Src->data[cn][j][i]];\
}

MImage *mImageBackground(MImage *src,int time_thresh,int diff_thresh)
{
    int j;
    int flag;
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    mException((diff_thresh > 128),EXIT,"invalid input");
    int height = src->height;
    int width = src->width;
    int src_cn = src->cn;
    
    if(time_thresh <= 0)time_thresh = 20;
    if(diff_thresh <= 0)diff_thresh = 10;
    
    MHandle *hdl; ObjectHandle(src,ImageBackground,hdl);
    struct HandleImageBackground *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        if(handle->cnt == NULL) handle->cnt = mTableCreate(height,width,S16,NULL);
        else mTableRedefine(handle->cnt,height,width,S16);
        for(j=0;j<height;j++)memset(handle->cnt->data[j],0,width*sizeof(S16));
        
        if(handle->buff == NULL) handle->buff= mImageCreate(src_cn,height,width,NULL);
        else mImageRedefine(handle->buff,src_cn,height,width);
        mImageCopy(src,handle->buff);
        
        if(handle->rst == NULL) handle->rst= mImageCreate(src_cn,height,width,NULL);
        else mImageRedefine(handle->rst,src_cn,height,width);
        mImageCopy(src,handle->rst);
        
        handle->count = 1;
        
        hdl->valid = 1;
        return handle->rst;
    }
    short **cnt = (handle->cnt)->dataS16;
    MImage *buff= handle->buff;
    MImage *rst = handle->rst;
    
    #pragma omp parallel for
    for(j=0;j<height;j++)
    {
        for(int i=0;i<width;i++)
        {
            int cn;
            DIFFERENT(src,rst,flag);
            // printf("flag is %d\n",flag);
            if(flag)
            {
                DIFFERENT(src,buff,flag);
                if(flag)
                {
                    if(cnt[j][i]>0) cnt[j][i] -= 1;
                    else POINT_COPY(src,buff);
                }
                else
                {
                    cnt[j][i] += 1;
                    if(cnt[j][i] > time_thresh)
                    {
                        cnt[j][i] = 0;
                        POINT_COPY(buff,rst);
                    }
                    else
                        POINT_CACULATE(src,buff);
                }
            }
            else
            {
                if(cnt[j][i]>0)cnt[j][i] -= 1;
                if(handle->count==0)
                    POINT_CACULATE(src,rst);
            }
        }
    }
    handle->count += 1;if(handle->count==4) handle->count=0;
    
    return rst;
}


/*
struct HandleImageBackground
{
    MMatrix *cnt;
    MMatrix *buff[MORN_MAX_IMAGE_CN];
    MMatrix *rst[MORN_MAX_IMAGE_CN];
}HandleImageBackground;

void endImageBackground(void *handle)
{
    struct HandleImageBackground *hdl;
    int i;
    
    hdl = (struct HandleImageBackground *)handle;
    mMatrixRelease(hdl->cnt);
    for(i=0;i<MORN_MAX_IMAGE_CN;i++)
    {
        if(hdl->buff[i]!=NULL)
            mMatrixRelease(hdl->buff[i]);
        if(hdl->rst[i]!=NULL)
            mMatrixRelease(hdl->rst[i]);
    }
}

#define HASH_ImageBackground 0x9f8230f8

#define DIFFERENT(Src1,Src2,flag) {\
    flag = (ABS(Src1->data[0][j][i] - Src2[0]->data[j][i]) > diff_thresh);\
    for(cn=1;cn<src_cn;cn++)\
    {\
        if(flag == 1)\
            break;\
        flag = flag||(ABS(Src1->data[cn][j][i] - Src2[cn]->data[j][i]) > diff_thresh);\
    }\
}
#define POINT_INIT(Src,Dst) {\
    Dst[0]->data[j][i] = Src->data[0][j][i];\
    for(cn=1;cn<src_cn;cn++)\
        Dst[cn]->data[j][i] = Src->data[cn][j][i];\
}
#define POINT_COPY(Src,Dst) {\
    Dst[0]->data[j][i] = Src[0]->data[j][i];\
    for(cn=1;cn<src_cn;cn++)\
        Dst[cn]->data[j][i] = Src[cn]->data[j][i];\
}
#define POINT_CACULATE(Src,Dst) {\
    Dst[0]->data[j][i] = Dst[0]->data[j][i]*0.9 + (float)(Src->data[0][j][i])*0.1;\
    for(cn=1;cn<src_cn;cn++)\
        Dst[cn]->data[j][i] = Dst[cn]->data[j][i]*0.9 + (float)(Src->data[cn][j][i])*0.1;\
}
#define POINT_GET(Src,Dst) {\
    Dst->data[0][j][i] = Src[0]->data[j][i];\
    for(cn=1;cn<src_cn;cn++)\
        Dst->data[cn][j][i] = Src[cn]->data[j][i];\
}

void mImageBackground(MImage *src,MImage *bgd,int time_thresh,int diff_thresh)
{
    int i,j,cn;
    int width,height;
    int src_cn;
    
    int flag;
    
    MMatrix *cnt;
    MMatrix **buff;
    MMatrix **rst;
    
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    mException((diff_thresh > 128),EXIT,"invalid input");
    height = src->height;
    width = src->width;
    src_cn = src->cn;
    
    MHandle *hdl; ObjectHandle(src,ImageBackground,hdl);
    struct HandleImageBackground *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        mException((INVALID_POINTER(bgd)||(bgd==src)),EXIT,"invalid input");
        mImageRedefine(bgd,src_cn,height,width);
        
        if(handle->cnt == NULL)
            handle->cnt = mMatrixCreate(height,width,NULL);
        else
            mMatrixRedefine(handle->cnt,height,width);
        
        for(cn=0;cn<src_cn;cn++)
        {
            if(handle->buff[cn] == NULL)
                handle->buff[cn] = mMatrixCreate(height,width,NULL);
            else
                mMatrixRedefine(handle->buff[cn],height,width);
            
            if(handle->rst[cn] == NULL)
                handle->rst[cn] = mMatrixCreate(height,width,NULL);
            else
                mMatrixRedefine(handle->rst[cn],height,width);
        }
            
        for(j=0;j<height;j++)
        {
            memset(handle->cnt->data[j],0,width*sizeof(float));
            for(i=0;i<width;i++)
            {
                POINT_INIT(src,(handle->rst));
                POINT_INIT(src,(handle->buff));
            }
        }
        
        hdl->valid = 1;
        
        mImageCopy(src,bgd);
        return;
    }
    else
    {
        mException(INVALID_IMAGE(bgd),EXIT,"invalid input");
        mException(((bgd->cn != src_cn)||(bgd->width != width)||(bgd->height != height)),EXIT,"invalid input");
    }
    
    if(time_thresh <= 0)
        time_thresh = 20;
    if(diff_thresh <= 0)
        diff_thresh = 10;
    
    cnt = handle->cnt;
    buff = handle->buff;
    rst = handle->rst;
    
    
    
    for(j=0;j<height;j++)
        for(i=0;i<width;i++)
        {
            DIFFERENT(src,rst,flag);
            if(flag)
            {
                DIFFERENT(src,buff,flag);
                if(flag)
                {
                    
                    if(cnt->data[j][i]>0.0f)
                        cnt->data[j][i] -= 1.0f;
                    else
                        POINT_INIT(src,buff)
                }
                else
                {
                    cnt->data[j][i] += 1.0f;
                    POINT_CACULATE(src,buff);
                    if(cnt->data[j][i] > time_thresh)
                    {
                        cnt->data[j][i] = 0.0f;
                        POINT_COPY(buff,rst);
                    }
                }
            }
            else
            {
                if(cnt->data[j][i]>0)
                    cnt->data[j][i] -= 1.0f;
                POINT_CACULATE(src,rst);
            }
            
            POINT_GET(rst,bgd);
            
            // if((i==333)&&(j==458))
                    // printf("src is %d,buff is %f,cnt is %d rst is %f,bgd is %d\n",src->data[0][j][i],buff[0]->data[j][i],cnt->data[0][j][i],rst[0]->data[j][i],bgd->data[0][j][i]);
        }
}
*/


/*
struct HandleVideoBackground
{
    MTable *value[3][MORN_MAX_IMAGE_CN];
    MTable *cnt[3];
    
    MImage *hst;
};

void endVideoBackground(void *handle)
{
    int cn;
    struct HandleVideoBackground *info;
    info = (struct HandleVideoBackground *)handle;

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
    mException(INVALID_IMAGE(bgd),EXIT,"invalid input");
    int height = src->height;
    int width = src->width;
    int src_cn = src->cn;
    mException(((bgd->cn != src_cn)||(bgd->width != width)||(bgd->height != height)),EXIT,"invalid input");
    
    MHandle *hdl; ObjectHandle(src,VideoBackground,hdl);
    struct HandleVideoBackground *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        mException((INVALID_POINTER(bgd)||(bgd==src)),EXIT,"invalid input");
        mImageRedefine(bgd,src_cn,height,width);

        for(int k=0;k<3;k++)
        {
            for(int cn=0;cn<src_cn;cn++)
            {
                if(handle->value[k][cn]==NULL) handle->value[k][cn]=mTableCreate(height,width,S16,NULL);
                else mTableRedefine(handle->value[k][cn],height,width,S16);
                
            }
            if(handle->cnt[k]==NULL) handle->cnt[k]=mTableCreate(height,width,S16,NULL);
            else mTableRedefine(handle->cnt[k],height,width,S16);
            for(int j=0;j<height;j++)
                memset(handle->cnt[k]->dataS16[j],0,width*sizeof(S16));
        }
        
        if(handle->hst==NULL) handle->hst = mImageCreate(src_cn,height,width,NULL);
        else mImageRedefine(handle->hst,src_cn,height,width);
        
        // mImageCopy(src,handle->hst,
        
        hdl->valid = 1;        
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
                    for(int l=0;l<3;l++)
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
*/


