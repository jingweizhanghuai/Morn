/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_image.h"

struct HoughLineInfo
{
    MImagePoint ps;
    MImagePoint pe;
    MImagePoint point1;
    MImagePoint point2;
    float k;
    float b;
    float a;
    float r;
    int count;
};

struct HandleImageHoughLine
{
    float sn[362];
    float cs[362];
    
    MTable *tab;
};

void endImageHoughLine(void *info)
{
    struct HandleImageHoughLine *handle = (struct HandleImageHoughLine *)info;
    
    if(handle->tab!=NULL)
        mTableRelease(handle->tab);
}
#define HASH_ImageHoughLine 0x3d5cf2dd

void mImageHoughLine(MImage *src,MList *list,int thresh1,int thresh2,int thresh)
{
    int j;
    mException(INVALID_IMAGE(src),EXIT,"invalid input source");
    mException((thresh<=0),EXIT,"invalid input threshold");
     
    if(thresh1<=0) thresh1=thresh;
    if(thresh2<=0) thresh2=0;
    
    int height = src->height;
    int width = src->width;
    
    float cx = (float)width/2.0f;
    float cy = (float)height/2.0f;
    
    unsigned char **data = src->data[0];
    int range = (int)(sqrt((double)(cx*cx+cy*cy)))+1;
    // printf("range is %d\n",range);
    
    MHandle *hdl; ObjectHandle(src,ImageHoughLine,hdl);
    struct HandleImageHoughLine *handle = (struct HandleImageHoughLine *)(hdl->handle);
    if(hdl->valid==0)
    {
        if(handle->tab==NULL) handle->tab = mTableCreate(range*2,362,U16,NULL);
        else                  mTableRedefine(handle->tab,range*2,362,U16,NULL);
        
        for(int i=0;i<362;i++)
        {
            handle->sn[i] = mSin(((float)i-1.0f)*0.5f);
            handle->cs[i] = mCos(((float)i-1.0f)*0.5f);
            // printf("sn[n] is %f,cs[n] is %f\n",sn[n],cs[n]);
        }
        
        hdl->valid = 1;
    }
    unsigned short **tab_data = handle->tab->dataU16;
    for(j=0;j<range*2;j++)
        memset(tab_data[j],0,handle->tab->col*sizeof(unsigned short));
    
    float *sn = handle->sn;
    float *cs = handle->cs;
    
    // mTimerBegin();
    
    #pragma omp parallel for
    for(j=0;j<height;j++)
        for(int i=0;i<width;i++)
        {
            if(data[j][i] == 255)
            {
                for(int n=0;n<362;n++)
                {
                    int l = (int)(((float)i-cx)*cs[n]+((float)j-cy)*sn[n]+(float)range+0.5);
                    tab_data[l][n] += 1;
                }
            }
        }
    // mTimerEnd();
    
    // printf("tab_data is %d,thresh is %d\n",tab_data[1169][125],(thresh<<2));
    // printf("tab_data[1169][124] is %d,tab_data[1169][126] is %d\n",tab_data[1169][124],tab_data[1169][126]);
    // printf("tab_data[1168][125] is %d,tab_data[1170][125] is %d\n",tab_data[1168][125],tab_data[1170][125]);
        
    list->num = 0;
    struct HoughLineInfo line;
    
    thresh  = thresh >>2;
    thresh1 = thresh1>>2;
    thresh2 = thresh2>>2;
    
    // mTimerBegin();
    for(j=0;j<range*2;j++)
    {
        for(int i=1;i<361;i++)
        {
            // if((i==125)&&(j==1169))
            // {
                // printf("tab_data is %d,thresh is %d\n",tab_data[j][i],(thresh<<2));
                // printf("tab_data[j][i-1] is %d,tab_data[j][i+1] is %d\n",tab_data[j][i-1],tab_data[j][i+1]);
            // }
            if(tab_data[j][i]<(thresh<<2))
                continue;
            
            if((tab_data[j][i]< tab_data[j][i-1])||(tab_data[j][i]< tab_data[j][i+1]))
                continue;
                
            tab_data[j][i] +=1;
            line.a = ((float)i-1.0f)*0.5f;
            line.r = ((float)(j-range));
            
            if(sn[i]==0.0f)
            {
                sn[i] = 0.0001f;
                cs[i] = (float)sqrt(1.0-sn[i]*sn[i]);
                // printf("sn is %f,cs is %f\t",sn,cs);
            }
            line.k = 0.0f-cs[i]/sn[i];
            line.b = line.r/sn[i]+cx*cs[i]/sn[i]+cy;
            // printf("k is %f,b is %f,a is %f,r is %f,tab_data is %d\n",k[m],b[m],a[m],r[m],tab_data[j][i]);
            
            line.point1.x = 0.0f;line.point1.y = line.b;
            if((line.point1.y<0)||(line.point1.y>=src->height))
            {
                line.point1.y = 0.0f;line.point1.x = (0.0f-line.b)/line.k;
                if((line.point1.x<0)||(line.point1.x>=src->width))
                    {line.point1.y = src->height-1;line.point1.x = (line.point1.y-line.b)/line.k;}
                if((line.point1.x<0)||(line.point1.x>=src->width)) continue;
            }
            
            line.point2.x = src->width;line.point2.y = line.k*line.point2.x + line.b;
            if((line.point2.y<0)||(line.point2.y>=src->height))
            {
                line.point2.y = src->height-1;line.point2.x = (line.point2.y-line.b)/line.k;
                if((line.point2.x<0)||(line.point2.x>=src->width))
                    {line.point2.y = 0.0f;line.point2.x = (0.0f-line.b)/line.k;}
                if((line.point2.x<0)||(line.point2.x>=src->width)) continue;
            }
            // if((i==125)&&(j==1169))
                // printf("line.point1 is %f,%f,line.point2 is %f,%f\n",line.point1.x,line.point1.y,line.point2.x,line.point2.y);
            
            int count1 = 0;
            int count2 = 0;
            int count = 0;
            line.count = 0;
            int state = 1;
            
            #define HOUGH_POINT_CHECK(U,V) {\
                int u=(int)(((float)U)+0.5);\
                int v=(int)(((float)V)+0.5);\
                if(state == 0)\
                {\
                         if(data[u  ][v  ] == 255) count1 += 1;\
                    else if(data[u-1][v  ]+data[u  ][v-1]+data[u  ][v+1]+data[u+1][v  ] != 0) ;\
                    else\
                    {\
                        if(count1 > thresh1)\
                        {\
                            line.count += count1;\
                            line.pe.x = V;\
                            line.pe.y = U;\
                            count2 = 1;\
                        }\
                        count += count1;\
                        count1 = 0;\
                        if((tab_data[j][i]/4 - count) < thresh1)\
                            break;\
                        state = 1;\
                    }\
                }\
                else\
                {\
                    if(data[u][v] != 255) count2 += 1;\
                    else\
                    {\
                        if(count2 > thresh2)\
                        {\
                            if(line.count > thresh)\
                            {\
                                mListWrite(list,DFLT,&line,sizeof(struct HoughLineInfo));\
                            }\
                            line.count = 0;\
                        }\
                        if(line.count==0) {line.ps.x = V; line.ps.y = U;}\
                        count1 = 1;\
                        state = 0;\
                    }\
                }\
            }
            
            int m,n;float l;
            if(ABS(line.point1.x-line.point2.x)>ABS(line.point1.y-line.point2.y))
            {
                float step = 4.0*line.k;
                if((line.point1.x)<(line.point2.x))
                {
                    for(n = line.point1.x,l = line.point1.y;n<line.point2.x;n=n+4,l = l+step) {HOUGH_POINT_CHECK(l,n);}
                    if(state == 0) {if(count1 > thresh1) {line.count += count1;line.pe.x = n;line.pe.y = l;}}
                }
                else
                {
                    for(n = line.point2.x,l = line.point2.y;n<line.point1.x;n=n+4,l = l+step) {HOUGH_POINT_CHECK(l,n);}
                    if(state == 0) {if(count1 > thresh1) {line.count += count1;line.pe.x = n;line.pe.y = l;}}
                }
            }
            else
            {
                float step = 4.0/line.k;
                if((line.point1.y)<(line.point2.y))
                {
                    for(m = line.point1.y,l = line.point1.x;m<line.point2.y;m=m+4,l = l+step) {HOUGH_POINT_CHECK(m,l);}
                    if(state == 0) {if(count1 > thresh1) {line.count += count1;line.pe.x = l;line.pe.y = m;}}
                }
                else
                {
                    for(m = line.point2.y,l = line.point2.x;m<line.point1.y;m=m+4,l = l+step) {HOUGH_POINT_CHECK(m,l);}
                    if(state == 0) {if(count1 > thresh1) {line.count += count1;line.pe.x = l;line.pe.y = m;}}
                }
            }
            if(line.count > thresh) 
            {
                // mLog(INFO,"%d:\t",list->num);
                // printf("tb_data[%d][%d] is %d,line.count is %d\n",j,i,tab_data[j][i],line.count*4);
                // printf("line.a is %f\t",line.a);
                // mLog(INFO,"line.ps is %f,%f\t",line.ps.x,line.ps.y);
                // mLog(INFO,"line.pe is %f,%f\n",line.pe.x,line.pe.y);
                mListWrite(list,DFLT,&line,sizeof(struct HoughLineInfo));
            }
        }
    }

    // mTimerEnd();
    // printf("time use is %f\n",mTimerUse());
}

void ImageHoughLineDrawImage(MImage *src,MList *list,char *filename)
{
    MImage *dst = mImageCreate(src->channel,src->height,src->width,NULL);
    mImageCopy(src,dst);
    unsigned char color[3] = {128,255,0};
    
    for(int i=0;i<list->num;i++)
    {
        MImagePoint *point = (MImagePoint *)(list->data[i]);
        mImageDrawLine(dst,NULL,&(point[0]),&(point[1]),color,4);
    }
    mBMPSave(dst,filename);
    mImageRelease(dst);
}