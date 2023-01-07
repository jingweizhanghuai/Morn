/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_math.h"

struct HandleLInt
{
    MArray *buff1;
    MArray *buff2;
    MArray *buff3;
    MArray *buff4;
};
struct HandleLInt *morn_LInt_handle = NULL;
void endLInt(struct HandleLInt *handle)
{
    if(handle->buff1!=NULL) mArrayRelease(handle->buff1);
    if(handle->buff2!=NULL) mArrayRelease(handle->buff2);
    if(handle->buff3!=NULL) mArrayRelease(handle->buff3);
    if(handle->buff4!=NULL) mArrayRelease(handle->buff4);
    morn_LInt_handle=NULL;
}
#define HASH_LInt 0xec1b15a6

struct HandleLInt *LIntIint()
{
    if(morn_LInt_handle!=NULL) return morn_LInt_handle;
    MHandle *hdl = mHandle("LInt",LInt);
    struct HandleLInt *handle = hdl->handle;
    if(!mHandleValid(hdl))
    {
        if(handle->buff1==NULL) handle->buff1 = mArrayCreate(16,sizeof(uint32_t));
        if(handle->buff2==NULL) handle->buff2 = mArrayCreate(16,sizeof(uint32_t));
        if(handle->buff3==NULL) handle->buff3 = mArrayCreate(16,sizeof(uint32_t));
        if(handle->buff4==NULL) handle->buff4 = mArrayCreate(16,sizeof(uint32_t));
        hdl->valid=1;
    }
    morn_LInt_handle=handle;
    return handle;
}
#define LIntBuff1() (LIntIint()->buff1)
#define LIntBuff2() (LIntIint()->buff2)
#define LIntBuff3() (LIntIint()->buff3)
#define LIntBuff4() (LIntIint()->buff4)

#ifdef __GNUC__

#ifdef __amd64
#define LInt128
#endif
#endif

#ifdef LInt128
void mIntToLInt(MArray *a,int64_t in)
{
    mArrayRedefine(a,16,sizeof(int64_t));
    if(in<0) {a->dataU64[0]=1;a->dataU64[1]=0-in;}
    else     {a->dataU64[0]=0;a->dataU64[1]=  in;}
    a->num = 2;
}
int64_t mLIntToInt(MArray *a)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    uint64_t data=(a->num>2)?0x7fffffffffffffff:(int64_t)(a->dataU64[1]);
    return ((a->dataU64[0])?(0-data):data);
}

#define SIGN(A) ((A)->dataU64[0])
void m_LIntAdd(MArray *a,MArray *b,MArray *c)
{
    if(c==NULL) c=a;
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if((SIGN(a)==0)&&(SIGN(b)==1)) {SIGN(b)=0;m_LIntSub(a,b,c);SIGN(b)=1;return;}
    if((SIGN(a)==1)&&(SIGN(b)==0)) {SIGN(a)=0;m_LIntSub(b,a,c);SIGN(a)=1;return;}
    if(a->num<b->num) {m_LIntAdd(b,a,c);return;}
    
    MArray *pc=c;if((c==a)||(c==b)) c=LIntBuff1();
    mArrayRedefine(c,MAX(a->num,b->num)+1,sizeof(uint64_t));
    SIGN(c)=SIGN(a);
    
    int i;
    __uint128_t rst=0;
    for(i=1;i<b->num;i++)
    {
        rst = rst+(__uint128_t)(a->dataU64[i])+(__uint128_t)(b->dataU64[i]);
        c->dataU64[i]=rst&0x0ffffffffffffffff;
        rst = rst>>64;
    }
    for(;i<a->num;i++)
    {
        rst = rst+(__uint128_t)(a->dataU64[i]);
        c->dataU64[i]=rst&0x0ffffffffffffffff;
        rst = rst>>64;
        if(rst==0) break;
    }
    if(i==a->num)
    {
        if(rst==0) {                           c->num=a->num;}
        else       {c->dataU64[i]=(uint64_t)rst;c->num=i+1;  }
    }
    else
    {
        memcpy(c->dataU64+i+1,a->dataU64+i+1,(a->num-i-1)*sizeof(uint64_t));
        c->num = a->num;
    }
    if(pc==a) mArrayDataExchange(a,c);
    if(pc==b) mArrayDataExchange(b,c);
}

void m_LIntAddInt(MArray *a,int32_t b,MArray *c)
{
    if(c==NULL) c=a;
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if((SIGN(a)==0)&&(b< 0)) {          m_LIntSubInt(a,0-b,c);return;}
    if((SIGN(a)==1)&&(b>=0)) {SIGN(a)=0;m_LIntSubInt(a,  b,c);SIGN(a)=1;SIGN(c)=!SIGN(c);return;}
    if(b<0) b=0-b;
    
    MArray *pc=c;if(c==a) c=LIntBuff1();
    mArrayRedefine(c,a->num+1,sizeof(uint64_t));
    SIGN(c)=SIGN(a);
    
    __uint128_t rst = (__uint128_t)(a->dataU64[1])+(__uint128_t)b;
    c->dataU64[1]=(uint64_t)(rst&0x0ffffffffffffffff);
    rst = rst>>64;
    
    int i;for(i=2;i<a->num;i++)
    {
        rst = rst+(__uint128_t)(a->dataU64[i]);
        c->dataU64[i]=(uint64_t)(rst&0x0ffffffffffffffff);
        rst = rst>>64;
        if(rst==0) break;
    }
    if(i==a->num)
    {
        if(rst==0) {                            c->num=a->num;}
        else       {c->dataU64[i]=(uint64_t)rst;c->num=i+1;   }
    }
    else
    {
        memcpy(c->dataU64+i+1,a->dataU64+i+1,(a->num-i-1)*sizeof(uint64_t));
        c->num = a->num;
    }
    if(pc==a) mArrayDataExchange(a,c);
}

int mLIntCompare(MArray *a,MArray *b)
{
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if((SIGN(a)==0)&&(SIGN(b)==1)) return  1;
    if((SIGN(a)==1)&&(SIGN(b)==0)) return -1;
    if(a->num>b->num) return ((SIGN(a))?-1:1);
    if(a->num<b->num) return ((SIGN(a))?1:-1);
    for(int i=a->num-1;i>0;i--)
    {
        if(a->dataU64[i]>b->dataU64[i]) return ((SIGN(a))?-1:1);
        if(a->dataU64[i]<b->dataU64[i]) return ((SIGN(a))?1:-1);
    }
    return 0;
}

void m_LIntSub(MArray *a,MArray *b,MArray *c)
{
    if(c==NULL) c=a;
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if((SIGN(a)==0)&&(SIGN(b)==1)) {SIGN(b)=0;m_LIntAdd(a,b,c);SIGN(b)=1;return;}
    if((SIGN(a)==1)&&(SIGN(b)==0)) {SIGN(b)=1;m_LIntAdd(a,b,c);SIGN(b)=0;return;}
    int flag = mLIntCompare(a,b);if(flag==0) {mArrayRedefine(c,1,sizeof(uint64_t));SIGN(c)=0;return;}
    if(flag<0) {m_LIntSub(b,a,c);SIGN(c)=!SIGN(c);return;}
    
    MArray *pc=c;if((c==a)||(c==b)) c=LIntBuff1();
    mArrayRedefine(c,MAX(a->num,b->num),sizeof(uint64_t));
    SIGN(c)=SIGN(a);
    
    int i;__uint128_t rst=1;
    for(i=1;i<b->num;i++)
    {
        rst = rst+0x0ffffffffffffffff+(__uint128_t)(a->dataU64[i])-(__uint128_t)(b->dataU64[i]);
        c->dataU64[i]=(uint64_t)(rst&0x0ffffffffffffffff);
        if(c->dataU64[i]) c->num=i+1;
        rst = rst>>64;
    }
    for(;i<a->num;i++)
    {
        rst = rst+0x0ffffffffffffffff+(__uint128_t)(a->dataU64[i]);
        c->dataU64[i]=(uint64_t)(rst&0x0ffffffffffffffff);
        if(c->dataU64[i]) c->num=i+1;
        rst = rst>>64;
        if(rst==1) break;
    }
    i=i+1;
    if(i<a->num)
    {
        memcpy(c->dataU64+i,a->dataU64+i,(a->num-i)*sizeof(uint64_t));
        c->num=a->num;
    }
    if(pc==a) mArrayDataExchange(a,c);
    if(pc==b) mArrayDataExchange(b,c);
}

void m_LIntSubInt(MArray *a,int32_t b,MArray *c)
{
    if(c==NULL) c=a;
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if((SIGN(a)==0)&&(b<0)) {m_LIntAddInt(a,0-b,c);return;}
    if((SIGN(a)==1)&&(b>0)) {m_LIntAddInt(a,0-b,c);return;}
    
    MArray *pc=c;if(c==a) c=LIntBuff1();
    mArrayRedefine(c,a->num,sizeof(uint64_t));
    if(a->num==2)
    {
        c->num=2;
             if(a->dataU64[1]<(uint64_t)b) {c->dataU64[1]=(uint64_t)b-a->dataU64[1];c->num=2;SIGN(c)=1;}
        else if(a->dataU64[1]>(uint64_t)b) {c->dataU64[1]=a->dataU64[1]-(uint64_t)b;c->num=2;SIGN(c)=0;}
        else                               {                                        c->num=1;SIGN(c)=0;}
        if(pc==a) mArrayDataExchange(a,c);
        return;
    }
    SIGN(c)=SIGN(a);
    
    __uint128_t rst=1+0x0ffffffffffffffff+(__uint128_t)(a->dataU64[1])-(__uint128_t)b;
    c->dataU64[1]=rst&0x0ffffffffffffffff;
    c->num=2;
    rst = rst>>64;
    int i=2;
    for(;i<a->num;i++)
    {
        rst = rst+0x0ffffffffffffffff+(__uint128_t)(a->dataU64[i]);
        c->dataU64[i]=rst&0x0ffffffffffffffff;
        if(c->dataU64[i]) c->num=i+1;
        rst = rst>>64;
        if(rst==1) break;
    }
    i=i+1;
    if(i<a->num)
    {
        memcpy(c->dataU64+i,a->dataU64+i,(a->num-i)*sizeof(uint64_t));
        c->num=a->num;
    }
    if(pc==a) mArrayDataExchange(a,c);
}

void LIntMulU64(MArray *a,uint64_t b,MArray *c)
{
    __uint128_t rst=0;
    __uint128_t k=b;
    for(int i=1;i<a->num;i++)
    {
        rst=rst+(__uint128_t)(a->dataU64[i])*k;
        c->dataU64[i] = rst&0x0ffffffffffffffff;
        rst=rst>>64;
    }
    if(rst==0) c->num=a->num;
    else {c->dataU64[a->num]=(uint64_t)rst; c->num=a->num+1;}
}

// void LIntMulU64(MArray *a,uint64_t b,MArray *c)
// {
//     c->dataU64[1]=0;
//     for(int i=1;i<a->num;i++)
//     {
//         __uint128_t *rst = (__uint128_t *)(c->dataU64+i);
//         c->dataU64[i+1]=0;
//         *rst=*rst+(__uint128_t)(a->dataU64[i])*(__uint128_t)b;
//     }
//     c->num = a->num+(c->dataU64[a->num]!=0);
// }

void m_LIntMulInt(MArray *a,int32_t b,MArray *c)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if(c==NULL) {c=a;} MArray *pc=c;if(c==a) c=LIntBuff2();
    
    mArrayRedefine(c,a->num+1,sizeof(uint64_t));
    
    int sign=(b<0);if(sign) b=0-b;
    LIntMulU64(a,(uint64_t)b,c);
    SIGN(c)=(SIGN(a)!=sign);
    
    if(pc==a) mArrayDataExchange(a,c);
}

void m_LIntMul(MArray *a,MArray *b,MArray *c)
{
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if(c==NULL) {c=a;} MArray *pc=c;if((c==a)||(c==b)) c=LIntBuff2();
    mArrayRedefine(c,a->num+b->num,sizeof(uint64_t));
    LIntMulU64(a,b->dataU64[1],c);
    
    MArray *d=LIntBuff1();
    mArrayRedefine(d,a->num+b->num,sizeof(uint64_t));
    for(int i=2;i<b->num;i++)
    {
        LIntMulU64(a,b->dataU64[i],d);
        memmove(d->dataU64+i,d->dataU64+1,(d->num-1)*sizeof(uint64_t));
        memset(d->dataU64+1,0,(i-1)*sizeof(uint64_t));
        d->num+=i-1;
        m_LIntAdd(d,c,c);
    }
    SIGN(c)=(SIGN(a)!=SIGN(b));
    
    if(pc==a) mArrayDataExchange(a,c);
    if(pc==b) mArrayDataExchange(b,c);
}

void LIntDivU64(MArray *a,uint64_t b,MArray *c,int64_t *remainder)
{
    int i=a->num-1;
    __uint128_t rst = (__uint128_t)(a->dataU64[i]);
    c->dataU64[i]=(uint64_t)(rst/b); rst=rst%b;
    c->num = (c->dataU64[i]==0)?(a->num-1):a->num;
    for(i=i-1;i>=1;i--)
    {
        rst=(rst<<64)+(__uint128_t)(a->dataU64[i]);
        c->dataU64[i]=(uint64_t)(rst/b);rst=rst%b;
    }
    if(remainder!=NULL) *remainder=(SIGN(a))?(0-(uint64_t)rst):(uint64_t)rst;
}

void m_LIntDivInt(MArray *a,int b,MArray *c,int *remainder)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if(c==NULL) {c=a;} MArray *pc=c;if(c==a) c=LIntBuff3();
    mArrayRedefine(c,a->num,sizeof(uint64_t));
    
    int sign=(b<0);if(sign) b=0-b;
    
    int64_t rmd;
    LIntDivU64(a,(uint64_t)b,c,&rmd);
    *remainder=(int)rmd;
    SIGN(c) = (SIGN(a)!=sign);
    if(pc==a) mArrayDataExchange(a,c);
}

void m_LIntDiv(MArray *a,MArray *b,MArray *c,MArray *remainder)
{
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if(c==NULL) c=a;
    MArray *pc=c        ;if((c        ==a)||(c        ==b))                            c=LIntBuff3();
    MArray *pr=remainder;if((remainder==a)||(remainder==b)||(remainder==NULL)) remainder=LIntBuff4();
    mArrayRedefine(c        ,a->num,sizeof(uint64_t));
    mArrayRedefine(remainder,b->num,sizeof(uint64_t));
    
    int num_a=a->num;
    uint64_t *buff =mMalloc(num_a*sizeof(uint64_t));
    memcpy(buff,a->dataU64, num_a*sizeof(uint64_t));
        
    __uint128_t a0,b0;
    while(mLIntCompare(a,b)>=0)
    {
        memset(remainder->dataU64,0,remainder->num*sizeof(uint64_t));
        int a_len = a->num;int b_len = b->num;
        b0 = (__uint128_t)(b->dataU64[b_len-1])+1;
        
        if(a_len>b_len)
        {
            a0 = (((__uint128_t)(a->dataU64[a_len-1]))<<64)+(__uint128_t)(a->dataU64[a_len-2]);
            remainder->num = a_len-b_len+1;
            a0=a0/b0;a0=MAX(a0,1);
            remainder->dataU64[remainder->num-1]=(uint64_t)(a0>>32);
            remainder->dataU64[remainder->num-2]=(uint64_t)(a0&0x0ffffffffffffffff);
            if(remainder->dataU64[remainder->num-1]==0) remainder->num-=1;
        }
        else//(a_len==b_len)
        {
            a0 = (__uint128_t)(a->dataU64[a_len-1]);
            remainder->num = 2;
            a0=a0/b0;a0=MAX(a0,1);
            remainder->dataU64[0]=(uint64_t)a0;
        }
        m_LIntAdd(c,remainder,c);
        m_LIntMul(b,remainder,remainder);
        m_LIntSub(a,remainder,a);
    }
    memcpy(remainder->dataU64,a->dataU64,a->num*sizeof(uint64_t));remainder->num=a->num;
    memcpy(a->dataU64,buff,num_a*sizeof(uint64_t));a->num=num_a;mFree(buff);
    SIGN(c) = (SIGN(a)!=SIGN(b));
    SIGN(remainder)=SIGN(a);
    
    if(pc==a) mArrayDataExchange(a,c);
    if(pc==b) mArrayDataExchange(b,c);
    if(pr==a) mArrayDataExchange(a,remainder);
    if(pr==b) mArrayDataExchange(b,remainder);
}

void mLIntToString(MArray *a,char *str)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    int num=a->num;
    if(num <=1) {str[0]='0';str[1]=0;return;}
    if(SIGN(a)) {str[0]='-';str++;}
    
    MArray *buff1 = LIntBuff1(); mArrayRedefine(buff1,num,sizeof(uint64_t));
    MArray *buff2 = LIntBuff2(); mArrayRedefine(buff2,num,sizeof(uint64_t));
    SIGN(buff1)=0;memcpy(buff1->dataU64+1,a->dataU64+1,(num-1)*sizeof(uint64_t));
    
    int64_t *rst = mMalloc(num*2*sizeof(int64_t));
    
    int i=num+num-1;
    while(1)
    {
        LIntDivU64(buff1,100000000000000000,buff2,rst+i);i--;
        if(buff2->num==1) break;
        LIntDivU64(buff2,100000000000000000,buff1,rst+i);i--;
        if(buff1->num==1) break;
    }
    
    i=i+1;sprintf(str,"%ld",rst[i]);str+=strlen(str);
    for(i=i+1;i<num+num;i++) {sprintf(str,"%09d%08d",rst[i]/100000000,rst[i]%100000000);str+=17;}
    str[0]=0;
    
    mFree(rst);
}

void mStringToLInt(MArray *a,const char *str)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    int l = strlen(str);
    mArrayRedefine(a,l/10,sizeof(uint64_t));
    memset(a->dataU64,0,a->num*sizeof(uint64_t));
    SIGN(a)=0;
    if(str[0]=='-') {str++;l--;SIGN(a)=1;}
    
    MArray *c = LIntBuff1();mArrayRedefine(c,a->num,sizeof(uint64_t));
    memset(c->dataU64,0,a->num*sizeof(uint64_t));
    
    char buff[10];buff[9]=0;
    memcpy(buff,str  ,9*sizeof(char)); int b=atoi(buff);
    mIntToLInt(a,(int64_t)b);
    
    int i=9;for(;i<=l-9;i+=9)
    {
        memcpy(buff,str+i,9*sizeof(char));b=atoi(buff);
        m_LIntMulInt(a,1000000000,c);
        m_LIntAddInt(c,b,a);
    }
    if(i<l)
    {
        int d=1;b=0;
        for(;i<l;i++){d=d*10;b=b*10+str[i]-'0';}
        m_LIntMulInt(a,d,c);
        m_LIntAddInt(c,b,a);
    }
}
#else
void mIntToLInt(MArray *a,int64_t in)
{
    mArrayRedefine(a,16,sizeof(int32_t));
    if(in<0) {a->dataU32[0]=1;in=0-in;}
    else     {a->dataU32[0]=0;}
    
    a->dataU32[1] = in&0x0ffffffff;
    a->dataU32[2] = in>>32;
    
    a->num = (a->dataU32[2]==0)?2:3;
}

int64_t mLIntToInt(MArray *a)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    
    if(a->num>3) return (a->dataU32[0])?(0-0x7fffffffffffffff):0x7fffffffffffffff;
    
    uint64_t data=(((uint64_t)(a->dataU32[2]))<<32)+(int64_t)(a->dataU32[1]);
    if(data>0x7fffffffffffffff) data=0x7fffffffffffffff;
    return ((a->dataU32[0])?(0-data):data);
}

#define SIGN(A) ((A)->dataU32[0])
void m_LIntAdd(MArray *a,MArray *b,MArray *c)
{
    if(c==NULL) c=a;
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if((SIGN(a)==0)&&(SIGN(b)==1)) {SIGN(b)=0;m_LIntSub(a,b,c);SIGN(b)=1;return;}
    if((SIGN(a)==1)&&(SIGN(b)==0)) {SIGN(a)=0;m_LIntSub(b,a,c);SIGN(a)=1;return;}
    if(a->num<b->num) {m_LIntAdd(b,a,c);return;}
    
    MArray *pc=c;if((c==a)||(c==b)) c=LIntBuff1();
    mArrayRedefine(c,MAX(a->num,b->num)+1,sizeof(uint32_t));
    SIGN(c)=SIGN(a);
    
    int i;
    uint64_t rst=0;
    for(i=1;i<b->num;i++)
    {
        rst = rst+(uint64_t)(a->dataU32[i])+(uint64_t)(b->dataU32[i]);
        c->dataU32[i]=rst&0x0ffffffff;
        rst = rst/0x0ffffffff;
    }
    for(;i<a->num;i++)
    {
        rst = rst+(uint64_t)(a->dataU32[i]);
        c->dataU32[i]=rst&0x0ffffffff;
        rst = rst>>32;
        if(rst==0) break;
    }
    if(i==a->num)
    {
        if(rst==0) {                  c->num=a->num;}
        else       {c->dataU32[i]=rst;c->num=i+1;   }
    }
    else
    {
        memcpy(c->dataU32+i+1,a->dataU32+i+1,(a->num-i-1)*sizeof(uint32_t));
        c->num = a->num;
    }
    if(pc==a) mArrayDataExchange(a,c);
    if(pc==b) mArrayDataExchange(b,c);
}

void m_LIntAddInt(MArray *a,int32_t b,MArray *c)
{
    if(c==NULL) c=a;
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if((SIGN(a)==0)&&(b< 0)) {          m_LIntSubInt(a,0-b,c);return;}
    if((SIGN(a)==1)&&(b>=0)) {SIGN(a)=0;m_LIntSubInt(a,  b,c);SIGN(a)=1;SIGN(c)=!SIGN(c);return;}
    if(b<0) b=0-b;
    
    MArray *pc=c;if(c==a) c=LIntBuff1();
    mArrayRedefine(c,a->num+1,sizeof(uint32_t));
    SIGN(c)=SIGN(a);
    
    uint64_t rst = (uint64_t)(a->dataU32[1])+(uint64_t)b;
    c->dataU32[1]=(uint32_t)(rst&0x0ffffffff);
    rst = rst>>32;
    
    int i;for(i=2;i<a->num;i++)
    {
        rst = rst+a->dataU32[i];
        c->dataU32[i]=rst&0x0ffffffff;
        rst = rst>>32;
        if(rst==0) break;
    }
    if(i==a->num)
    {
        if(rst==0) {c->num=a->num;}
        else       {c->dataU32[i]=rst;c->num=i+1;}
    }
    else
    {
        memcpy(c->dataU32+i+1,a->dataU32+i+1,(a->num-i-1)*sizeof(uint32_t));
        c->num = a->num;
    }
    if(pc==a) mArrayDataExchange(a,c);
}

int mLIntCompare(MArray *a,MArray *b)
{
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if((SIGN(a)==0)&&(SIGN(b)==1)) return  1;
    if((SIGN(a)==1)&&(SIGN(b)==0)) return -1;
    if(a->num>b->num) return ((SIGN(a))?-1:1);
    if(a->num<b->num) return ((SIGN(a))?1:-1);
    for(int i=a->num-1;i>0;i--)
    {
        if(a->dataU32[i]>b->dataU32[i]) return ((SIGN(a))?-1:1);
        if(a->dataU32[i]<b->dataU32[i]) return ((SIGN(a))?1:-1);
    }
    return 0;
}

void m_LIntSub(MArray *a,MArray *b,MArray *c)
{
    if(c==NULL) c=a;
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if((SIGN(a)==0)&&(SIGN(b)==1)) {SIGN(b)=0;m_LIntAdd(a,b,c);SIGN(b)=1;return;}
    if((SIGN(a)==1)&&(SIGN(b)==0)) {SIGN(b)=1;m_LIntAdd(a,b,c);SIGN(b)=0;return;}
    int flag = mLIntCompare(a,b);if(flag==0) {mArrayRedefine(c,1,sizeof(uint32_t));SIGN(c)=0;return;}
    if(flag<0) {m_LIntSub(b,a,c);SIGN(c)=!SIGN(c);return;}
    
    MArray *pc=c;if((c==a)||(c==b)) c=LIntBuff1();
    mArrayRedefine(c,MAX(a->num,b->num),sizeof(uint32_t));
    SIGN(c)=SIGN(a);
    
    int i;uint64_t rst=1;
    for(i=1;i<b->num;i++)
    {
        rst = rst+0x0100000000+a->dataU32[i]-1-b->dataU32[i];
        c->dataU32[i]=rst&0x0ffffffff;
        if(c->dataU32[i]) c->num=i+1;
        rst = rst>>32;
    }
    for(;i<a->num;i++)
    {
        rst = rst+0x100000000+a->dataU32[i]-1;
        c->dataU32[i]=rst&0x0ffffffff;
        if(c->dataU32[i]) c->num=i+1;
        rst = rst>>32;
        if(rst==1) break;
    }
    i=i+1;
    if(i<a->num)
    {
        memcpy(c->dataU32+i,a->dataU32+i,(a->num-i)*sizeof(uint32_t));
        c->num=a->num;
    }
    if(pc==a) mArrayDataExchange(a,c);
    if(pc==b) mArrayDataExchange(b,c);
}

void m_LIntSubInt(MArray *a,int32_t b,MArray *c)
{
    if(c==NULL) c=a;
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if((SIGN(a)==0)&&(b<0)) {m_LIntAddInt(a,0-b,c);return;}
    if((SIGN(a)==1)&&(b>0)) {m_LIntAddInt(a,0-b,c);return;}
    
    MArray *pc=c;if(c==a) c=LIntBuff1();
    mArrayRedefine(c,a->num,sizeof(int32_t));
    if(a->num==2)
    {
        c->num=2;
             if(a->dataU32[1]<b) {c->dataU32[1]=b-a->dataU32[1];c->num=2;SIGN(c)=1;}
        else if(a->dataU32[1]>b) {c->dataU32[1]=a->dataU32[1]-b;c->num=2;SIGN(c)=0;}
        else                     {                              c->num=1;SIGN(c)=0;}
        if(pc==a) mArrayDataExchange(a,c);
        return;
    }
    SIGN(c)=SIGN(a);
    
    uint64_t rst=0x0100000000+a->dataU32[1]-b;
    c->dataU32[1]=rst&0x0ffffffff;
    c->num=2;
    rst = rst>>32;
    int i=2;
    for(;i<a->num;i++)
    {
        rst = rst+0x100000000+a->dataU32[i]-1;
        c->dataU32[i]=rst&0x0ffffffff;
        if(c->dataU32[i]) c->num=i+1;
        rst = rst>>32;
        if(rst==1) break;
    }
    i=i+1;
    if(i<a->num)
    {
        memcpy(c->dataU32+i,a->dataU32+i,(a->num-i)*sizeof(uint32_t));
        c->num=a->num;
    }
    if(pc==a) mArrayDataExchange(a,c);
}

void LIntMulU32(MArray *a,uint32_t b,MArray *c)
{
    uint64_t rst=0;
    int i=1;for(;i<a->num;i++)
    {
        rst=rst+(uint64_t)(a->dataU32[i])*(uint64_t)b;
        c->dataU32[i] = rst&0x0ffffffff;
        rst=rst>>32;
    }
    if(rst==0) c->num=a->num;
    else {c->dataU32[i]=rst; c->num=a->num+1;}
}

void m_LIntMulInt(MArray *a,int32_t b,MArray *c)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if(c==NULL) {c=a;} MArray *pc=c;if(c==a) c=LIntBuff2();
    
    mArrayRedefine(c,a->num+1,sizeof(uint32_t));
    
    int sign=(b<0);if(sign) b=0-b;
    LIntMulU32(a,b,c);
    SIGN(c)=(SIGN(a)!=sign);
    
    if(pc==a) mArrayDataExchange(a,c);
}

void m_LIntMul(MArray *a,MArray *b,MArray *c)
{
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if(c==NULL) {c=a;} MArray *pc=c;if((c==a)||(c==b)) c=LIntBuff2();
    mArrayRedefine(c,a->num+b->num,sizeof(uint32_t));
    LIntMulU32(a,b->dataU32[1],c);
    
    MArray *d=LIntBuff1();
    mArrayRedefine(d,a->num+b->num,sizeof(uint32_t));
    for(int i=2;i<b->num;i++)
    {
        LIntMulU32(a,b->dataU32[i],d);
        memmove(d->dataU32+i,d->dataU32+1,(d->num-1)*sizeof(uint32_t));
        memset(d->dataU32+1,0,(i-1)*sizeof(uint32_t));
        d->num+=i-1;
        m_LIntAdd(d,c,c);
    }
    SIGN(c)=(SIGN(a)!=SIGN(b));
    
    if(pc==a) mArrayDataExchange(a,c);
    if(pc==b) mArrayDataExchange(b,c);
}

void LIntDivU32(MArray *a,uint32_t b,MArray *c,int32_t *remainder)
{
    int i=a->num-1;
    uint64_t rst = a->dataU32[i];
    c->dataU32[i]=rst/b; rst=rst%b;
    c->num = (c->dataU32[i]==0)?(a->num-1):a->num;
    for(i=i-1;i>=1;i--)
    {
        rst=(rst<<32)+a->dataU32[i];
        c->dataU32[i]=rst/b;rst=rst%b;
    }
    if(remainder!=NULL) *remainder=(SIGN(a))?(0-(int32_t)rst):(int32_t)rst;
}

void m_LIntDivInt(MArray *a,int b,MArray *c,int *remainder)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if(c==NULL) {c=a;} MArray *pc=c;if(c==a) c=LIntBuff3();
    mArrayRedefine(c,a->num,sizeof(uint32_t));
    
    int sign=(b<0);
    if(sign) b=0-b;
    
    LIntDivU32(a,b,c,remainder);
    SIGN(c) = (SIGN(a)!=sign);
    if(pc==a) mArrayDataExchange(a,c);
}

void m_LIntDiv(MArray *a,MArray *b,MArray *c,MArray *remainder)
{
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if(c==NULL) c=a;
    MArray *pc=c        ;if((c        ==a)||(c        ==b))                            c=LIntBuff3();
    MArray *pr=remainder;if((remainder==a)||(remainder==b)||(remainder==NULL)) remainder=LIntBuff4();
    mArrayRedefine(c        ,a->num,sizeof(uint32_t));
    mArrayRedefine(remainder,b->num,sizeof(uint32_t));
    
    int num_a=a->num;
    uint32_t *buff =mMalloc(num_a*sizeof(int32_t));
    memcpy(buff,a->dataU32, num_a*sizeof(int32_t));
        
    uint64_t a0,b0;
    while(mLIntCompare(a,b)>=0)
    {
        memset(remainder->dataU32,0,remainder->num*sizeof(uint32_t));
        int a_len = a->num;int b_len = b->num;
        b0 = (uint64_t)(b->dataU32[b_len-1])+1;
        
        if(a_len>b_len)
        {
            a0 = (((uint64_t)(a->dataU32[a_len-1]))<<32)+(uint64_t)(a->dataU32[a_len-2]);
            remainder->num = a_len-b_len+1;
            a0=a0/b0;a0=MAX(a0,1);
            remainder->dataU32[remainder->num-1]=(uint32_t)(a0>>32);
            remainder->dataU32[remainder->num-2]=(uint32_t)(a0&0x0ffffffff);
            if(remainder->dataU32[remainder->num-1]==0) remainder->num-=1;
        }
        else //(a_len==b_len)
        {
            a0 = (uint64_t)(a->dataU32[a_len-1]);
            remainder->num = 2;
            a0=a0/b0;a0=MAX(a0,1);
            remainder->dataU32[0]=(uint32_t)a0;
        }
        m_LIntAdd(c,remainder,c);
        m_LIntMul(b,remainder,remainder);
        m_LIntSub(a,remainder,a);
    }
    memcpy(remainder->dataU32,a->dataU32,a->num*sizeof(uint32_t));remainder->num=a->num;
    memcpy(a->dataU32,buff,num_a*sizeof(uint32_t));a->num=num_a;mFree(buff);
    SIGN(c) = (SIGN(a)!=SIGN(b));
    SIGN(remainder)=SIGN(a);
    
    if(pc==a) mArrayDataExchange(a,c);
    if(pc==b) mArrayDataExchange(b,c);
    if(pr==a) mArrayDataExchange(a,remainder);
    if(pr==b) mArrayDataExchange(b,remainder);
}

void mLIntToString(MArray *a,char *str)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    int num=a->num;
    if(num <=1) {str[0]='0';str[1]=0;return;}
    if(SIGN(a)) {str[0]='-';str++;}
    
    MArray *buff1 = LIntBuff1(); mArrayRedefine(buff1,num,sizeof(uint32_t));
    MArray *buff2 = LIntBuff2(); mArrayRedefine(buff2,num,sizeof(uint32_t));
    SIGN(buff1)=0;memcpy(buff1->dataU32+1,a->dataU32+1,(num-1)*sizeof(uint32_t));
    
    int *rst = mMalloc(num*2*sizeof(int));
    
    int i=num+num-1;
    while(1)
    {
        LIntDivU32(buff1,1000000000,buff2,rst+i);i--;
        if(buff2->num==1) break;
        LIntDivU32(buff2,1000000000,buff1,rst+i);i--;
        if(buff1->num==1) break;
    }
    i=i+1;sprintf(str,"%d",rst[i]);str+=strlen(str);
    for(i=i+1;i<num+num;i++) {sprintf(str,"%09d",rst[i]);str+=9;}
    str[0]=0;
    
    mFree(rst);
}

void mStringToLInt(MArray *a,const char *str)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    int l = strlen(str);
    mArrayRedefine(a,l/10,sizeof(uint32_t));
    memset(a->dataU32,0,a->num*sizeof(uint32_t));
    SIGN(a)=0;
    if(str[0]=='-') {str++;l--;SIGN(a)=1;}
    
    MArray *c = LIntBuff1();mArrayRedefine(c,a->num,sizeof(uint32_t));
    memset(c->dataU32,0,a->num*sizeof(uint32_t));
    
    char buff[10];buff[9]=0;
    memcpy(buff,str  ,9*sizeof(char)); int b=atoi(buff);
    mIntToLInt(a,(int64_t)b);
    
    int i=9;for(;i<=l-9;i+=9)
    {
        memcpy(buff,str+i,9*sizeof(char));b=atoi(buff);
        m_LIntMulInt(a,1000000000,c);
        m_LIntAddInt(c,b,a);
    }
    if(i<l)
    {
        int d=1;b=0;
        for(;i<l;i++){d=d*10;b=b*10+str[i]-'0';}
        m_LIntMulInt(a,d,c);
        m_LIntAddInt(c,b,a);
    }
}
#endif
