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

void mImageBinaryEdge(MImage *src,MSheet *edge,MList *rect)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input image");
    mException((src->cn!=1),EXIT,"invalid input image");
    mException((edge==NULL)&&(rect==NULL),EXIT,"invalid input");
    
    MImagePoint point;
    unsigned char **data = src->data[0];
    
    MImageRect rect_buff;
    MSheet *edge_buff = edge;
    if(edge == NULL) edge_buff = mSheetCreate(DFLT,NULL,NULL);
    
    int k = 0;
    for(int j=0;j<src->height;j++)
        for(int i=0;i<src->width;i++)
        {
            if(data[j][i]==0)                      continue;
            if(data[j][i]!=255) {data[j][i] = 255; continue;}
            if(data[j][i-1]!=0)                    continue;
            if(data[j][i+1]+data[j+1][i]+data[j+1][i+1]<255*2) continue;
            
            int row = (edge==NULL)?0:k;
            
            rect_buff.x1 = i; rect_buff.x2 = i;
            rect_buff.y1 = j; rect_buff.y2 = j;
            
            point.x = i; point.y = j;
            mSheetWrite(edge_buff,row,0,&point,sizeof(MImagePoint));
            data[j][i] = 254;
            
            int order = 1;
            
            int x = i; int y = j;
            while(1)
            {
                #define SET_EDGE(X,Y) {\
                    point.x = X;point.y = Y;\
                    if(X<rect_buff.x1) rect_buff.x1 = X; else if(X+1>rect_buff.x2) rect_buff.x2 = X+1;\
                    if(Y<rect_buff.y1) rect_buff.y1 = Y; else if(Y+1>rect_buff.y2) rect_buff.y2 = Y+1;\
                    mSheetWrite(edge_buff,row,order,&point,sizeof(MImagePoint));\
                    order += 1;\
                    \
                    data[Y][X] = 254;\
                    x = X;y = Y;\
                    \
                    flag = 1;\
                }
                int flag = 0;
                     if((data[y  ][x-1] == 255)&&((data[y-1][x-1] == 0)||(data[y+1][x-1] == 0))) SET_EDGE(x-1,y  )
                else if((data[y-1][x-1] == 255)&&((data[y-1][x  ] == 0)||(data[y  ][x-1] == 0))) SET_EDGE(x-1,y-1)
                else if((data[y-1][x  ] == 255)&&((data[y-1][x+1] == 0)||(data[y-1][x-1] == 0))) SET_EDGE(x  ,y-1)
                else if((data[y-1][x+1] == 255)&&((data[y  ][x+1] == 0)||(data[y-1][x  ] == 0))) SET_EDGE(x+1,y-1)
                else if((data[y  ][x+1] == 255)&&((data[y+1][x+1] == 0)||(data[y-1][x+1] == 0))) SET_EDGE(x+1,y  )
                else if((data[y+1][x+1] == 255)&&((data[y+1][x  ] == 0)||(data[y  ][x+1] == 0))) SET_EDGE(x+1,y+1)
                else if((data[y+1][x  ] == 255)&&((data[y+1][x-1] == 0)||(data[y+1][x+1] == 0))) SET_EDGE(x  ,y+1)
                else if((data[y+1][x-1] == 255)&&((data[y  ][x-1] == 0)||(data[y+1][x  ] == 0))) SET_EDGE(x-1,y+1)
                    
                if(flag==0)
                {
                    order -= 1;if(order <= 1) break;
                    x = ((MImagePoint *)(edge_buff->data[row][order]))->x;
                    y = ((MImagePoint *)(edge_buff->data[row][order]))->y;
                }
                    
                if((ABS(x-i)<=1)&&(ABS(y-j)<=1)&&(order>4))
                    break;
            }
            
            data[j][i] = 255;
            
            if(order>1)
            {
                if(rect != NULL) mListWrite(rect,k,&rect_buff,sizeof(MImageRect));
                edge_buff->col[row] = order;
                k = k+1;
            }
        }
    if(rect != NULL) rect->num = k;
    if(edge != NULL) edge->row = k;
    if(edge == NULL) mSheetRelease(edge_buff);
}

void mImageBinaryFilter(MImage *src,MImage *dst,int r,int thresh)
{
    if(thresh < 0) thresh = 0;
    mException(INVALID_IMAGE(src)||(thresh>8),EXIT,"invalid input");
    
    MImage *p = dst;
    if((INVALID_POINTER(dst))||(dst==src))
        dst = mImageCreate(1,src->height,src->width,NULL);
    else
        mImageRedefine(dst,1,src->height,src->width);

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






