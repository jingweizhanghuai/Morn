/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_image.h"
#include "morn_image_caculate.h"
 
void mBinaryRect(MImage *src,int distance,int min_area,MImageRect *rst,int *num)
{
    int i,j,k,n;
    MImageRect rect[512];
    int rect_area[512];
    int rect_valid[512];
    
    unsigned char **data;
   
    mException(INVALID_POINTER(src)||INVALID_POINTER(rect),EXIT,"invalid input");
  
    data = src->data[0];
    
    #define SRC(x,y) data[y][x]
    
    n=0;
    for(j=ImageY1(src);j<ImageY2(src);j++)
        for(i=ImageX1(src,j);i<ImageX2(src,j);i++)
        {
            if(SRC(i,j)!= 255)
                continue;
            
            for(k=0;k<n;k++)
            {
                if((i>rect[k].x1-distance)&&(i<rect[k].x2+distance)&&(j<rect[k].y2+distance))
                {
                    rect[k].x1 = (i<rect[k].x1)?i:rect[k].x1;
                    rect[k].x2 = (i>rect[k].x2)?i:rect[k].x2;
                    rect[k].y2 = (j>rect[k].y2)?j:rect[k].y2;
                    rect_area[k] = rect_area[k]+1;
                    break;
                }
            }
            
            if(k==n)
            {
                rect[k].x1 = i;
                rect[k].x2 = i;
                rect[k].y1 = j;
                rect[k].y2 = j;
                
                rect_area[k]=1;
                rect_valid[k] =1;
                n = n+1;
            }
            
        }
        
    // printf("n is %d\n",n);
    // for(i=0;i<n;i++)
    // {
        // printf("rect is %d,%d,%d,%d,rect_area is %d\n",rect[i].x1,rect[i].x2,rect[i].y1,rect[i].y2,rect_area[i]);
    // }
        
    #define IS_AROUND_RECT(x,y,rect) ((x>rect.x1-distance)&&(x<rect.x2+distance)&&(y>rect.y1-distance)&&(y<rect.y2+distance))
    for(i=0;i<n;i++)
    {
        if(rect_valid[i]==0)
            continue;
        
        for(j=i+1;j<n;j++)
        {
            if(rect_valid[j]==0)
                continue;
            
            if(IS_AROUND_RECT(rect[j].x1,rect[j].y1,rect[i]))
            {
                rect[i].x1 = (rect[i].x1<rect[j].x1)?rect[i].x1:rect[j].x1;
                rect[i].x2 = (rect[i].x2>rect[j].x2)?rect[i].x2:rect[j].x2;
                rect[i].y1 = (rect[i].y1<rect[j].y1)?rect[i].y1:rect[j].y1;
                rect[i].y2 = (rect[i].y2>rect[j].y2)?rect[i].y2:rect[j].y2;
                rect_area[i] = rect_area[i]+rect_area[j];
                rect_valid[j] = 0;
            }
            else if(IS_AROUND_RECT(rect[j].x1,rect[j].y2,rect[i]))
            {
                rect[i].x1 = (rect[i].x1<rect[j].x1)?rect[i].x1:rect[j].x1;
                rect[i].x2 = (rect[i].x2>rect[j].x2)?rect[i].x2:rect[j].x2;
                rect[i].y1 = (rect[i].y1<rect[j].y1)?rect[i].y1:rect[j].y1;
                rect[i].y2 = (rect[i].y2>rect[j].y2)?rect[i].y2:rect[j].y2;
                rect_area[i] = rect_area[i]+rect_area[j];
                rect_valid[j] = 0;
            }
            else if(IS_AROUND_RECT(rect[j].x2,rect[j].y1,rect[i]))
            {
                rect[i].x1 = (rect[i].x1<rect[j].x1)?rect[i].x1:rect[j].x1;
                rect[i].x2 = (rect[i].x2>rect[j].x2)?rect[i].x2:rect[j].x2;
                rect[i].y1 = (rect[i].y1<rect[j].y1)?rect[i].y1:rect[j].y1;
                rect[i].y2 = (rect[i].y2>rect[j].y2)?rect[i].y2:rect[j].y2;
                rect_area[i] = rect_area[i]+rect_area[j];
                rect_valid[j] = 0;
            }
            else if(IS_AROUND_RECT(rect[j].x2,rect[j].y2,rect[i]))
            {
                rect[i].x1 = (rect[i].x1<rect[j].x1)?rect[i].x1:rect[j].x1;
                rect[i].x2 = (rect[i].x2>rect[j].x2)?rect[i].x2:rect[j].x2;
                rect[i].y1 = (rect[i].y1<rect[j].y1)?rect[i].y1:rect[j].y1;
                rect[i].y2 = (rect[i].y2>rect[j].y2)?rect[i].y2:rect[j].y2;
                rect_area[i] = rect_area[i]+rect_area[j];
                rect_valid[j] = 0;
            }
        }
    }
    
    // printf("n is %d\n",n);
    // for(i=0;i<n;i++)
    // {
        // printf("rect is %d,%d,%d,%d,rect_valid is %d,rect_area is %d\n",rect[i].x1,rect[i].x2,rect[i].y1,rect[i].y2,rect_valid[i],rect_area[i]);
    // }
    
    k=0;
    for(i=0;i<n;i++)
    {
        if(rect_valid[i] == 1)
            if(rect_area[i]>min_area)
            {
                rst[k] = rect[i];
                k = k+1;
            }
    }
    
    *num = k;
}

struct HandleImageBinaryDenoise
{
    int *lx;
    int *ly;
    int num;
};
void endImageBinaryDenoise(void *info)
{
    struct HandleImageBinaryDenoise *handle = (struct HandleImageBinaryDenoise *)info;
    if(handle->lx != NULL) mFree(handle->lx);
    if(handle->ly != NULL) mFree(handle->ly);
}
#define HASH_ImageBinaryDenoise 0x2dc7afe2
void _BinaryDenoise(MImage *src,int *x,int y,int *num,int *lx,int *ly,int thresh)
{
    int i;int min,max;
    unsigned char *data = src->data[0][y];

    for(i=*x  ;i<src->width;i++) {if(data[i]==0) break;if(*num<thresh){lx[*num]=i;ly[*num]=y;(*num)++;}data[i]=254;} max = i;
    for(i=*x-1;i>=0;        i--) {if(data[i]==0) break;if(*num<thresh){lx[*num]=i;ly[*num]=y;(*num)++;}data[i]=254;} min = i;
    *x=max;
    
    data = src->data[0][y-1]; for(i=min;i<=max;i++) {if(data[i]==255) _BinaryDenoise(src,&i,y-1,num,lx,ly,thresh);}
    data = src->data[0][y+1]; for(i=min;i<=max;i++) {if(data[i]==255) _BinaryDenoise(src,&i,y+1,num,lx,ly,thresh);}
}
void mImageBinaryDenoise(MImage *src,MImage *dst,int num_thresh)
{
    mException(src==NULL,EXIT,"invalid input image");
    mException((src->channel!=1),EXIT,"invalid input image type");
    mException((num_thresh<=0),EXIT,"invalid threshold %d",num_thresh);
    
    MHandle *hdl=mHandle(src,ImageBinaryDenoise);
    struct HandleImageBinaryDenoise *handle = (struct HandleImageBinaryDenoise *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->num<num_thresh)
        {
            if(handle->lx!=NULL) mFree(handle->lx); handle->lx=mMalloc(num_thresh*sizeof(int));
            if(handle->ly!=NULL) mFree(handle->ly); handle->ly=mMalloc(num_thresh*sizeof(int));
            handle->num = num_thresh;
        }
        hdl->valid = 1;
    }
    
    if(dst==NULL) dst=src;
    if(src!=dst) {mImageCopy(src,dst);src=dst;}

    int *lx =handle->lx;int *ly = handle->ly;
    unsigned char **data = src->data[0];
    for(int j=0;j<src->height;j++)for(int ii=0;ii<src->width;ii+=8)
    {
        uint64_t *pdata = (uint64_t *)(&(data[j][ii]));
        if(*pdata==0) continue;
        if(*pdata==0xFEFEFEFEFEFEFEFE) {*pdata=0xFFFFFFFFFFFFFFFF;continue;}
        for(int i=ii;i<ii+8;i++)
        {
            if(data[j][i] ==  0) continue;
            if(data[j][i] ==254){data[j][i] =255;continue;}
            
            int num=0;int x=i;
            _BinaryDenoise(src,&x,j,&num,lx,ly,num_thresh);
            if(num<num_thresh) for(int n=0;n<num;n++) {data[ly[n]][lx[n]]=0;}
            else data[j][i]=255;
        }
    }
}

// unsigned char _burr_array[256] = {0,0,0,0,0,1,0,1,0,0,0,0,0,1,0,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,0,0,0,0,0,1,0,1,0,0,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,0,1,0,0,0,1,0,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,0,0,0,1,0,1,1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
void _PointDelete(unsigned char **data,int x,int y)
{
    // int value =(data[y-1][x-1]&0x80)+(data[y-1][x]&0x40)+(data[y-1][x+1]&0x20)
    //           +(data[y  ][x-1]&0x01)                    +(data[y  ][x+1]&0x10)
    //           +(data[y+1][x-1]&0x02)+(data[y+1][x]&0x04)+(data[y+1][x+1]&0x08);
    // if(_burr_array[value] == 1) return;
    int sum = data[y][x-1]+data[y][x+1];if(sum>255) return;
    sum+=data[y-1][x];if(sum>255) return;
    sum+=data[y+1][x];if(sum>255) return;
    data[y][x]=0;
    if(data[y-1][x-1]) _PointDelete(data,x-1,y-1);
    if(data[y-1][x  ]) _PointDelete(data,x  ,y-1);
    if(data[y-1][x+1]) _PointDelete(data,x+1,y-1);
    if(data[y  ][x-1]) _PointDelete(data,x-1,y  );
    if(data[y  ][x+1]) _PointDelete(data,x+1,y  );
    if(data[y+1][x-1]) _PointDelete(data,x-1,y+1);
    if(data[y+1][x  ]) _PointDelete(data,x  ,y+1);
    if(data[y+1][x+1]) _PointDelete(data,x+1,y+1);
}

void mImageBinaryBurrRemove(MImage *src,MImage *dst)
{
    mException(src==NULL,EXIT,"invalid input image");
    mException((src->channel!=1),EXIT,"invalid input image type");

    if(dst==NULL) dst=src;
    if(src!=dst) {mImageCopy(src,dst);src=dst;}

    unsigned char **data = src->data[0];
    for(int j=0;j<src->height;j++)for(int ii=0;ii<src->width;ii+=8)
    {
        uint64_t *pdata = (uint64_t *)(&(data[j][ii]));
        if(*pdata==0) continue;
        for(int i=ii;i<ii+8;i++)
        {
            if(data[j][i]==0) continue;
            _PointDelete(data,i,j);
        }
    }
}

struct HandleImageBinaryArea
{
    int x;
    int y;
};
void endImageBinaryArea(void *info) {NULL;}
#define HASH_ImageBinaryArea 0x2e4ea8f2
void _BinaryArea(MImage *src,int *x,int y,MList *list)
{
    int i;int min,max;
    unsigned char *data = src->data[0][y];
    MImagePoint pt;pt.y=y;

    for(i=*x  ;i<src->width;i++) {if(data[i]==0) break;pt.x=i;mListWrite(list,DFLT,&pt,sizeof(MImagePoint)); data[i]=254;} max = i;
    for(i=*x-1;i>=0;        i--) {if(data[i]==0) break;pt.x=i;mListWrite(list,DFLT,&pt,sizeof(MImagePoint)); data[i]=254;} min = i;
    *x=max;
    
    data = src->data[0][y-1]; for(i=min;i<=max;i++) {if(data[i]==255) _BinaryArea(src,&i,y-1,list);}
    data = src->data[0][y+1]; for(i=min;i<=max;i++) {if(data[i]==255) _BinaryArea(src,&i,y+1,list);}
}
int mImageBinaryArea(MImage *src,MList *list,MImagePoint *point)
{
    mException(src==NULL,EXIT,"invalid input image");
    mException((src->channel!=1),EXIT,"invalid input image type");
    mException(list==NULL,EXIT,"invalid result list");
    mListClear(list);
    unsigned char **data = src->data[0];
    
    if(point!=NULL) 
    {
        int x=point->x;int y=point->y;
        if(data[y][x]!=255) return MORN_FAIL;
        _BinaryArea(src,&x,y,list);
        for(int i=0;i<list->num;i++)
        {
            MImagePoint *p=(MImagePoint *)(list->data[i]);
            data[(int)(p->y)][(int)(p->x)]=255;
        }
        return MORN_SUCCESS;
    }
    
    MHandle *hdl=mHandle(src,ImageBinaryArea);
    struct HandleImageBinaryArea *handle = (struct HandleImageBinaryArea *)(hdl->handle);
    if(hdl->valid==0){handle->x=0;handle->y=0;hdl->valid = 1;}
    
    for(int j=handle->y;j<src->height;j++)
    {
        if(j==handle->y)
        {
            for(int i=handle->x;i<src->width;i++) 
            {
                if(data[j][i] ==  0) continue;
                if(data[j][i] ==254){data[j][i] =255;continue;}
                int lx=i;_BinaryArea(src,&lx,j,list);
                handle->y=j;handle->x=i+1;
                data[j][i] = 255;
                return MORN_SUCCESS;
            }
        }
        else
        {
            for(int ii=0;ii<src->width;ii+=8)
            {
                uint64_t *pdata = (uint64_t *)(&(data[j][ii]));
                if(*pdata==0) continue;
                if(*pdata==0xFEFEFEFEFEFEFEFE) {*pdata=0xFFFFFFFFFFFFFFFF;continue;}
                for(int i=ii;i<ii+8;i++)
                {
                    if(data[j][i] ==  0) continue;
                    if(data[j][i] ==254){data[j][i] =255;continue;}
                    int lx=i;_BinaryArea(src,&lx,j,list);
                    handle->y=j;handle->x=i+1;
                    data[j][i] = 255;
                    return MORN_SUCCESS;
                }
            }
        }
    }
    handle->x=0;handle->y=0;
    return MORN_FAIL;
}

struct HandleImageBinaryEdge
{
    MList *point_buff;
    MSheet *edge_buff;
};
void endImageBinaryEdge(void *info)
{
    struct HandleImageBinaryEdge *handle = (struct HandleImageBinaryEdge *)info;
    if(handle->point_buff!= NULL) mListRelease(handle->point_buff);
    if(handle->edge_buff != NULL) mSheetRelease(handle->edge_buff);
}
#define HASH_ImageBinaryEdge 0xd7a1d30e
void mImageBinaryEdge(MImage *src,MSheet *edge,MList *edge_rect)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input image");
    mException((src->channel!=1),EXIT,"invalid input image type");
    mException((edge==NULL)&&(edge_rect==NULL),EXIT,"invalid input");
    
    mImageExpand(src,7,MORN_BORDER_BLACK);
    MHandle *hdl=mHandle(src,ImageBinaryEdge);
    struct HandleImageBinaryEdge *handle = (struct HandleImageBinaryEdge *)(hdl->handle);
    if(hdl->valid ==0)
    {
        if(handle->point_buff==NULL) handle->point_buff=mListCreate(DFLT,NULL);
        hdl->valid = 1;
    }
    MList *point_buff = handle->point_buff;
    MSheet *edge_buff = edge;
    if(edge == NULL) 
    {
        if(handle->edge_buff==NULL) handle->edge_buff=mSheetCreate(1,NULL,NULL);
        edge_buff = handle->edge_buff;
    }
    
    MImagePoint point;
    unsigned char **data = src->data[0];

    int data1,data2,data3,data4,data5,data6,data7,data8;
    
    int row = 0;
    for(int j=0;j<src->height;j++)for(int ii=0;ii<src->width;ii+=8)
    {
        uint64_t *pdata = (uint64_t *)(&(data[j][ii]));
        if(*pdata==0) continue;
        for(int i=ii;i<ii+8;i++)
        {
            if(data[j][i]==0)  continue;
            if(data[j][i]!=255) {data[j][i] = 255; continue;}
            if(data[j][i-1]!=0) continue; 
            // if((data[j][i-1]!=0)&&(data[j+1][i-1]!=0)) continue; 
            if(data[j][i+1]+data[j+1][i]+data[j+1][i+1]<=510) continue;

            point_buff->num=0;
            
            int x = i; int y = j;
            point.x = x; point.y = y;
            mSheetWrite(edge_buff,row,0,&point,sizeof(MImagePoint));
            int order = 1;
            data[y][x] = 254;
            
            while(1)
            {
                #define SET_EDGE(X,Y) {\
                    x = X;y = Y;\
                    point.x = x;point.y = y;\
                    mSheetWrite(edge_buff,row,order,&point,sizeof(MImagePoint));\
                    order += 1;\
                    data[y][x] = 254;\
                }
                
                data8=data[y-1][x  ];
                data1=data[y-1][x+1];
                data2=data[y  ][x+1];if((data1==255)&&((data2&data8)==0)) {SET_EDGE(x+1,y-1);continue;}
                data3=data[y+1][x+1];if((data2==255)&&((data3&data1)==0)) {SET_EDGE(x+1,y  );continue;}
                data4=data[y+1][x  ];if((data3==255)&&((data4&data2)==0)) {SET_EDGE(x+1,y+1);continue;}
                data5=data[y+1][x-1];if((data4==255)&&((data5&data3)==0)) {SET_EDGE(x  ,y+1);continue;}
                data6=data[y  ][x-1];if((data5==255)&&((data6&data4)==0)) {SET_EDGE(x-1,y+1);continue;}
                data7=data[y-1][x-1];if((data6==255)&&((data7&data5)==0)) {SET_EDGE(x-1,y  );continue;}
                                     if((data7==255)&&((data8&data6)==0)) {SET_EDGE(x-1,y-1);continue;}
                                     if((data8==255)&&((data1&data7)==0)) {SET_EDGE(x  ,y-1);continue;}

                if((ABS(x-i)<=1)&&(ABS(y-j)<=1)&&(order>4)) break;
                
                int buff[2]={x,y};
                mListWrite(point_buff,DFLT,buff,sizeof(MImagePoint));
                
                order -= 1;if(order <= 1) break;
                x = ((MImagePoint *)(edge_buff->data[row][order-1]))->x;
                y = ((MImagePoint *)(edge_buff->data[row][order-1]))->y;
            }
            
            if(order>1)
            {
                if(edge_rect!=NULL) 
                {
                    float xmin=i;float xmax=i;float ymin=j;float ymax=j;
                    for(int n=0;n<order;n++)
                    {
                        MImagePoint *pt=edge_buff->data[row][n];
                        if(pt->x<xmin) xmin=pt->x;else if(pt->x>xmax) xmax=pt->x;
                        if(pt->y<ymin) ymin=pt->y;else if(pt->y>ymax) ymax=pt->y;
                    }
                    for(int n=0;n<point_buff->num;n++)
                    {
                        int *p =point_buff->data[n];
                        if(p[0]<xmin) xmin=p[0];else if(p[0]>xmax) xmax=p[0];
                        if(p[1]<ymin) ymin=p[1];else if(p[1]>ymax) ymax=p[1];
                    }
                    MImageRect rect;mRect(&rect,xmin,ymin,xmax,ymax);
                    mListWrite(edge_rect,row,&rect,sizeof(MImageRect));
                }
                if(edge!=NULL) {edge_buff->col[row] = order; row++;}
            }

            for(int n=0;n<point_buff->num;n++)
            {
                int *p =point_buff->data[n];
                data[p[1]][p[0]]=255;
            }
            data[j][i] = 255;
        }
    }
}

void mImageBinaryFilter(MImage *src,MImage *dst,int r,float threshold1,float threshold2)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src)) dst = mImageCreate(1,src->height,src->width,NULL);
    else                                   mImageRedefine(dst,1,src->height,src->width,dst->data);
    
    unsigned char **sdata = src->data[0];
    unsigned char **ddata = dst->data[0];
    float a=(r+r+1)*(r+r+1);
    int thresh1 = threshold1*a*255;int thresh2 = threshold2*a*255;
    
    #define BinaryFilter(X,Y) {\
        int Sum=0;\
        for(int j=Y-r;j<=Y+r;j++)for(int i=X-r;i<X+r;i++)\
            Sum+=sdata[j-r][i-r];\
             if(Sum < thresh1) ddata[Y][X] = 0;\
        else if(Sum > thresh2) ddata[Y][X] = 255;\
        else                   ddata[Y][X] = sdata[Y][X];\
    }
    
    mImageRegion(src,r,BinaryFilter);
    
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}






