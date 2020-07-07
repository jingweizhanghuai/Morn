/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morn_math.h"

// #define ABS(Data) (((Data)>0)?(Data):(0-(Data)))
// #define MIN(A,B) (((A)<(B))?(A):(B))
// #define MAX(A,B) (((A)>(B))?(A):(B))
// typedef struct MLInt
// {
// 	uint32_t data[32];
// 	char sign;
// 	char len;
// }MLInt;

void mS64ToLInt(int64_t in,MLInt *a)
{
    memset(a,0,sizeof(MLInt));
    if(in>0) {*((int64_t *)a)=  in;a->sign=0;}
    else     {*((int64_t *)a)=0-in;a->sign=1;}
    a->len=(a->data[1]==0)?1:2;
}

int64_t mLIntToS64(MLInt *a)
{
    if(a->len>2) return 0x7fffffffffffffff;
    int64_t out = *((int64_t *)a);
    if(a->sign==1) out=0-out;
    return out;
}

void mLIntAdd(MLInt *a,MLInt *b,MLInt *c)
{
    MLInt buff;if(c==a) {buff=*a;a=&buff;} else if(c==b) {buff=*b;b=&buff;}
    memset(c,0,sizeof(MLInt));
    if((a->sign==0)&&(b->sign==1)) {b->sign=0;mLIntSub(a,b,c);b->sign=1;return;}
    if((a->sign==1)&&(b->sign==0)) {a->sign=0;mLIntSub(b,a,c);a->sign=1;return;}
    if(a->len<b->len) {mLIntAdd(b,a,c);return;}
    c->sign=a->sign;
    int i;
    uint64_t rst=0;
    for(i=0;i<b->len;i++)
    {
        rst = rst+a->data[i]+b->data[i];
        c->data[i]=rst&0x00000000ffffffff;
        rst = rst>>32;
    }
    for(;i<a->len;i++)
    {
        rst = rst+a->data[i];
        c->data[i]=rst&0x00000000ffffffff;
        rst = rst>>32;
        if(rst==0) break;
    }
    if(i==a->len) {if(rst==0) c->len=a->len; else {c->data[i]=rst;c->len=i+1;} return;}
    i=i+1;
    for(;i<a->len;i++) c->data[i]=a->data[i];
    c->len = a->len;
}

void mLIntAddU32(MLInt *a,uint32_t b,MLInt *c)
{
    MLInt buff;if(c==a) {buff=*a;a=&buff;}
    memset(c,0,sizeof(MLInt));
    if(a->sign==1) {a->sign=0;mLIntSubU32(a,b,c);a->sign=1;c->sign=!c->sign;return;}
    int i;
    uint64_t rst = a->data[0]+b;
    c->data[0]=rst&0x00000000ffffffff;
    rst = rst>>32;
    for(i=1;i<a->len;i++)
    {
        rst = rst+a->data[i];
        c->data[i]=rst&0x00000000ffffffff;
        rst = rst>>32;
        if(rst==0) break;
    }
    if(i==a->len) {if(rst==0) c->len=a->len; else {c->data[i]=rst;c->len=i+1;} return;}
    i=i+1;
    for(;i<a->len;i++) c->data[i]=a->data[i];
    c->len = a->len;
}
void mLIntAddS32(MLInt *a,int b,MLInt *c)
{
    if(b>=0) {mLIntAddU32(a,(uint32_t)b    ,c); return;}
    if(b< 0) {mLIntSubU32(a,(uint32_t)(0-b),c); return;}
}

int mLIntCompare(MLInt *a,MLInt *b)
{
    if((a->sign==0)&&(b->sign==1)) return  1;
    if((a->sign==1)&&(b->sign==0)) return -1;
    if(a->len>b->len) return ((a->sign)?-1:1);
    if(a->len<b->len) return ((a->sign)?1:-1);
    for(int i=a->len-1;i>=0;i--)
    {
        if(a->data[i]>b->data[i]) return ((a->sign)?-1:1);
        if(a->data[i]<b->data[i]) return ((a->sign)?1:-1);
    }
    return 0;
}

void mLIntSub(MLInt *a,MLInt *b,MLInt *c)
{
    MLInt buff;if(c==a) {buff=*a;a=&buff;} else if(c==b) {buff=*b;b=&buff;}
    memset(c,0,sizeof(MLInt));
    if((a->sign==0)&&(b->sign==1)) {b->sign=0;mLIntAdd(a,b,c);b->sign=1;return;}
    if((a->sign==1)&&(b->sign==0)) {b->sign=1;mLIntAdd(a,b,c);b->sign=0;return;}
    int flag = mLIntCompare(a,b);if(flag==0) return;
    if(flag<0) {mLIntSub(b,a,c);c->sign=!c->sign;return;}
    c->sign=a->sign;
    int i;uint64_t rst=1;
    for(i=0;i<b->len;i++)
    {
        rst = rst+0x0100000000+a->data[i]-1-b->data[i];
        c->data[i]=rst&0x00000000ffffffff;
        if(c->data[i]) c->len=i+1;
        rst = rst>>32;
    }
    for(;i<a->len;i++)
    {
        rst = rst+0x100000000+a->data[i]-1;
        c->data[i]=rst&0x00000000ffffffff;
        if(c->data[i]) c->len=i+1;
        rst = rst>>32;
        if(rst==1) break;
    }
    i=i+1;
    if(i>=a->len) return;
    for(;i<a->len;i++) c->data[i]=a->data[i];
    c->len=a->len;
}

void mLIntSubU32(MLInt *a,uint32_t b,MLInt *c)
{
    MLInt buff;if(c==a) {buff=*a;a=&buff;}
    memset(c,0,sizeof(MLInt));
    if(a->sign==1) {a->sign=0;mLIntAddU32(a,b,c);a->sign=1;c->sign=!c->sign;return;}
    if(a->len==1)
    {
        if(a->data[0]<b) {c->data[0]=b-a->data[0];c->len=1;c->sign=1;return;}
        if(a->data[0]>b) {c->data[0]=a->data[0]-b;c->len=1;c->sign=0;return;}
        c->len=0;c->sign=0;return;
    }
    c->sign=a->sign;
    
    uint64_t rst=0x0100000000+a->data[0]-b;
    c->data[0]=rst&0x00000000ffffffff;
    c->len=1;
    rst = rst>>32;
    int i=1;
    for(;i<a->len;i++)
    {
        rst = rst+0x100000000+a->data[i]-1;
        c->data[i]=rst&0x00000000ffffffff;
        if(c->data[i]) c->len=i+1;
        rst = rst>>32;
        if(rst==1) break;
    }
    i=i+1;
    if(i>=a->len) return;
    for(;i<a->len;i++) c->data[i]=a->data[i];
    c->len=a->len;
}

void mLIntSubS32(MLInt *a,int b,MLInt *c)
{
    if(b>=0) {mLIntSubU32(a,(uint32_t)b    ,c); return;}
    if(b< 0) {mLIntAddU32(a,(uint32_t)(0-b),c); return;}
}

void mLIntMulU32(MLInt *a,uint32_t b,MLInt *c)
{
    c->sign=a->sign;
    int i;
    uint64_t rst=0;
    for(i=0;i<a->len;i++)
    {
        rst=rst+(uint64_t)(a->data[i])*(uint64_t)b;
        c->data[i] = rst&0x00000000ffffffff;
        rst=rst>>32;
    }
    if(rst==0) c->len=a->len;
    else {c->data[i]=rst; c->len=a->len+1;}
}
void mLIntMulS32(MLInt *a,int b,MLInt *c)
{
    uint32_t buff = ABS(b);
    mLIntMulU32(a,buff,c);
    c->sign=(b>0)?a->sign:(!(a->sign));
}

void mLIntMul(MLInt *a,MLInt *b,MLInt *c)
{
    MLInt buff;if(c==a) {buff=*a;a=&buff;} else if(c==b) {buff=*b;b=&buff;}
    mLIntMulU32(a,b->data[0],c);
    MLInt d;
    for(int i=1;i<b->len;i++)
    {
        mLIntMulU32(a,b->data[i],&d);
        memmove(d.data+i,d.data,d.len*sizeof(uint32_t));
        memset(d.data,0,i*sizeof(uint32_t));
        d.len+=i;
        mLIntAdd(&d,c,c);
    }
    c->sign=(a->sign!=b->sign);
}

void mLIntDivU32(MLInt *a,uint32_t b,MLInt *c,int32_t *remainder)
{
    c->sign=a->sign;
    int i=a->len-1;
    uint64_t rst = a->data[i];
    c->data[i] = rst/b; rst=rst%b;
    c->len = (c->data[i]==0)?(a->len-1):a->len;
    for(i=i-1;i>=0;i--)
    {
        rst=(rst<<32)+a->data[i];
        c->data[i]=rst/b;
        rst=rst%b;
    }
    if(remainder!=NULL) *remainder=(a->sign)?(0-(int32_t)rst):(int32_t)rst;
}

void mLIntDivS32(MLInt *a,int b,MLInt *c,int *remainder)
{
    uint32_t buff = ABS(b);
    mLIntDivU32(a,buff,c,remainder);
    c->sign=(b>0)?a->sign:(!(a->sign));
}

void mLIntDiv(MLInt *a,MLInt *b,MLInt *c,MLInt *remainder)
{
    MLInt buff1;if(c        ==a) {buff1=*a;a=&buff1;} else if(c        ==b) {buff1=*b;b=&buff1;}
    MLInt buff2;if(remainder==a) {buff2=*a;a=&buff2;} else if(remainder==b) {buff2=*b;b=&buff2;}
    memset(c,0,sizeof(MLInt));
    MLInt d = *a;
    uint64_t a0,b0;
    while(mLIntCompare(a,b)>=0)
    {
        memset(remainder,0,sizeof(MLInt));
        int a_len = a->len;int b_len = b->len;
        b0 = (uint64_t)(b->data[b_len-1])+1;
        
        if(a_len>b_len)
        {
            a0 = (((uint64_t)(a->data[a_len-1]))<<32)+(uint64_t)(a->data[a_len-2]);
            remainder->len = a_len-b_len+1;
            a0=a0/b0;a0=MAX(a0,1);
            remainder->data[remainder->len-1]=(uint32_t)(a0>>32);
            remainder->data[remainder->len-2]=(uint32_t)(a0&0x00000000ffffffff);
            if(remainder->data[remainder->len-1]==0) remainder->len-=1;
        }
        else //(a_len==b_len)
        {
            a0 = (uint64_t)(a->data[a_len-1]);
            remainder->len = 1;
            a0=a0/b0;a0=MAX(a0,1);
            remainder->data[0]=(uint32_t)a0;
        }
        mLIntAdd(c,remainder,c);
        mLIntMul(b,remainder,remainder);
        mLIntSub(a,remainder,a);
    }
    *remainder = *a;
    *a=d;
    c->sign = (a->sign==b->sign)?0:1;
    remainder->sign = a->sign;
}

void mLIntToString(MLInt *a,char *str)
{
    if(a->len ==0) {str[0]='0';str[1]=0;return;}
    if(a->sign) {str[0]='-';str++;}
    MLInt buff = *a;buff.sign=0;int rst[33];int i;
    for(i=32;i>=0;i--)
    {
        mLIntDivU32(&buff,1000000000,&buff,rst+i);
        if(buff.len==0) {i=i-1;break;}
    }
    i=i+1;sprintf(str,"%d",rst[i]);str+=strlen(str);
    for(i=i+1;i<33;i++) {sprintf(str,"%09d",rst[i]);str+=9;}
    str[0]=0;
}

void mStringToLInt(char *str,MLInt *a)
{
    int l = strlen(str);
    memset(a,0,sizeof(MLInt));
    int sign=0;
    if(str[0]=='-') {str++;l--;sign=1;}
    uint64_t b;MLInt c;int i=0;
    char buff[10];buff[9]=0;
    for(i=0;i<l-18;i+=18)
    {
        memcpy(buff,str+i  ,9*sizeof(char));b=atoi(buff);
        memcpy(buff,str+i+9,9*sizeof(char));b=b*1000000000+((uint64_t)atoi(buff));
        mS64ToLInt(b,&c);
        mLIntMulU32(a,1000000000,a);
        mLIntMulU32(a,1000000000,a);
        mLIntAdd(a,&c,a);
    }
    l=l-i;b=0;
    if(l>=9)
    {
        memcpy(buff,str+i,9*sizeof(char));b=atoi(buff);
        mLIntMulU32(a,1000000000,a);
        l=l-9;i=i+9;
    }
    if(l>0)
    {
        uint32_t d=1;for(int j=0;j<l;j++) d=d*10;
        b=b*(uint64_t)d+(uint64_t)atoi(str+i);
        mLIntMulU32(a,d,a);
    }
    mS64ToLInt(b,&c);
    mLIntAdd(a,&c,a);
    a->sign = sign;
}
