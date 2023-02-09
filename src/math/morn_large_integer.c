/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_math.h"

struct HandleLIntBuff
{
    MArray *buff1;
    MArray *buff2;
    MArray *buff3;
    MArray *buff4;
};
struct HandleLIntBuff *morn_LInt_handle = NULL;
void endLIntBuff(struct HandleLIntBuff *handle)
{
    if(handle->buff1!=NULL) mArrayRelease(handle->buff1);
    if(handle->buff2!=NULL) mArrayRelease(handle->buff2);
    if(handle->buff3!=NULL) mArrayRelease(handle->buff3);
    if(handle->buff4!=NULL) mArrayRelease(handle->buff4);
    morn_LInt_handle=NULL;
}
#define HASH_LIntBuff 0x7c0e2065
struct HandleLIntBuff *LIntIint()
{
    if(morn_LInt_handle!=NULL) return morn_LInt_handle;
    MHandle *hdl = mHandle("LInt",LIntBuff);
    struct HandleLIntBuff *handle = hdl->handle;
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

#define HASH_LInt 0xec1b15a6
// #define LInt_ADD 1
// #define LInt_SUB 2
#define LInt_MUL 3
#define LInt_DIV 4
#define LInt_NUL 0

#ifdef __GNUC__
#ifdef __amd64
#define LInt128
#endif
#endif

#ifdef LInt128
#define SIGN(A) (A->dataS32[0])
#define TYPE(A) (A->dataS32[1])
#define DATA(A) (A->dataU64[1])
static uint64_t *sb_gcc;
#define HI(A) (sb_gcc=(uint64_t *)(&(A)))[1]
#define LO(A) (sb_gcc=(uint64_t *)(&(A)))[0]
// #define HI(A) ((A)>>64)
// #define LO(A) ((A)&0x0ffffffffffffffff)

void LIntAppend(MArray *a,int num)
{
    if(num<a->num) return;
    if(a->element_size!=sizeof(uint64_t)) {mArrayRedefine(a,num,sizeof(int64_t));a->num=2;return;}
    int a_num=a->num; mArrayAppend(a,num-a->num);a->num=a_num;
}

void LIntCaculate(MArray *a);
void mIntToLInt(MArray *a,int64_t in)
{
    LIntAppend(a,32);
    
    if(in<0) {SIGN(a)=1;a->dataU64[2]=0-in;}
    else     {SIGN(a)=0;a->dataU64[2]=  in;}
    TYPE(a)=LInt_NUL;DATA(a)=1;
    a->num = 3;
}
int64_t mLIntToInt(MArray *a)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if(a->num >4) return (SIGN(a)==0)?0x7fffffffffffffff:0-0x7fffffffffffffff;
    
    if(a->num==3) a->dataU64[3]=0;
    __uint128_t data=(((__uint128_t)(a->dataU64[3]))<<64)+(__uint128_t)(a->dataU64[2]);
    
         if(TYPE(a)==LInt_MUL) data=data*(__uint128_t)DATA(a);
    else if(TYPE(a)==LInt_DIV) data=data/(__uint128_t)DATA(a);
    data=MIN(data,0x7fffffffffffffff);
    if(data>0x7fffffffffffffff) return (SIGN(a)==0)?0x7fffffffffffffff:0-0x7fffffffffffffff;
    return (int64_t)((SIGN(a)==0)?data:0-data);
}

void m_LIntAdd(MArray *a,MArray *b,MArray *c)
{
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if(c==NULL) c=a;
    if(TYPE(a)) LIntCaculate(a);
    if(TYPE(b)) LIntCaculate(b);
    if(a->num<b->num) {m_LIntAdd(b,a,c);return;}
    
    LIntAppend(c,a->num+1);
    TYPE(c)=LInt_NUL;DATA(c)=1;
    if((SIGN(a)==0)&&(SIGN(b)==1)) {SIGN(b)=0;m_LIntSub(a,b,c);SIGN(b)=1;return;}
    if((SIGN(a)==1)&&(SIGN(b)==0)) {SIGN(a)=0;m_LIntSub(b,a,c);SIGN(a)=1;return;}
    SIGN(c)=SIGN(a);
    
    int i=2;__uint128_t rst=0;
    for(;i<b->num;i++)
    {
        rst = rst+(__uint128_t)(a->dataU64[i])+(__uint128_t)(b->dataU64[i]);
        c->dataU64[i]=LO(rst);
        rst = HI(rst);
    }
    for(;(rst!=0)&&(i<a->num);i++)
    {
        rst = rst+(__uint128_t)(a->dataU64[i]);
        c->dataU64[i]=LO(rst);
        rst = HI(rst);
    }
    if(i==a->num) {c->dataU64[i]=(uint64_t)rst;c->num=i+(c->dataU64[i]!=0);}
    else if(c!=a) {memcpy(c->dataU64+i+1,a->dataU64+i+1,(a->num-i-1)*sizeof(uint64_t));c->num=a->num;}
}

void m_LIntSubInt(MArray *a,int64_t b,MArray *c);
void m_LIntAddInt(MArray *a,int64_t b,MArray *c)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if(c==NULL) c=a;
    
    if(TYPE(a)) LIntCaculate(a);
    LIntAppend(c,a->num+1);
    
    TYPE(c)=LInt_NUL;DATA(c)=1;
    if((SIGN(a)==0)&&(b< 0)) {          m_LIntSubInt(a,0-b,c);                            return;}
    if((SIGN(a)==1)&&(b>=0)) {SIGN(a)=0;m_LIntSubInt(a,  b,c);SIGN(a)=1;SIGN(c)=1-SIGN(c);return;}
    SIGN(c)=SIGN(a);
    if(b<0) b=0-b;
    
    __uint128_t rst = (__uint128_t)(a->dataU64[2])+(__uint128_t)b;
    c->dataU64[2]=LO(rst);
    rst = HI(rst);
    
    int i=3;
    for(;(rst!=0)&&(i<a->num);i++)
    {
        rst = rst+(__uint128_t)(a->dataU64[i]);
        c->dataU64[i]=LO(rst);
        rst = HI(rst);
    }
    if(i==a->num) {c->dataU64[i]=(uint64_t)rst;c->num=i+(c->dataU64[i]!=0);}
    else if(c!=a) {memcpy(c->dataU64+i+1,a->dataU64+i+1,(a->num-i-1)*sizeof(uint64_t));c->num=a->num;}
}

int m_LIntCompare(MArray *a,MArray *b)
{
    if((SIGN(a)==0)&&(SIGN(b)==1)) return  1;
    if((SIGN(a)==1)&&(SIGN(b)==0)) return -1;
    if(a->num>b->num) return ((SIGN(a))?-1:1);
    if(a->num<b->num) return ((SIGN(a))?1:-1);
    for(int i=a->num-1;i>=1;i--)
    {
        if(a->dataU64[i]>b->dataU64[i]) return ((SIGN(a))?-1:1);
        if(a->dataU64[i]<b->dataU64[i]) return ((SIGN(a))?1:-1);
    }
    return 0;
}

int mLIntCompare(MArray *a,MArray *b)
{
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    LIntCaculate(a);LIntCaculate(b);
    return m_LIntCompare(a,b);
}

void m_LIntSub(MArray *a,MArray *b,MArray *c)
{
    if(c==NULL) c=a;
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    
    if(TYPE(a)) LIntCaculate(a);
    if(TYPE(b)) LIntCaculate(b);
    if((SIGN(a)==0)&&(SIGN(b)==1)) {SIGN(b)=0;m_LIntAdd(a,b,c);SIGN(b)=1;return;}
    if((SIGN(a)==1)&&(SIGN(b)==0)) {SIGN(a)=0;m_LIntAdd(a,b,c);SIGN(a)=1;return;}
    
    LIntAppend(c,a->num);
    TYPE(c)=LInt_NUL;DATA(c)=1;
    int flag = m_LIntCompare(a,b);
    if(flag==0) {c->num=3;c->dataU64[2]=0;SIGN(c)=0;return;}
    if(flag <0) {m_LIntSub(b,a,c);SIGN(c)=1-SIGN(c);return;}
    SIGN(c)=SIGN(a);
    
    int i=2;__uint128_t rst=1;
    for(;i<b->num;i++)
    {
        rst = rst+0x0ffffffffffffffff+(__uint128_t)(a->dataU64[i])-(__uint128_t)(b->dataU64[i]);
        c->dataU64[i]=LO(rst);
        if(c->dataU64[i]) c->num=i+1;
        rst = HI(rst);
    }
    for(;(rst>1)&&(i<a->num);i++)
    {
        rst = rst+0x0ffffffffffffffff+(__uint128_t)(a->dataU64[i]);
        c->dataU64[i]=LO(rst);
        if(c->dataU64[i]) c->num=i+1;
        rst = HI(rst);
    }
    
    i=i+1;
    if((i<a->num)&&(c!=a)) {memcpy(c->dataU64+i,a->dataU64+i,(a->num-i)*sizeof(uint64_t));c->num=a->num;}
}

void m_LIntSubInt(MArray *a,int64_t b,MArray *c)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if(c==NULL) c=a;
    
    if(TYPE(a)) LIntCaculate(a);
    if((SIGN(a)==0)&&(b<0)) {m_LIntAddInt(a,0-b,c);return;}
    if((SIGN(a)==1)&&(b>0)) {m_LIntAddInt(a,0-b,c);return;}
    
    LIntAppend(c,a->num);
    SIGN(c)=SIGN(a);if(b<0) b=0-b;
    
    if(a->num==3)
    {
             if(a->dataU64[2]<b) {c->dataU64[2]=b-a->dataU64[2];SIGN(c)= 1;}
        else if(a->dataU64[2]>b) {c->dataU64[2]=a->dataU64[2]-b;SIGN(c)= 0;}
        else                     {c->dataU64[2]=0;              SIGN(c)= 0;}
        c->num=3;return;
    }
    
    __uint128_t rst=1+0x0ffffffffffffffff+(__uint128_t)(a->dataU64[2])-(__uint128_t)b;
    c->dataU64[2]=LO(rst);
    c->num=2;rst =HI(rst);
    
    int i=3;
    for(;(rst>1)&&(i<a->num);i++)
    {
        rst = rst+0x0ffffffffffffffff+(__uint128_t)(a->dataU64[i]);
        c->dataU64[i]=LO(rst);
        if(c->dataU64[i]) c->num=i+1;
        rst = HI(rst);
    }
    i=i+1;
    if((i<a->num)&&(c!=a)) {memcpy(c->dataU64+i,a->dataU64+i,(a->num-i)*sizeof(uint64_t));c->num=a->num;}
}

void LIntMulInt(MArray *a,uint64_t b,MArray *c)
{
    __uint128_t rst=0;
    __uint128_t k=(__uint128_t)b;
    for(int i=2;i<a->num;i++)
    {
        rst=rst+(__uint128_t)(a->dataU64[i])*k;
        c->dataU64[i] = (uint64_t)(rst&0x0ffffffffffffffff);
        rst=rst>>64;
    }
    c->dataU64[a->num]=(uint64_t)rst;
    c->num=a->num+(rst!=0);
}
 
void m_LIntMulInt(MArray *a,int64_t b,MArray *c)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    if(c==NULL) c=a;
    LIntAppend(c,a->num+2);
    
    SIGN(c)=(SIGN(a)!=(b<0));
    if(b<0)b=0-b;
    
    if(TYPE(a)==LInt_DIV)
    {
        if((DATA(a)>b)&&(DATA(a)%b==0))
        {
            if(c!=a) {memcpy(c->dataU64+2,a->dataU64+2,(a->num-2)*sizeof(uint64_t));c->num=a->num;}
            TYPE(c)=LInt_DIV;DATA(c)=DATA(a)/(uint64_t)b;return;
        }
        if((b>DATA(a))&&(b%DATA(a)==0))
        {
            if(c!=a) {memcpy(c->dataU64+2,a->dataU64+2,(a->num-2)*sizeof(uint64_t));c->num=a->num;}
            TYPE(c)=LInt_MUL;DATA(c)=(uint64_t)b/DATA(a);return;
        }
        LIntCaculate(a);
    }
    else if(TYPE(a)==LInt_MUL)
    {
        __uint128_t data=((__uint128_t)(DATA(a)))*((__uint128_t)b);
        if(data<0x0ffffffffffffffff) 
        {
            if(c!=a) {memcpy(c->dataU64+2,a->dataU64+2,(a->num-2)*sizeof(uint64_t));c->num=a->num;}
            TYPE(c)=LInt_MUL;DATA(c)=data;return;
        }
        LIntCaculate(a);
    }
    
    if(c!=a) {memcpy(c->dataU64+2,a->dataU64+2,(a->num-2)*sizeof(uint64_t));c->num=a->num;}
    TYPE(c)=LInt_MUL; DATA(c)=(uint64_t)b;
}

void LIntMul(MArray *a,MArray *b,MArray *c)
{
    int na=a->num-2;int nb=b->num-2;
    uint64_t *buff1=NULL,*buff2=NULL;
    uint64_t *pa;if(c==a){buff1=mMalloc(na*sizeof(uint64_t));memcpy(buff1,a->dataU64+2,na*sizeof(uint64_t));pa=buff1;} else pa=a->dataU64+2;
    uint64_t *pb;if(c==b){buff2=mMalloc(nb*sizeof(uint64_t));memcpy(buff2,b->dataU64+2,nb*sizeof(uint64_t));pb=buff2;} else pb=b->dataU64+2;
//     printf("na=%d,nb=%d\n",na,nb);
    
    __uint128_t mul=((__uint128_t)(pb[0]))*((__uint128_t)(pa[0]));
    __uint128_t rst1=LO(mul);
    __uint128_t rst2=HI(mul);
    c->dataU64[2]=rst1;rst1=rst2;rst2=0;
    
    int j=1;
    for(;j<nb;j++)
    {
        for(int i=0;i<=j;i++)      {mul=((__uint128_t)(pb[i]))*((__uint128_t)(pa[j-i]));rst1+=LO(mul);rst2+=HI(mul);}
        c->dataU64[j+2]=LO(rst1);
        rst1=rst2+HI(rst1);rst2=0;
    }
    for(;j<na;j++)
    {
        for(int i=0;i<nb;i++)      {mul=((__uint128_t)(pb[i]))*((__uint128_t)(pa[j-i]));rst1+=LO(mul);rst2+=HI(mul);}
        c->dataU64[j+2]=LO(rst1);
        rst1=rst2+HI(rst1);rst2=0;
    }
    for(;j<na+nb-1;j++)
    {
        for(int i=j-na+1;i<nb;i++) {mul=((__uint128_t)(pb[i]))*((__uint128_t)(pa[j-i]));rst1+=LO(mul);rst2+=HI(mul);}
        c->dataU64[j+2]=LO(rst1);
        rst1=rst2+HI(rst1);rst2=0;
    }
    c->dataU64[j+2]=rst1;
    c->num=j+2+(rst1!=0);
    if(buff1!=NULL) mFree(buff1);
    if(buff2!=NULL) mFree(buff2);
}
// na=5;nb=3;
// (0,0)(0,1)(0,2)(0,3)(0,4)
// (1,0)(1,1)(1,2)(1,3)(1,4)
// (2,0)(2,1)(2,2)(2,3)(2,4)

void m_LIntMul(MArray *a,MArray *b,MArray *c)
{
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    
    LIntAppend(c,a->num+b->num+3);
    SIGN(c)=(SIGN(a)!=SIGN(b));
         if(TYPE(a)==LInt_NUL) {                TYPE(c)=TYPE(b);DATA(c)=DATA(b);}
    else if(TYPE(b)==LInt_NUL) {                TYPE(c)=TYPE(a);DATA(c)=DATA(a);}
    else if(DATA(b) > DATA(a)) {LIntCaculate(b);TYPE(c)=TYPE(a);DATA(c)=DATA(a);}
    else                       {LIntCaculate(a);TYPE(c)=TYPE(b);DATA(c)=DATA(b);}
    
    if(b->num>a->num) LIntMul(b,a,c);
    else              LIntMul(a,b,c);
}

void LIntDivInt(MArray *a,__int128_t b,MArray *c,__int128_t *remainder)
{
    int i=a->num-1;
    __int128_t rst = (__int128_t)(a->dataU64[i]);
    c->dataU64[i]=(uint64_t)(rst/b); rst=rst%b;
    c->num = (c->dataU64[i]==0)?(a->num-1):a->num;
    for(i=i-1;i>=2;i--)
    {
        rst=(rst<<64)+(__int128_t)(a->dataU64[i]);
        c->dataU64[i]=(uint64_t)(rst/b);rst=rst%b;
    }
    if(remainder!=NULL) *remainder=rst;
}

void m_LIntDivInt(MArray *a,int b,MArray *c,int *remainder)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    LIntAppend(c,a->num+1);
    SIGN(c)=(SIGN(a)!=(b<0));
    if(b<0) b=0-b;
    
    if(remainder)
    {
        __int128_t r;
        if(TYPE(a)==LInt_MUL) LIntCaculate(a);
        if(TYPE(a)==LInt_NUL) LIntDivInt(a,b,c,&r);
        else
        {
            __int128_t data0=DATA(a);
            __int128_t data =data0*b;
            LIntDivInt(a,data,c,&r);
            r=(r+data)/data0%b;
        }
        *remainder=(SIGN(c))?(0-r):r;
        return;
    }
    
    if(TYPE(a)==LInt_MUL)
    {
        if((DATA(a)>b)&&(DATA(a)%b==0))
        {
            if(c!=a) {memcpy(c->dataU64+2,a->dataU64+2,(a->num-2)*sizeof(uint64_t));c->num=a->num;}
            DATA(c)=DATA(a)/(uint64_t)b;TYPE(c)=LInt_MUL; return;
        }
        if((b>DATA(a))&&(b%DATA(a)==0))
        {
            if(c!=a) {memcpy(c->dataU64+2,a->dataU64+2,(a->num-2)*sizeof(uint64_t));c->num=a->num;}
            DATA(c)=(uint64_t)b/DATA(a);TYPE(c)=LInt_DIV; return;
        }
        LIntCaculate(a);
    }
    else //if(TYPE(a)==LInt_DIV)
    {
        __int128_t data=(__int128_t)(DATA(a))*(__int128_t)b;
        if(data<0x0ffffffffffffffff) 
        {
            if(c!=a) {memcpy(c->dataU64+2,a->dataU64+2,(a->num-2)*sizeof(uint64_t));c->num=a->num;}
            DATA(c)=data;TYPE(c)=LInt_DIV;return;
        }
        LIntCaculate(a);
    }
    
    if(c!=a) {memcpy(c->dataU64+2,a->dataU64+2,(a->num-2)*sizeof(uint64_t));c->num=a->num;}
    TYPE(c)=LInt_DIV; DATA(c)=(uint64_t)b;return;
}

void m_LIntDiv(MArray *a,MArray *b,MArray *c,MArray *remainder)
{
    mException(INVALID_POINTER(a)||INVALID_POINTER(b),EXIT,"invalid input");
    if(c==NULL) c=a;
   
    LIntAppend(c,a->num+1);
    SIGN(c)=(SIGN(a)!=SIGN(b));
    if(remainder==NULL)
    {
             if(TYPE(a)==LInt_NUL) {TYPE(c)=TYPE(b);DATA(c)=DATA(b);}
        else if(TYPE(b)==LInt_NUL) {TYPE(c)=TYPE(a);DATA(c)=DATA(a);}
        else if(TYPE(a)!=TYPE(b))
        {
            __int128_t data=DATA(a)*DATA(b);
            if(data>0x0ffffffffffffffff)
            {
                if(DATA(b)>DATA(a)) {LIntCaculate(b);TYPE(c)=  TYPE(a);DATA(c)=DATA(a);}
                else                {LIntCaculate(a);TYPE(c)=7-TYPE(b);DATA(c)=DATA(b);}
            }
            else {TYPE(c)=TYPE(a);DATA(c)=data;}
        }
        else
        {
            if(DATA(a)>DATA(b)) 
            {
                if(DATA(a)%DATA(b)==0) {TYPE(c)=  TYPE(a);DATA(c)=DATA(a)/DATA(b);}
                else   {LIntCaculate(a);TYPE(c)=7-TYPE(b);DATA(c)=DATA(b);        }
            }
            else if(DATA(a)<DATA(b))
            {
                if(DATA(b)%DATA(a)==0) {TYPE(c)=7-TYPE(b);DATA(c)=DATA(b)/DATA(a);}
                else   {LIntCaculate(b);TYPE(c)=  TYPE(a);DATA(c)=DATA(a);        }
            }
            else {TYPE(c)=LInt_NUL;DATA(c)=1;}
        }
    }
    else 
    {
        LIntCaculate(a);LIntCaculate(b);
        LIntAppend(remainder,b->num);
        TYPE(remainder)=LInt_NUL;DATA(remainder)=1;
    }
    
    if(a->num<b->num) 
    {
        c->num=3;c->dataU64[2]=0;TYPE(c)=LInt_NUL;SIGN(c)=0;DATA(c)=1;
        if(remainder) memcpy(remainder->dataU64,a->dataU64,a->num*sizeof(uint64_t));
        return;
    }
    
    MArray *pc=c        ;if((c        ==a)||(c        ==b))                            c=LIntBuff3();
    MArray *pr=remainder;if((remainder==a)||(remainder==b)||(remainder==NULL)) remainder=LIntBuff4();
    mArrayRedefine(c        ,a->num,sizeof(uint64_t));
    mArrayRedefine(remainder,b->num,sizeof(uint64_t));
    
    int num_a=a->num;
    uint64_t *buff =mMalloc(num_a*sizeof(uint64_t));
    memcpy(buff,a->dataU64, num_a*sizeof(uint64_t));
        
    __uint128_t a0,b0;
    while(m_LIntCompare(a,b)>=0)
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
            remainder->dataU64[1]=(uint64_t)a0;
        }
        m_LIntAdd(c,remainder,c);
        m_LIntMul(b,remainder,remainder);
        m_LIntSub(a,remainder,a);
    }
    memcpy(remainder->dataU64,a->dataU64,a->num*sizeof(uint64_t));remainder->num=a->num;
    memcpy(a->dataU64,buff,num_a*sizeof(uint64_t));a->num=num_a;mFree(buff);
    DATA(c) = DATA(a)*DATA(b);
    DATA(remainder)=DATA(a);
    
    if(pc==a) mArrayDataExchange(a,c);
    if(pc==b) mArrayDataExchange(b,c);
    if(pr==a) mArrayDataExchange(a,remainder);
    if(pr==b) mArrayDataExchange(b,remainder);
}

void LIntCaculate(MArray *a)
{
    if(TYPE(a)==LInt_DIV) LIntDivInt(a,(uint64_t)DATA(a),a,NULL);
    else {LIntAppend(a,a->num+1);LIntMulInt(a,(uint64_t)DATA(a),a);}
    TYPE(a)=LInt_NUL;DATA(a)=1;
}

void mLIntToString(MArray *a,char *str)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
//     printf("aaaaaaaaaaaaaaaaaaaaaaaaaDATA(a)=%lu\n",DATA(a));
    LIntCaculate(a);
    
    int num=a->num;
    if(num   <=2) {str[0]='0';str[1]=0;return;}
    if(DATA(a)<0) {str[0]='-';str++;}
    
    MArray *buff = LIntBuff1(); mArrayRedefine(buff,num,sizeof(uint64_t));
    memcpy(buff->dataU64+2,a->dataU64+2,(num-2)*sizeof(uint64_t));
    
    __int128_t *prst = mMalloc(num*2*sizeof(__int128_t));
    int i=num+num-1;
    while(1)
    {
        LIntDivInt(buff,100000000000000000,buff,prst+i);i--;
        if(buff->num==2) break;
    }
    
    i=i+1;uint64_t rst=prst[i];sprintf(str,"%ld",rst);str+=strlen(str);
    for(i=i+1;i<num+num;i++) {rst=prst[i];sprintf(str,"%09d%08d",rst/100000000,rst%100000000);str+=17;}
    str[0]=0;
    
    mFree(prst);
}

void mStringToLInt(MArray *a,const char *str)
{
    mException(INVALID_POINTER(a),EXIT,"invalid input");
    
    int l = strlen(str);
    LIntAppend(a,l/10);SIGN(a)=0;TYPE(a)=LInt_NUL;DATA(a)=1;
    int sign=0;
    if(str[0]=='-') {str++;l--;sign=1;}
    while(str[0]=='0') {str++;l--;}
    
    char buff[10];buff[9]=0;
    memcpy(buff,str  ,9*sizeof(char));  int64_t b =   atoi(buff);
    memcpy(buff,str+9,9*sizeof(char)); b=b*1000000000+atoi(buff);
    a->dataU64[2]=b;a->num=3;
    
    int i=18;for(;i<=l-18;i+=18)
    {
        memcpy(buff,str+i  ,9*sizeof(char));             b=atoi(buff);
        memcpy(buff,str+i+9,9*sizeof(char));b=b*1000000000+atoi(buff);
          LIntMulInt(a,1000000000000000000,a);
        m_LIntAddInt(a,b,a);
    }
    if(i<l)
    {
        int64_t d=1;b=0;
        for(;i<l;i++){d=d*10;b=b*10+str[i]-'0';}
          LIntMulInt(a,d,a);
        m_LIntAddInt(a,b,a);
    }
    SIGN(a)=sign;
}

uint64_t morn_factorial[32]={0,1,2,3,8,15,48,105,384,945,3840,10395,46080,135135,645120,2027025,10321920,34459425,185794560,654729075,3715891200,13749310575,81749606400,316234143225,1961990553600,7905853580625,51011754393600,213458046676875,1428329123020800,6190283353629375,42849873690624000,191898783962510625};
void mFactorial(MArray *array,int n)
{
    int a[32];
    a[0]=n;
    int b=n;int c=1; 
    for(;b>31;c++) {b=b/2;a[c]=b;}
    
    int d=a[c-1];
    int e0=d-  (d&0x01);
    int e1=d-1+(d&0x01);
    MArray *x0=mArrayCreate(n/16,sizeof(uint64_t));x0->dataS64[0]=0;x0->dataS64[1]=1;x0->dataU64[2]=morn_factorial[e0];x0->num=3;
    MArray *x1=array;LIntAppend(x1,n/8);           x1->dataS64[0]=0;x1->dataS64[1]=1;x1->dataU64[2]=morn_factorial[e1];x1->num=3;


    int n2=0;
    for(int i=c-2;i>=0;i--)
    {
        n2=n2+d;d=a[i];
        int e=d-1+(d&0x01);
//         printf("a[i]=%d,n2=%d\n",a[i],n2);
        m_LIntMul(x1,x0,x0);
        for(int k=e1+2;k<=e;k+=2) m_LIntMulInt(x1,k,x1);
        e1=e;
    }
    
    m_LIntMul(x1,x0,array);
    mArrayRelease(x0);

    int s=n2/64;int s1=n2%64;int s2=64-s1;
//     printf("s=%d,s1=%d,s2=%d\n",s,s1,s2);
    if(s1==0)
    {
        LIntAppend(array,array->num+s);
        memmove(array->dataU64+2+s,array->dataU64+2,(array->num-2)*sizeof(uint64_t));
        memset( array->dataU64+2,0,s*sizeof(uint64_t));
        array->num=array->num+s;
        return;
    }
    
    LIntAppend(array,array->num+s+1);
    array->dataU64[array->num+s]=0;
    for(int i=array->num+s;i-s>2;i--)
    {
        array->dataU64[i  ]+=array->dataU64[i-s-1]>>s2;
        array->dataU64[i-1] =array->dataU64[i-s-1]<<s1;
    }
    memset(array->dataU64+2,0,s*sizeof(uint64_t));
    array->num=array->num+s+(array->dataU64[array->num+s]!=0);
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

void LIntCaculate(MArray *a,int32_t b,MArray *c,int type)
{
         if(type==1) m_LIntAddInt(a,b,c);
    else if(type==2) m_LIntSubInt(a,b,c);
    else if(type==3) m_LIntMulInt(a,b,c);
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
