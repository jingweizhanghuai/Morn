/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
    mException((edge==NULL)&&(edge_rect==NULL),EXIT,"invalid input");
    
    mImageExpand(src,7,MORN_BORDER_BLACK);
    MHandle *hdl; ObjectHandle(src,ImageBinaryEdge,hdl);
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
    MImageRect rect;
    int xmin,xmax,ymin,ymax;
    
    int k = 0;
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
            int row = (edge==NULL)?0:k;
            
            int x = i; int y = j;
            xmin = x; xmax = x;
            ymin = y; ymax = y;
            point.x = x; point.y = y;
            mSheetWrite(edge_buff,row,0,&point,sizeof(MImagePoint));
            int order = 1;
            data[y][x] = 254;
            
            while(1)
            {
                #define SET_EDGE(X,Y) {\
                    x = X;y = Y;\
                    if(x<xmin) xmin = x; else if(x>=xmax) xmax = x+1;\
                    if(y<ymin) ymin = y; else if(y>=ymax) ymax = y+1;\
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
                    mRect(&rect,xmin,ymin,xmax,ymax);
                    mListWrite(edge_rect,k,&rect,sizeof(MImageRect));
                }
                edge_buff->col[row] = order;
                k = k+1;
            }

            for(int n=0;n<point_buff->num;n++)
            {
                int *p =point_buff->data[n];
                data[p[1]][p[0]]=255;
            }
            data[j][i] = 255;
        }
    }
    
    if(edge !=NULL) edge->row=k;
    if(edge_rect!=NULL) edge_rect->num=k;
}

void mImageBinaryFilter(MImage *src,MImage *dst,int r,int thresh)
{
    if(thresh < 0) thresh = 0;
    mException(INVALID_IMAGE(src)||(thresh>8),EXIT,"invalid input");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(1,src->height,src->width,NULL);
    else
        mImageRedefine(dst,1,src->height,src->width,dst->data);

    unsigned char **sdata = src->data[0];
    unsigned char **ddata = dst->data[0];
    int thresh1 = thresh*255;
    int thresh2 = (8-thresh)*255;
    
    #define BinaryFilter(X,Y) {\
        int Sum = sdata[Y-r][X-r] + sdata[Y-r][X] + sdata[Y-r][X+r] \
                + sdata[Y  ][X-r] +               + sdata[Y  ][X+r] \
                + sdata[Y+r][X-r] + sdata[Y+r][X] + sdata[Y+r][X+r];\
             if(Sum <= thresh1) ddata[Y][X] = 0;\
        else if(Sum >= thresh2) ddata[Y][X] = 255;\
        else                    ddata[Y][X] = sdata[Y][X];\
    }
    
    mImageRegion(src,r,BinaryFilter);
    
    memcpy(&(dst->info),&(src->info),sizeof(MInfo));
    
    if(p!=dst) { mImageExchange(src,dst); mImageRelease(dst);}
}






