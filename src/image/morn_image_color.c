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

#define CHANNEL(func) {for(cn=0;cn<channel;cn++) {func;}}

void mImageColorInterest(MImage *src,MImage *dst,unsigned char **color,int color_num,int range)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) dst=src;
    if(dst!=src) mImageRedefine(dst,1,src->height,src->width,dst->data);
    
    float k = (float)range*1.201122; k = k*k;
    // printf("range is %d,k is %f\n",range,k);
    int i,j,cn;
    unsigned char rst[256];
    for(i=0;i<256;i++)
    {
        rst[i] = (unsigned char)(exp(0.0-((double)(i*i))/((double)k))*255.0+0.5);
        // printf("%d:%d\n",i,rst[i]);
        if(rst[i] == 0) break;
    }
    int thresh = i*i;
    
    // printf("color is %d,%d,%d\n",color[0][0],color[0][1],color[0][2]);
    for(j=0;j<src->height;j++)
        for(i=0;i<src->width;i++)
        {
            int diff_min = 255*255;
            for(int n=0;n<color_num;n++)
            {
                register int d = src->data[0][j][i]-color[n][0];
                register int diff = d*d;
                for(cn=1;cn<MIN(src->channel,3);cn++)
                    {d = src->data[cn][j][i]-color[n][cn];diff += d*d;}
                diff_min = MIN(diff,diff_min);
            }
            if(diff_min>thresh) dst->data[0][j][i] = 0;
            else
            {
                diff_min = (int)sqrt((double)diff_min);
                dst->data[0][j][i] = rst[diff_min];
            }
        }
    
    dst->channel = 1;
}

/*
void mColorReplace(MImage *src,MImage *dst,unsigned char *color_in,unsigned char *color_out
{
        #define ColorReplace(X,Y) 
        {
            int diff_max = 0;\
            int db,dg,dr;
            db=b[X  ][Y  ]-ib;dg=g[X  ][Y  ]-ig;dr=r[X  ][Y  ]-ir;int diff  =MAX(db,MAX(dg,dr));\
            if(diff<10) {dst->data[0][Y][X]=b[Y][X]-ib+ob;dst->data[1][Y][X]=g[Y][X]-ig+og;dst->data[2][Y][X]=r[Y][X]-ir+or;continue;}
            
            db=b[X-4][Y-4]-ib;dg=g[X-4][Y-4]-ig;dr=r[X-4][Y-4]-ir;int diff1 =MAX(db,MAX(dg,dr));\
            db=b[X-4][Y-2]-ib;dg=g[X-4][Y-2]-ig;dr=r[X-4][Y-2]-ir;int diff2 =MAX(db,MAX(dg,dr));\
            db=b[X-4][Y  ]-ib;dg=g[X-4][Y  ]-ig;dr=r[X-4][Y  ]-ir;int diff3 =MAX(db,MAX(dg,dr));\
            db=b[X-4][Y+2]-ib;dg=g[X-4][Y+2]-ig;dr=r[X-4][Y+2]-ir;int diff4 =MAX(db,MAX(dg,dr));\
            db=b[X-4][Y+4]-ib;dg=g[X-4][Y+4]-ig;dr=r[X-4][Y+4]-ir;int diff5 =MAX(db,MAX(dg,dr));\
            db=b[X-2][Y+4]-ib;dg=g[X-2][Y+4]-ig;dr=r[X-2][Y+4]-ir;int diff6 =MAX(db,MAX(dg,dr));\
            db=b[X  ][Y+4]-ib;dg=g[X  ][Y+4]-ig;dr=r[X  ][Y+4]-ir;int diff7 =MAX(db,MAX(dg,dr));\
            db=b[X+2][Y+4]-ib;dg=g[X+2][Y+4]-ig;dr=r[X+2][Y+4]-ir;int diff8 =MAX(db,MAX(dg,dr));\
            db=b[X+4][Y+4]-ib;dg=g[X+4][Y+4]-ig;dr=r[X+4][Y+4]-ir;int diff9 =MAX(db,MAX(dg,dr));\
            db=b[X+4][Y+2]-ib;dg=g[X+4][Y+2]-ig;dr=r[X+4][Y+2]-ir;int diff10=MAX(db,MAX(dg,dr));\
            db=b[X+4][Y  ]-ib;dg=g[X+4][Y  ]-ig;dr=r[X+4][Y  ]-ir;int diff11=MAX(db,MAX(dg,dr));\
            db=b[X+4][Y-2]-ib;dg=g[X+4][Y-2]-ig;dr=r[X+4][Y-2]-ir;int diff12=MAX(db,MAX(dg,dr));\
            db=b[X+4][Y-4]-ib;dg=g[X+4][Y-4]-ig;dr=r[X+4][Y-4]-ir;int diff13=MAX(db,MAX(dg,dr));\
            db=b[X+2][Y-4]-ib;dg=g[X+2][Y-4]-ig;dr=r[X+2][Y-4]-ir;int diff14=MAX(db,MAX(dg,dr));\
            db=b[X  ][Y-4]-ib;dg=g[X  ][Y-4]-ig;dr=r[X  ][Y-4]-ir;int diff15=MAX(db,MAX(dg,dr));\
            db=b[X-2][Y-4]-ib;dg=g[X-2][Y-4]-ig;dr=r[X-2][Y-4]-ir;int diff16=MAX(db,MAX(dg,dr));\
            
            if(diff1 <10) if(diff9 >diff_max) {diff_max=diff9 ;x=-4;y=-4;}
            if(diff2 <10) if(diff10>diff_max) {diff_max=diff10;x=-4;y=-4;}
            if(diff3 <10) if(diff11>diff_max) {diff_max=diff11;x=-4;y=-4;}
            if(diff4 <10) if(diff12>diff_max) {diff_max=diff12;x=-4;y=-4;}
            if(diff5 <10) if(diff13>diff_max) {diff_max=diff13;x=-4;y=-4;}
            if(diff6 <10) if(diff14>diff_max) {diff_max=diff14;x=-4;y=-4;}
            if(diff7 <10) if(diff15>diff_max) {diff_max=diff15;x=-4;y=-4;}
            if(diff8 <10) if(diff16>diff_max) {diff_max=diff16;x=-4;y=-4;}
            if(diff9 <10) if(diff1 >diff_max) {diff_max=diff1 ;x=-4;y=-4;}
            if(diff10<10) if(diff2 >diff_max) {diff_max=diff2 ;x=-4;y=-4;}
            if(diff11<10) if(diff3 >diff_max) {diff_max=diff3 ;x=-4;y=-4;}
            if(diff12<10) if(diff4 >diff_max) {diff_max=diff4 ;x=-4;y=-4;}
            if(diff13<10) if(diff5 >diff_max) {diff_max=diff5 ;x=-4;y=-4;}
            if(diff14<10) if(diff6 >diff_max) {diff_max=diff6 ;x=-4;y=-4;}
            if(diff15<10) if(diff7 >diff_max) {diff_max=diff7 ;x=-4;y=-4;}
            if(diff16<10) if(diff8 >diff_max) {diff_max=diff8 ;x=-4;y=-4;}
            
            if(diff_max == 0) {dst->data[0][Y][X]=b[Y][X];dst->data[1][Y][X]=g[Y][X];dst->data[2][Y][X]=r[Y][X];}
            else
            {
                int b0=b[Y-y][X+x];int g0=g[Y-y][X+x];int r0=r[Y-y][X+x];
                float db0=b0     -ib;float dg0=g0     -ig;float dr0=r0     -ir;
                float db =b[Y][X]-ib;float dg =g[Y][X]-ig;float dr =r[Y][X]-ir;
                float k = (db*db0+dg*dg0+dr*dr0)/(db0*db0+dg0*dg0+dr0*dr0); k=1.0-k;
                
                b0 = b[Y][X] - k*(ib - ob);if(b0<0) b0=0;else if(b0>255) b0=255; dst->data[0][Y][X] = b0;
                g0 = g[Y][X] - k*(ig - og);if(g0<0) g0=0;else if(g0>255) g0=255; dst->data[1][Y][X] = g0;
                r0 = b[Y][X] - k*(ir - or);if(r0<0) r0=0;else if(r0>255) r0=255; dst->data[2][Y][X] = r0;
            }
        }
}
*/

void mColorReplace0(MImage *src,MImage *dst,unsigned char *color_in,unsigned char *color_out,int range)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    
    if(INVALID_POINTER(dst)) dst=src;
    if(dst!=src) mImageRedefine(dst,src->channel,src->height,src->width,dst->data);
    
    float k = (float)range*1.201122; k = k*k;
    // printf("range is %d,k is %f\n",range,k);
   
    int i,j,cn;
    
    float rst[256];
    for(i=0;i<256;i++)
    {
        rst[i] = (exp(0.0-((double)(i*i))/(double)k));
        printf("%d:%f\n",i,rst[i]);
    }
    
    for(j=0;j<src->height;j++)
        for(i=0;i<src->width;i++)
        {
            // int diff = ABS(src->data[2][j][i]-color_in[2]);
            int diff = ABS(src->data[0][j][i]-color_in[0]);
            for(cn=1;cn<src->channel;cn++)
                diff = MAX(diff,ABS(src->data[cn][j][i]-color_in[cn]));
            
            
            for(cn=0;cn<src->channel;cn++)
            {
                float value = src->data[cn][j][i] + rst[diff]*(color_out[cn]-src->data[cn][j][i]);
                     if(value>255.0f) dst->data[cn][j][i] = 255;
                else if(value<0.0f  ) dst->data[cn][j][i] = 0;
                else                  dst->data[cn][j][i] = (unsigned char)(value+0.5);
            }
            
            // if((i==920)&&(j==1354))
            // {
                // printf("src is %d,%d,%d\n",
                    // src->data[0][j][i],src->data[1][j][i],src->data[2][j][i]);
                // printf("diff is %d,rst[diff] is %d\n",diff,rst[diff]);
                // printf("dst is %d,%d,%d\n",
                    // dst->data[0][j][i],dst->data[1][j][i],dst->data[2][j][i]);
            // }
            
        }
}


void mColorConvert(MImage *src,MImage *dst,unsigned char **color_in,unsigned char **color_out)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    
    int height= src->height;
    int width = src->width;
    int channel = src->channel;
    
    int i,j,m,n;
    
    float k[4][5];
    
    MMatrix *mat = mMatrixCreate(channel+1,channel+2,NULL,DFLT);
    float **data = mat->data;
    for(n=0;n<channel+1;n++)
    {
        for(m=0;m<channel;m++)
            data[n][m] = (float)color_in[n][m];
        data[n][channel]=1.0f;
    }
    
    for(m=0;m<channel;m++)
    {
        for(n=0;n<channel+1;n++)
            data[n][channel+1]=0.0-(float)color_out[n][m];
        mLinearEquation(mat,(float *)k+m*5);
    }
    
    mMatrixRelease(mat);
    
    for(j=0;j<height;j++)
        for(i=0;i<width;i++)
        {
            for(m=0;m<channel;m++)
            {
                float value = k[m][channel];
                for(n=0;n<channel+1;n++)
                    value += k[m][n]*(float)(src->data[n][j][i]);
                
                     if(value>255.0f) dst->data[m][j][i] = 255;
                else if(value<0.0f  ) dst->data[m][j][i] = 0;
                else                  dst->data[m][j][i] = (unsigned char)(value+0.5);
            }
        }
}




/*
struct ColorMap
{
    float vector0[3];
    
    float color1[3];
    float vector1[3];
    
    float w1_para[3];
    float w2_para[3];
    float w3_para[3];
};

void ReplaceMap(unsigned char *in,unsigned char *out,int range,struct ColorMap *map);

void mColorReplace(MImage *src,MImage *dst,unsigned char *color_in,unsigned char *color_out,int range)
{
    int height = src->height;
    int width = src->width;
    
    struct ColorMap[4096];
    ReplaceMap(color_in,color_out,range,map);
    
    #define COLOR_DIFF(Lx,Ly) MAX(MAX(ABS(src->data[0][Ly][Ly]-color_in[0]),\
                                      ABS(src->data[1][Ly][Ly]-color_in[1])),\
                                      ABS(src->data[2][Ly][Ly]-color_in[2]))
    // #define MeanFilter(X,Y) {\
        // int Sum = 0;\
        // for(int M=Y-r;M<=Y+r;M++)\
            // for(int N=X-r;N<=X+r;N++)\
            // {\
                // Sum += sdata[M][N];\
            // }\
        // ddata[Y][X] = Sum/area;\
    // }
    
    
    for(j=0;j<height;j++)
        for(i=0;i<width;i++)
        {
            unsigned char color1[3];
            unsigned char color2[3];
            
            int diff_min = 255;
            int diff_max = 0;
            
            for(m=MAX(0,j-3);m<=MIN(height,j+3);m++)
                for(n=MAX(0,i-3);n<=MIN(width,i+3;n++)
                {
                    int diff = COLOR_DIFF(n,m);
                    if(diff<diff_min) 
                    {
                        diff_min = diff;
                        color1[0]= src->data[0][m][n];
                        color1[1]= src->data[1][m][n];
                        color1[2]= src->data[2][m][n];
                    }
                    if(diff>diff_max)
                    {
                        diff_max = diff;
                        color2[0]= src->data[0][m][n];
                        color2[1]= src->data[1][m][n];
                        color2[2]= src->data[2][m][n];
                    }
                }
                    
            if(diff_min > range)
            {
                dst->data[0][j][i] = src->data[0][j][i];
                dst->data[1][j][i] = src->data[1][j][i];
                dst->data[2][j][i] = src->data[2][j][i];
                continue;
            }
            
            float c1[3];
            c1[0] = (float)color_in[0];
            c1[1] = (float)color_in[1];
            c1[2] = (float)color_in[2];
            
            float c2[3]
            c2[0] = (float)color2[0];
            c2[1] = (float)color2[1];
            c2[2] = (float)color2[2];
            
            float c3[3];
            c3[0] = (c1[2]*c2[1]-c1[1]*c2[2])/(c1[0]*c2[1]-c1[1]*c2[0]);
            c3[0] = (c1[2]*c2[1]-c1[1]*c2[2])/(c1[0]*c2[1]-c1[1]*c2[0]);
                
            c3[2] = 1.0f;
                
            float w1 = 
                        
                        
        }
        
}
*/



/*
void ColorMap(unsigned char *in,unsigned char *out,float *w,char *map)//MTable *map)
{
    float k[9];
    
    MMatrix *mat = mMatrixCreate(3,4,NULL);
    float **data = mat->data;
    data[0][0] = (float) in[0];data[0][1] = (float) in[1];data[0][2] = (float) in[2];
    data[1][0] = (float)out[0];data[1][1] = (float)out[1];data[1][2] = (float)out[2];
    
    data[2][0] = (float)in[0] + (float)(in[2]*out[1]-in[1]*out[2])/(float)(in[0]*out[1]-in[1]*out[0]);
    data[2][1] = (float)in[1] + (float)(in[2]*out[0]-in[0]*out[2])/(float)(in[1]*out[0]-in[0]*out[1]);
    data[2][2] = (float)in[2] -1.0;
    
    data[0][3] = 0-(float)out[0];
    data[1][3] = 0-(float)out[0];
    data[2][3] = data[2][0];
    mLinearEquation(mat,k);
    
    data[0][3] = 0-(float)out[1];
    data[1][3] = 0-(float)out[1];
    data[2][3] = data[2][1];
    mLinearEquation(mat,k+3);
    
    data[0][3] = 0-(float)out[2];
    data[1][3] = 0-(float)out[2];
    data[2][3] = data[2][2];
    mLinearEquation(mat,k+6);
    
    printf("k1 is %f,%f,%f\n",k[0],k[1],k[2]);
    printf("k2 is %f,%f,%f\n",k[3],k[4],k[5]);
    printf("k3 is %f,%f,%f\n",k[6],k[7],k[8]);
        
    float b0 = (float)in[0];
    float g0 = (float)in[1];
    float r0 = (float)in[2];
    float l0 = sqrt(r0*r0+g0*g0+b0*b0);
    
    int r,g,b;
    for(r=0;r<256;r+=16)
        for(g=0;g<256;g+=16)
            for(b=0;b<256;b+=16)
            {
                int diff = 256*((r*r0+g*g0+b*b0)/(l0*sqrt(r*r+g*g+b*b)));
                
                map[r*16+g+b/16] = w[diff];

            }
            
            
}

struct HandleImageColorReplace
{
    unsigned char ci[4];
    unsigned char co[4];
    MMatrix *k;
    unsigned char w[256];
};
void endImageColorReplace(void *info)
{
    struct HandleImageColorReplace *handle = info;
    if(handle->k!=NULL)
        mMatrixRelease(handle->k);
}


void mImageColorReplace(MImage *src,MImage *dst,unsigned char *color_in,unsigned char *color_out,int range)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    
    MHandle *hdl; ObjectHandle(src,ColorCluster,hdl);
    struct HandleColorCluster *handle = hdl->handle;
    if(memcmp(color_in,handle->ci,channel*sizeof(unsigned char))!=0)
        hdl->valid = 0;
    else if(memcmp(color_out,handle->co,channel*sizeof(unsigned char))!=0)
        hdl->valid = 0;
    if(hdl->valid == 0)
    {
        memcpy(handle->ci,color_in ,channel*sizeof(unsigned char));
        memcpy(handle->co,color_out,channel*sizeof(unsigned char));
        
        
        
    }
}
*/


/*
void mImageColorCluster(MImage *src,MSheet *color,int distance,int thresh)
{
    mException(INVALID_IMAGE(src)||INVALID_POINTER(color),EXIT,"invalid input");
    
    int i,j,k,n,cn;
    int flag;
    
    int channel = src->channel;
    
    unsigned char ***data = src->data;
    
    MImagePoint point;
    
    int s[512];
    int e[512];
    
    int l[512];
    int r[512];
    int d[512];
    
    int mean[512][4];
    int sum[512][4];
    int num[512];
    int valid[512];
    
    
    
    color->row=0;
    for(j=ImageY1(src);j<5;j++)//ImageY2(src);j++)
    {
        printf("j is %d\n",j);
        for(i=ImageX1(src,j);i<ImageX2(src,j);i++)
        {
            // printf("i is %d\n",i);
            flag = 1;CHANNEL(flag=flag&&(data[cn][j][i]==0));
            if(flag == 1) continue;
            
            
            printf("(%d,%d):data is %d\n",i,j,data[0][j][i]);
            
            int ok = 0;
            point.x = i;point.y = j;
            printf("color->row is %d\n",color->row);
            for(k=0;k<color->row;k++)
            {
                if(color->col[k] == num[k])
                {
                    CHANNEL(mean[k][cn] = sum[k][cn]/num[k]);
                    l[k] = s[k];r[k] = e[k];
                }
                
                if(j==3)
                    printf("l is %d,r is %d,d is %d\n",l[k],r[k],d[k]);
                if(((i<l[k]-distance)&&(i<s[k]-distance))
                 ||((i>r[k]+distance)&&(i>e[k]+distance))||(j>d[k]+distance))
                    continue;
                
                printf("k is %d,s[k] is %d,e[k] is %d,mean[k] is %d,sum[k] is %d,num[k] is %d\n",
                        k,s[k],e[k],mean[k][0],sum[k][0],num[k]);
                    
                flag = 1;CHANNEL(flag=flag&&(ABS(data[cn][j][i]-mean[k][cn])<thresh));
                
                if(flag == 1)
                {
                    ok = 1;
                    
                    s[k] = MIN(i,s[k]);
                    e[k] = MAX(i,e[k]);
                    d[k] = j;
                
                    mSheetWrite(color,k,DFLT,&point,sizeof(MImagePoint));
                    CHANNEL(sum[k][cn] += data[cn][j][i]);
                    num[k] = num[k]+1;
                }
            }
            
            // printf("ok is %d\n",ok);
            
            if(ok==0)
            {
                printf("aaa k is %d\n",k);
                mSheetWrite(color,k,DFLT,&point,sizeof(MImagePoint));
                
                CHANNEL(mean[k][cn] = data[cn][j][i];sum[k][cn] = data[cn][j][i];);
                printf("aaa mean[%d] is %d,sum is %d\n",k,mean[k][0],sum[k][0]);
                num[k] = 1;
                s[k] = i;
                e[k] = i;
                d[k] = j;
            }
            
            // printf("ok is %d\n",ok);
        }
        
        printf("aaaaaaaaa color->row is %d\n",color->row);
        // EXIT;
        
        for(k=0;k<color->row;k++)
        {
            printf("num[k] is %d,color->col[k] is %d\n",num[k],color->col[k]);
            if(num[k] == 0) continue;
            
            CHANNEL(mean[k][cn] = (mean[k][cn]*(color->col[k]-num[k]) + sum[k][cn])/color->col[k];
                    sum[k][cn] = 0;);
            num[k] = 0;
            
            printf("bbb mean[%d] is %d,sum is %d,num is %d\n",k,mean[k][0],sum[k][0],num[k]);
            
            l[k] = s[k];s[k] = src->width;
            r[k] = e[k];e[k] = 0;
            d[k] = j;
            
            
            for(n=0;n<k;n++)
            {
                if(valid[n] == 0) continue;
                
                flag = (d[k] == d[n]);
                if(flag == 0) continue;
                
                flag = (((l[k]>=l[n])&&(l[k]<=r[n]))||((r[k]>=l[n])&&(r[k]<=r[n])));
                if(flag == 0) continue;
                
                CHANNEL(flag=flag&&(ABS(mean[n][cn] - mean[k][cn])<thresh/8));
                if(flag == 0) continue;
               
                valid[k] = 0;
                CHANNEL(mean[n][cn] = (mean[k][cn]*color->col[k] + mean[k][cn]*color->col[n])/(color->col[k]+color->col[n]);
                        mean[k][cn] = 0;);
                
                l[n] = MIN(l[n],l[k]);
                r[n] = MAX(r[n],r[k]);
                d[n] = MAX(d[n],d[k]);
                
                void **data = mMalloc((color->col[n]+color->col[n])*sizeof(void *));
                memcpy(data              ,color->data[n],color->col[n]*sizeof(void *));
                memcpy(data+color->col[n],color->data[k],color->col[k]*sizeof(void *));
                color->col[n] = color->col[n] + color->col[k];
                color->col[k] = 0;
                mFree(color->data[n]);color->data[n] = data;
                mFree(color->data[k]);color->data[k] = NULL;
                
                break;
            }
        }
        printf("aaaaaaaaaaaaaaa color->row is %d\n",color->row);
    }
    
        n = 0;
        for(k=0;k<color->row;k++)
        {
            if(num[k] == 0) continue;
        
            if(k!=n)
            {
                color->data[n]= color->data[k];
                color->col[n] = color->col[k];
            }
            
            num[k] = 0;
            n=n+1;
        }
        printf("aaaaaaaaaaaaaaa color->row is %d\n",color->row);
        for(k=n;k<color->row;k++)
        {
            color->data[k]= NULL;
            color->col[k] = 0;
        }
        
        color->row = n;
    }
    
}


void mImageColorCluster1(MImage *src,MSheet *color,int distance,int thresh)
{
    mException(INVALID_IMAGE(src)||INVALID_POINTER(color),EXIT,"invalid input");
    
    int i,j,k,n,cn;
    int flag;
    
    int channel = src->channel;
    
    unsigned char ***data = src->data;
    
    MImagePoint point;
    
    int l[512];
    int r[512];
    int d[512];
    
    int mean[512][4];
    int sum[512][4];
    int num[512];
    int valid[512];
    
    MImagePoint **p = mMalloc(src->width*sizeof(MImagePoint *));
    char *valid = mMalloc(src->width*sizeof(char));
    
    color->row=0;
    
    for(j=0;j<5;j++)//ImageY2(src);j++)
    {
        printf("j is %d\n",j);
        point.y = j;
        for(i=0;i<src->width;i++)
        {
            flag = 1;CHANNEL(flag=flag&&(data[cn][j][i]==0));
            if(flag == 1) {valid[i] = -1; continue;}
            
            point.x = i;
            p[i] = mMemoryWrite(handle->memory,&point,sizeof(MImagePoint));
            valid[i] = 0;
        }
        
        int row = 0;
        for(k=row;k<color->row;k++)
        {
            if(j-d[k]>distance) continue;
            
            int c = (l[k] + r[k])/2;
            int r = (r[k] - l[k])/2;
            int col = color->col[row];
            for(i=c;i<src->width;i++)
            {
                if((i-r[k]>distance)&&(i-e[k]>distance)) break;
                if(valid[i]<0) continue;
                
                flag = 1;CHANNEL(flag=flag&&(ABS(data[cn][j][i]-mean[k][cn])<thresh));
                if(flag == 0) continue;
                
                valid[i] = 1;
                s[k] = MIN(i,s[k]);
                e[k] = MAX(i,e[k]);
                d[k] = j;
            
                mSheetColAppend(color,k,DFLT);
                color->data[k][col] = p[i];
                col=col+1;
                
                CHANNEL(sum[k][cn] += data[cn][j][i]);
                num[k] = num[k]+1;
            }
            
            for(i=c;i>=0;i--)
            {
                if((l[k]-i>distance)&&(s[k]-l>distance)) break;
                if(valid[i]<0) continue;
                
                flag = 1;CHANNEL(flag=flag&&(ABS(data[cn][j][i]-mean[k][cn])<thresh));
                if(flag == 0) continue;
                
                valid[i] = 1;
                s[k] = MIN(i,s[k]);
                e[k] = MAX(i,e[k]);
                d[k] = j;
            
                mSheetColAppend(color,k,DFLT);
                color->data[k][col] = p[i];
                col=col+1;
                
                CHANNEL(sum[k][cn] += data[cn][j][i]);
                num[k] = num[k]+1;
            }
        }
        
        row = color->row;
            
        n = color->row;
        for(i=0;i<src->width;i++)
        {
            if(valid[i] != 0) continue;
            
            mean[n][cn] = data[cn][j][i];
            sum[n][cn] += data[cn][j][i];
            num[n] = 1;
            
            s[n] = i;
            e[n] = i;
            
            for(k=0;k<
            
            
            
        }
            
            
            
            
        
            
        
        
        
        
        
        
        
        
    }
*/

unsigned int GetLocate(MTable *locate,int x,int y)
{
    unsigned int **l = locate->dataU32;
    
    int i = x;int j = y;
    
    while(1)
    {
        int n = l[y][x]&0x0FFFF;
        int m = l[y][x]>>16;
        // printf("l[y][x] is %d,m is %d,n is %d\n",l[y][x],m,n);
        
        if((m==y)&&(n==x))
        {
            l[j][i] = l[y][x];
            return l[y][x];
        }
        
        l[y][x]=l[m][n];
        y=m;x=n;
    }
}

// void UpdateLocate(MTable *locate)
// {
    // unsigned int **l = locate->dataU32;
    
    // for(int j=0;j<locate->row;j++)
        // for(int i=0;i<locate->col;i++)
        // {
            // unsigned int data = GetLocate(locate,i,j);
            // l[j][i] = data;
        // }
// }

struct HandleColorCluster
{
    MTable *locate;
    MTable *number;
    MTable *sum[4];
};
void endColorCluster(void *info)
{
    struct HandleColorCluster *handle = (struct HandleColorCluster *)info;
    if(handle->locate != NULL) mTableRelease(handle->locate);
    if(handle->number != NULL) mTableRelease(handle->number);
    if(handle->sum[0] != NULL) mTableRelease(handle->sum[0]);
    if(handle->sum[1] != NULL) mTableRelease(handle->sum[1]);
    if(handle->sum[2] != NULL) mTableRelease(handle->sum[2]);
    if(handle->sum[3] != NULL) mTableRelease(handle->sum[3]);
}
#define HASH_ColorCluster 0x148a9842

int mColorCluster(MImage *src,MImage *dst,MList *list,int r,int thresh)
{
    mException(INVALID_IMAGE(src),EXIT,"invalid input");
    
    int width = src->width;
    int height= src->height;
    int channel = src->channel;
    
    int i,j,cn;
    
    MHandle *hdl=mHandle(src,ColorCluster);
    struct HandleColorCluster *handle = (struct HandleColorCluster *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->locate == NULL) handle->locate = mTableCreate(height,width,U32,NULL);
        else                       mTableRedefine(handle->locate,height,width,U32,NULL);
        
        if(handle->number == NULL) handle->number = mTableCreate(height,width,S32,NULL);
        else                       mTableRedefine(handle->number,height,width,S32,NULL);
   
        for(cn=0;cn<channel;cn++)
        {
            if(handle->sum[cn] == NULL) handle->sum[cn] = mTableCreate(height,width,S32,NULL);
            else                        mTableRedefine(handle->sum[cn],height,width,S32,NULL);
        }
  
        hdl->valid = 1;
    }
    unsigned int **locate = handle->locate->dataU32;
    int **number = handle->number->dataS32;
    int **sum[4]; CHANNEL(sum[cn] = handle->sum[cn]->dataS32;)
    
    for(j=0;j<height;j++)for(i=0;i<width;i++)
    {
        locate[j][i] = (j<<16) + i;
        CHANNEL(sum[cn][j][i] = src->data[cn][j][i]);
        number[j][i] = 1;
    }
    
    for(int aa=0;aa<10;aa++)
    {
        int ok = 1;
        for(j=0;j<height;j++)for(i=0;i<width;i++)
        {
            unsigned int l0 = GetLocate(handle->locate,i,j);
            int y0 = l0 >> 16; int x0 = l0 & 0x0FFFF;
            
            int mean0[4];CHANNEL(mean0[cn] = sum[cn][y0][x0]/number[y0][x0]);
                            
            // if((i==830)&&(j==243))
                // printf("aa (%d,%d)->%d,%d\n",i,j,x0,y0);
            // printf("(%d,%d)->(%d,%d)\n",i,j,x0,y0);
            
            int min = 256; unsigned int ll=0;
            for(int n=MAX(j-r,0);n<=MIN(j+r,height-1);n++)for(int m=MAX(i-r,0);m<=MIN(i+r,width-1);m++)
            {
                unsigned int l = GetLocate(handle->locate,m,n);
                if(l==l0) continue;
                int y = l >> 16; int x = l & 0x0FFFF;
                // printf("aa (%d,%d)->(%d,%d)\n",m,n,x,y);
                
                int diff=0; CHANNEL(diff = MAX(diff,ABS(sum[cn][y][x]/number[y][x] - mean0[cn])));
                if(diff<min) {min = diff; ll = l;}
            }
                
            if(min < thresh)
            {
                locate[j ][i ] = ll; locate[y0][x0] = ll;
                int y = ll >> 16; int x = ll & 0x0FFFF;
                
                CHANNEL(sum[cn][y][x] += sum[cn][y0][x0]);
                number[y][x] +=  number[y0][x0];
                         
                ok = 0;
            }
        }
        printf("ok is %d\n",ok);
        if(ok) break;
    }
    
    if(dst==NULL) dst=src;
    else mImageRedefine(dst,channel,height,width,dst->data);
    
    int color_num = 0;
    for(j=0;j<height;j++)for(i=0;i<width;i++)
    {
        unsigned int l = GetLocate(handle->locate,i,j);
        int y = l >> 16;int x = l & 0x0FFFF;
        
        int mean[4]; CHANNEL(mean[cn] = sum[cn][y][x]/number[y][x];dst->data[cn][j][i] = mean[cn];);
        
        if((y==j)&&(x==i)) 
        {
            if(list!=NULL) mListWrite(list,color_num,mean,cn*sizeof(int));
            color_num+=1;
        }
        
        // if((i>1220)&&(i<1229)&&(j==147))
            // printf("dd (%d,%d)->%d(%d,%d) data is %d\n",i,j,locate[j][i],x,y,dst->data[0][j][i]);
    }
    return color_num;
}


