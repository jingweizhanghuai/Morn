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

struct HandleImageSeperate
{
    MList *list;
};
void endImageSeperate(void *info)
{
    struct HandleImageSeperate *handle = (struct HandleImageSeperate *)info;
    if(handle->list!=NULL) mListRelease(handle->list);
}
#define HASH_ImageSeperate 0x50c2063
void mImageSeperate(MImage *src,MImage *dst,MImageRect *rect,MImagePoint *point,int thresh)
{
    int height = src->height;int width=src->width;
    
    MHandle *hdl=mHandle(src,ImageSeperate);
    struct HandleImageSeperate *handle = hdl->handle;
    if(hdl->valid==0)
    {
        handle->list = mListCreate(DFLT,NULL);
        hdl->valid=1;
    }
    MList *list=handle->list;
    mListClear(list);

    MImageRect seprate_rect;
    if(rect==NULL) rect = &seprate_rect;
    
    float k = (float)thresh/255+1.0f;
    int grid=MIN(height/64,width/64);grid=MAX(grid,2);
    int locate[2] = {(int)(point->y+0.5),(int)(point->x+0.5)};

    mListWrite(list,DFLT,locate,2*sizeof(int));
    unsigned char **r=src->data[2];unsigned char **g=src->data[1];unsigned char **b=src->data[0];
    char **d = (char **)(dst->data[0]);

    for(int n=0;n<list->num;n++)
    {
        int *l=list->data[n];int y=l[0];int x=l[1];
        int r1=r[y][x]-thresh; int r2=r[y][x]+thresh;
        int g1=g[y][x]-thresh; int g2=g[y][x]+thresh;
        int b1=b[y][x]-thresh; int b2=b[y][x]+thresh;
        float k1=((float)r[y][x]+32.0f)/((float)g[y][x]+32.0f);float k11=k1/k;float k12=k1*k;
        float k2=((float)r[y][x]+32.0f)/((float)b[y][x]+32.0f);float k21=k2/k;float k22=k2*k;
        float k3=((float)b[y][x]+32.0f)/((float)g[y][x]+32.0f);float k31=k3/k;float k32=k3*k;

        int x1=MAX(x-grid,0);int x2=MIN(x+grid, width-1);
        int y1=MAX(y-grid,0);int y2=MIN(y+grid,height-1);

        int valid = 1;
        for(int j=y1;j<=y2;j++)for(int i=x1;i<=x2;i++)
        {
            if(d[j][i]!=-1)continue;
            if(r[j][i]<r1) {continue;} if(r[j][i]>r2) {continue;}
            if(g[j][i]<g1) {continue;} if(g[j][i]>g2) {continue;}
            if(b[j][i]<b1) {continue;} if(b[j][i]>b2) {continue;}
            k1=((float)r[j][i]+32.0f)/((float)g[j][i]+32.0f);if(k1<k11) {continue;} if(k1>k12) {continue;}
            k2=((float)r[j][i]+32.0f)/((float)b[j][i]+32.0f);if(k2<k21) {continue;} if(k2>k22) {continue;}
            k3=((float)b[j][i]+32.0f)/((float)g[j][i]+32.0f);if(k3<k31) {continue;} if(k3>k32) {continue;}
            valid =0; goto ImageSeperate_next;
        }
        ImageSeperate_next:
        if(valid == 0) continue;

        if(d[y1][x1]==1) d[y1][x1]=2;
        if(d[y1][x2]==1) d[y1][x2]=2;
        if(d[y2][x1]==1) d[y2][x1]=2;
        if(d[y2][x2]==1) d[y2][x2]=2;
        for(int j=y1;j<=y2;j++)for(int i=x1;i<=x2;i++)
        {
            if(d[j][i]!=0)continue;
            d[j][i]=-1;
            if(r[j][i]<r1) {continue;} if(r[j][i]>r2) {continue;}
            if(g[j][i]<g1) {continue;} if(g[j][i]>g2) {continue;}
            if(b[j][i]<b1) {continue;} if(b[j][i]>b2) {continue;}
            k1=((float)r[j][i]+32.0f)/((float)g[j][i]+32.0f);if(k1<k11) {continue;} if(k1>k12) {continue;}
            k2=((float)r[j][i]+32.0f)/((float)b[j][i]+32.0f);if(k2<k21) {continue;} if(k2>k22) {continue;}
            k3=((float)b[j][i]+32.0f)/((float)g[j][i]+32.0f);if(k3<k31) {continue;} if(k3>k32) {continue;}
            d[j][i]=1;
            rect->x1=MIN(rect->x1,i);rect->x2=MAX(rect->x2,i);
            rect->y1=MIN(rect->y1,j);rect->y2=MAX(rect->y2,j);
        }

        if(d[y1][x1]==1) {locate[0]=y1;locate[1]=x1;mListWrite(list,DFLT,locate,2*sizeof(int));}
        if(d[y1][x2]==1) {locate[0]=y1;locate[1]=x2;mListWrite(list,DFLT,locate,2*sizeof(int));}
        if(d[y2][x1]==1) {locate[0]=y2;locate[1]=x1;mListWrite(list,DFLT,locate,2*sizeof(int));}
        if(d[y2][x2]==1) {locate[0]=y2;locate[1]=x2;mListWrite(list,DFLT,locate,2*sizeof(int));}
    }

    for(int j=MAX(rect->y1-grid,0);j<=MIN(rect->y2+grid,height-1);j++)
    for(int i=MAX(rect->x1-grid,0);i<=MIN(rect->x2+grid,width -1);i++)
        dst->data[0][j][i]=(d[j][i]>0)?255:0;
}


/*
#define MID(X1,X2,X3) (((X1>X2)==(X2>=X3))?X2:(((X1>X2)==(X3>=X1))?X1:X3))
#define MID_FILTER(j,i)\
{\
    int j1=j-dy;int j2=j+dy;if(j1<0)j1=j2;else if(j2>=height)j2=j1;\
    int i1=i-dx;int i2=i+dx;if(i1<0)i1=i2;else if(i2>=width )i2=i1;\
    for(int cn=0;cn<src->channel;cn++)\
    {\
        data1 = data[cn][j1][i1];\
        data2 = data[cn][j ][i2];\
        data3 = data[cn][j2][i ];mid1 = MID(data1,data2,data3);\
        data1 = data[cn][j1][i2];\
        data2 = data[cn][j ][i1];\
        data3 = data[cn][j2][i2];mid2 = MID(data1,data2,data3);\
        data1 = data[cn][j1][i ];\
        data2 = data[cn][j ][i ];\
        data3 = data[cn][j2][i1];mid3 = MID(data1,data2,data3);\
        img->data[cn][n][m]=MID(mid1,mid2,mid3);\
    }\
}

void SeperateImage(MImage *src,MImage *img,MImagePoint *point)
{
    int height= src->height; int y_step = (height-1)/31;
    int width = src->width;  int x_step = (width -1)/31;
    int dy = y_step/4;dy=MAX(2,dy);
    int dx = x_step/4;dx=MAX(2,dx);
    // printf("dx is %d,dy is %d\n",dx,dy);
    
    unsigned char ***data = src->data;
    int data1,data2,data3;
    int mid1,mid2,mid3;
    
    int i,j,m,n;
    for(j=0,n=0;n<32;j+=y_step,n++)
        for(i=0,m=0;m<32;i+=x_step,m++)
        {
            MID_FILTER(j,i)
        }
        
    i = (int)(point->x);
    j = (int)(point->y);
    m = (int)(point->x/(float)x_step+0.5);
    n = (int)(point->y/(float)y_step+0.5);
    MID_FILTER(j,i);
}

#define DIFF(data1,M1,N1,data2,M2,N2,Diff)\
{\
    int B1=data1[0][N1][M1];int B2=data2[0][N2][M2];int Diff0 = ABS(B1-B2);\
    Diff = Diff0;if((Diff<thresh)&&(img->channel>1)){\
    int G1=data1[1][N1][M1];int G2=data2[1][N2][M2];int Diff1 = ABS(G1-G2);\
    float K1=(float)(B1*G2)/(float)(B2*G1);K1=MAX(K1,1.0f/K1);\
    Diff = MAX(Diff0,Diff1)*K1;if((Diff<thresh)&&(img->channel>2)){\
    int R1=data1[2][N1][M1];int R2=data2[2][N2][M2];int Diff2 = ABS(R1-R2);\
    float K2=(float)(R1*G2)/(float)(R2*G1);K2=MAX(K2,1.0f/K2);\
    float K3=(float)(R1*B2)/(float)(R2*B1);K3=MAX(K3,1.0f/K3);\
    Diff = MAX(MAX(Diff0,Diff1),Diff2)*MAX(MAX(K1,K2),K3);}}\
}

#define IMG_CHECK(M1,N1,M2,N2,Flag) {\
    Flag = 0;\
    if((M2>=0)&&(N2>=0)&&(M2<32)&&(N2<32))\
    {\
        if(tab[N2][M2]==1)\
        {\
            int Diff;DIFF(img->data,M1,N1,img->data,M2,N2,Diff);\
            Flag = (Diff>thresh)?-1:1;\
        }\
        else if(tab[N2][M2] == -1)\
        {\
            int Diff;DIFF(img->data,M1,N1,img->data,M2,N2,Diff);\
            Flag = (Diff>thresh)?0:-1;\
        }\
    }\
}

#define SRC_CHECK(X,Y,M,N,Flag) {\
    Flag = 0;\
    if((M>=0)&&(N>=0)&&(M<32)&&(N<32))\
    {\
        if(tab[N][M]==1)\
        {\
            Flag = 1;\
            for(int cn=0;cn<img->channel;cn++)\
            {\
                if(ABS(src->data[cn][Y][X] - img->data[cn][N][M])>thresh*2)\
                    {Flag = 0;break;}\
            }\
        }\
    }\
}

struct HandleImageSeperate
{
    MImage *img;
};
void endImageSeperate(void *info)
{
    struct HandleImageSeperate *handle = (struct HandleImageSeperate *)info;
    if(handle->img!= NULL) mImageRelease(handle->img);
}
#define HASH_ImageSeperate 0x50c2063
int mImageSeperate1(MImage *src,MImage *dst,MImagePoint *point,int thresh)
{
    int i,j,cn;
    int flag,valid;
    
    int height = src->height;
    int width = src->width;
    int channel=src->channel;
    unsigned char ***data = src->data;
    
    int tab[32][32];
    memset(tab,0,1024*sizeof(int));
    
    MHandle *hdl; ObjectHandle(src,ImageSeperate,hdl);
    struct HandleImageSeperate *handle = (struct HandleImageSeperate *)(hdl->handle);
    if(hdl->valid == 0)
    {
        handle->img = mImageCreate(src->channel,32,32,NULL);
        SeperateImage(src,handle->img,point);
        mImageMidValueFilter(handle->img,handle->img);
        // mImageSave(handle->img,"./test_ImageSeperate1.bmp");
        hdl->valid = 1;
    }
    MImage *img = handle->img;
    
    int x_step = (width-1)/31;
    int y_step = (height-1)/31;
    
    int x0 = (int)(point->x);int m = (int)(point->x/(float)x_step+0.5);
    int y0 = (int)(point->y);int n = (int)(point->y/(float)y_step+0.5);
    tab[n][m] = 1;
    // printf("m is %d,n is %d\n",m,n);
    
    int x,y;
    
    valid = 0;
    
    IMG_CHECK(m-1,n-1,m,n,flag);if(flag>0){tab[n-1][m-1]=1;valid=1;}
    IMG_CHECK(m  ,n-1,m,n,flag);if(flag>0){tab[n-1][m  ]=1;valid=1;}
    IMG_CHECK(m+1,n-1,m,n,flag);if(flag>0){tab[n-1][m+1]=1;valid=1;}
    IMG_CHECK(m-1,n  ,m,n,flag);if(flag>0){tab[n  ][m-1]=1;valid=1;}
    IMG_CHECK(m+1,n  ,m,n,flag);if(flag>0){tab[n  ][m+1]=1;valid=1;}
    IMG_CHECK(m-1,n+1,m,n,flag);if(flag>0){tab[n+1][m-1]=1;valid=1;}
    IMG_CHECK(m  ,n+1,m,n,flag);if(flag>0){tab[n+1][m  ]=1;valid=1;}
    IMG_CHECK(m+1,n+1,m,n,flag);if(flag>0){tab[n+1][m+1]=1;valid=1;}
    
    int sum1,sum2;
    for(int time=0;time<10;time++)
    {
        if(valid == 0) break;valid = 0;
        for(n=0;n<32;n++)
            for(m=0;m<32;m++)
            {
                if(tab[n][m] != 0) continue;
                
                sum1 = 0;sum2 = 0;
                IMG_CHECK(m,n,m+1,n+1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m+1,n  ,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m+1,n-1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m  ,n+1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m  ,n-1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m-1,n+1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m-1,n  ,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m-1,n-1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                if(ABS(sum1-sum2>=2)) {valid =1;tab[n][m] = (sum1>sum2)?1:-1;continue;}
                
                if((n>0)&&(m>0)&&(n<31)&&(m<31))
                    if(tab[n-1][m-1]+tab[n-1][m]+tab[n-1][m+1]+
                       tab[n  ][m-1]            +tab[n  ][m+1]+
                       tab[n+1][m-1]+tab[n+1][m]+tab[n+1][m+1]>=7)
                        tab[n][m]=1;
            }
        
        if(valid == 0) break;valid = 0;
        for(n=31;n>=0;n--)
            for(m=31;m>=0;m--)
            {
                if(tab[n][m] != 0) continue;
                
                sum1 = 0;sum2 = 0;
                IMG_CHECK(m,n,m-1,n-1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m-1,n  ,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m-1,n+1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m  ,n-1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m  ,n+1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m+1,n-1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m+1,n  ,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                IMG_CHECK(m,n,m+1,n+1,flag);if(flag>0)sum1+=1;else if(flag<0)sum2+=1;
                if(ABS(sum1-sum2>=2)) {valid =1;tab[n][m] = (sum1>sum2)?1:-1;continue;}
            
                if((n>0)&&(m>0)&&(n<31)&&(m<31))
                    if(tab[n-1][m-1]+tab[n-1][m]+tab[n-1][m+1]+
                       tab[n  ][m-1]            +tab[n  ][m+1]+
                       tab[n+1][m-1]+tab[n+1][m]+tab[n+1][m+1]>=7)
                        tab[n][m]=1;
            }
    }
    
    // for(n=1;n<31;n++)for(m=1;m<31;m++)
    // {
        // if(tab[n][m]!=1)
        // if(tab[n-1][m-1]+tab[n-1][m]+tab[n-1][m+1]+
           // tab[n  ][m-1]            +tab[n  ][m+1]+
           // tab[n+1][m-1]+tab[n+1][m]+tab[n+1][m+1]>=7)
           // tab[n][m]=1;
    // }
    
    int count=0;
    for(j=0;j<height;j++)
    {
        n = j/y_step;if(n>31) n=31;
        for(i=0;i<width;i++)
        {
            m = i/x_step;if(m>31) m=31;
            
            #define VALID_LOCATE(N,M) (((M<0)||(N<0)||(M>31)||(N>31))?0:(tab[N][M]>0))
            int sum = VALID_LOCATE(n,m)+VALID_LOCATE(n,m+1)+VALID_LOCATE(n+1,m)+VALID_LOCATE(n+1,m+1);
            if(sum == 4)
                dst->data[0][j][i] = 255;
            else if(sum == 0)
                dst->data[0][j][i] = 0;
            else
            {
                sum=0;
                SRC_CHECK(i,j,m  ,n  ,flag);sum=sum+flag;
                SRC_CHECK(i,j,m+1,n  ,flag);sum=sum+flag;
                SRC_CHECK(i,j,m  ,n+1,flag);sum=sum+flag;
                SRC_CHECK(i,j,m+1,n+1,flag);sum=sum+flag;
                
                dst->data[0][j][i] = (sum>0)?255:0;
            }
            
            count += (dst->data[0][j][i] == 255);
        }
    }
    
    return count;
}




#define SIMILAR_CHECK(x1,y1,x2,y2,Flag) {\
    int M=(x2)/x_step;int N=(y2)/y_step;\
    Flag = 0;\
    if((M>=0)&&(N>=0)&&(M<32)&&(N<32))\
    {\
        if(tab[N][M]==1)\
        {\
            Flag = 1;\
            for(cn=0;cn<channel;cn++)\
            {\
                if(ABS(data[cn][y1][x1] - data[cn][y2][x2])>thresh)\
                    {Flag = 0;break;}\
            }\
        }\
    }\
}

#define DIFFERENT_CHECK(x1,y1,x2,y2,Flag) {\
    int M=(x2)/x_step;int N=(y2)/y_step;\
    Flag = 0;\
    if((M>=0)&&(N>=0)&&(M<32)&&(N<32))\
    {\
        if(tab[N][M]==1)\
        {\
            for(cn=0;cn<channel;cn++)\
            {\
                if(ABS(data[cn][y1][x1] - data[cn][y2][x2])>thresh)\
                    {Flag = 1;break;}\
            }\
        }\
    }\
}



int mImageSeperate0(MImage *src,MImage *dst,MImagePoint *point,int thresh)
{
    int i,j,cn;
    int flag,valid;
    
    int height = src->height;
    int width = src->width;
    int channel=src->channel;
    
    unsigned char ***data = src->data;
    
    int tab[32][32];
    memset(tab,0,1024*sizeof(int));
    
    int x_step = (width-1)/31;
    int y_step = (height-1)/31;
    
    int x0 = (int)(point->x);//x_step;
    int y0 = (int)(point->y);
    
    int m = x0/x_step; int n = y0/y_step;tab[n][m] = 1;
    int x,y;
    
    valid = 0;
    
    x=(m+1)*x_step;y=n*y_step;SIMILAR_CHECK(x,y,x0,y0,flag);if(flag==1){tab[n  ][m+1]=1;valid=1;}
    y=(n+1)*y_step;           SIMILAR_CHECK(x,y,x0,y0,flag);if(flag==1){tab[n+1][m+1]=1;valid=1;}
    x=m*x_step;               SIMILAR_CHECK(x,y,x0,y0,flag);if(flag==1){tab[n+1][m  ]=1;valid=1;}
    y = n*y_step;             SIMILAR_CHECK(x,y,x0,y0,flag);if(flag==1){tab[n  ][m  ]=1;valid=1;}else tab[n][m]=0;
    
    int sum;
    printf("tab c is %d,%d,%d,%d\n",tab[n  ][m  ],tab[n  ][m+1],tab[n+1][m+1],tab[n+1][m  ]);
    printf("m is %d,n is %d\n",m,n);
    
    for(int time=0;time<5;time++)
    {
        // #define VALID_LOCATE(N,M) (((M<0)||(N<0)||(M>31)||(N>31))?0:tab[N][M])
        
        if(valid == 0) break;valid = 0;
        for(j=0,n=0;n<32;j+=y_step,n++)
            for(i=0,m=0;m<32;i+=x_step,m++)
            {
                if(tab[n][m] == 1) continue;
                
                sum = 0;
                DIFFERENT_CHECK(i,j,i+x_step,j+y_step,flag);sum+=flag;
                DIFFERENT_CHECK(i,j,i+x_step,j       ,flag);sum+=flag;
                
                // if((m==17)&&(n==15))
                // {
                    // printf("tab[%d][%d] is %d\n",n,m,tab[n][m]);
                    // printf("sum is %d\n",sum);
                    // return 0;
                // }
                
                DIFFERENT_CHECK(i,j,i+x_step,j-y_step,flag);sum+=flag;if(sum >= 2) continue;
                
                DIFFERENT_CHECK(i,j,i       ,j+y_step,flag);sum+=flag;             
                DIFFERENT_CHECK(i,j,i       ,j-y_step,flag);sum+=flag;if(sum >= 2) continue;
                DIFFERENT_CHECK(i,j,i-x_step,j+y_step,flag);sum+=flag;             
                DIFFERENT_CHECK(i,j,i-x_step,j       ,flag);sum+=flag;             
                DIFFERENT_CHECK(i,j,i-x_step,j-y_step,flag);sum+=flag;if(sum >= 2) continue;
                sum = 0;
                SIMILAR_CHECK(i,j,i+x_step,j+y_step,flag);sum+=flag;
                SIMILAR_CHECK(i,j,i+x_step,j       ,flag);sum+=flag;
                SIMILAR_CHECK(i,j,i+x_step,j-y_step,flag);sum+=flag;if(sum >= 2){tab[n][m] = 1;valid = 1;continue;}
                SIMILAR_CHECK(i,j,i       ,j+y_step,flag);sum+=flag;
                SIMILAR_CHECK(i,j,i       ,j-y_step,flag);sum+=flag;if(sum >= 2){tab[n][m] = 1;valid = 1;continue;}
                SIMILAR_CHECK(i,j,i-x_step,j+y_step,flag);sum+=flag;
                SIMILAR_CHECK(i,j,i-x_step,j       ,flag);sum+=flag;
                SIMILAR_CHECK(i,j,i-x_step,j-y_step,flag);sum+=flag;if(sum >= 2){tab[n][m] = 1;valid = 1;continue;}
            }
        
        if(valid == 0) break;valid = 0;
        for(j=31*y_step,n=31;j>=0;j-=y_step,n--)
            for(i=31*x_step,m=31;i>=0;i-=x_step,m--)
            {
                if(tab[n][m] == 1) continue;
                sum = 0;
                DIFFERENT_CHECK(i,j,i-x_step,j-y_step,flag);sum+=flag;
                DIFFERENT_CHECK(i,j,i-x_step,j       ,flag);sum+=flag;
                DIFFERENT_CHECK(i,j,i-x_step,j+y_step,flag);sum+=flag;if(sum >= 2) continue;
                DIFFERENT_CHECK(i,j,i       ,j-y_step,flag);sum+=flag;             
                DIFFERENT_CHECK(i,j,i       ,j+y_step,flag);sum+=flag;if(sum >= 2) continue;
                DIFFERENT_CHECK(i,j,i+x_step,j-y_step,flag);sum+=flag;             
                DIFFERENT_CHECK(i,j,i+x_step,j       ,flag);sum+=flag;             
                DIFFERENT_CHECK(i,j,i+x_step,j+y_step,flag);sum+=flag;if(sum >= 2) continue;
                sum = 0;
                SIMILAR_CHECK(i,j,i-x_step,j-y_step,flag);sum+=flag;
                SIMILAR_CHECK(i,j,i-x_step,j       ,flag);sum+=flag;
                SIMILAR_CHECK(i,j,i-x_step,j+y_step,flag);sum+=flag;if(sum >= 2){tab[n][m] = 1;valid = 1;continue;}
                SIMILAR_CHECK(i,j,i       ,j-y_step,flag);sum+=flag;
                SIMILAR_CHECK(i,j,i       ,j+y_step,flag);sum+=flag;if(sum >= 2){tab[n][m] = 1;valid = 1;continue;}
                SIMILAR_CHECK(i,j,i+x_step,j-y_step,flag);sum+=flag;
                SIMILAR_CHECK(i,j,i+x_step,j       ,flag);sum+=flag;
                SIMILAR_CHECK(i,j,i+x_step,j+y_step,flag);sum+=flag;if(sum >= 2){tab[n][m] = 1;valid = 1;continue;}
            }
    }
    
    int count;
    for(j=0;j<height;j++)
    {
        n = j/y_step;if(n>31) n=31;
        for(i=0;i<width;i++)
        {
            m = i/x_step;if(m>31) m=31;
            
            int sum = VALID_LOCATE(n,m)+VALID_LOCATE(n,m+1)+VALID_LOCATE(n+1,m)+VALID_LOCATE(n+1,m+1);
            if(sum == 4)
                dst->data[0][j][i] = 255;
            else if(sum == 0)
                dst->data[0][j][i] = 0;
            else
            {
                x = m*x_step; y = n*y_step;
                // if((i==1055)&&(j==1000))
                // {
                    // printf("(%d,%d)%d,(%d,%d)%d,(%d,%d)%d,(%d,%d)%d\n",
                    // x,y,VALID_LOCATE(n,m),
                    // x+x_step,y,VALID_LOCATE(n,m+1),
                    // x,y+y_step,VALID_LOCATE(n+1,m),
                    // x+x_step,y+y_step,VALID_LOCATE(n+1,m+1));
                // }
                
                
                SIMILAR_CHECK(i,j,x,y,flag);
                if(flag == 1) {dst->data[0][j][i] = 255;continue;}
                
                SIMILAR_CHECK(i,j,x+x_step,y,flag);
                if(flag == 1) {dst->data[0][j][i] = 255;continue;}
            
                SIMILAR_CHECK(i,j,x,y+y_step,flag);
                if(flag == 1) {dst->data[0][j][i] = 255;continue;}
            
                SIMILAR_CHECK(i,j,x+x_step,y+y_step,flag);
                if(flag == 1) {dst->data[0][j][i] = 255;continue;}
                
                dst->data[0][j][i] = 0;
            }
            
            count += (dst->data[0][j][i] == 255);
        }
    }
    
    return count;
}
*/
