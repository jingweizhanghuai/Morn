#include "morn_util.h"

int morn_endian=1;

struct Base64
{
    uint8_t a21:2;
    uint8_t a1:6;
    
    uint8_t a31:4;
    uint8_t a22:4;
    
    uint8_t a4:6;
    uint8_t a32:2;
};

char *morn_base64_encode="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
uint8_t  morn_base64_decode[128]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,0,0,0,63,52,53,54,55,56,57,58,59,60,61,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0,0,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,0,0,0,0,0};
void _Base64Encode(uint8_t *in,int insize,char *out,int *outsize)
{
    if(insize<0) insize=strlen((char *)in);
    uint8_t *input=NULL;if((uint8_t *)out==in) {input=mMalloc(insize);memcpy(input,in,insize);in=input;}
    
    int size=(insize+2)/3*4+1;
    if(outsize!=NULL)
    {
        if(*outsize>size-1) *outsize=size-1;
        else {insize=(*outsize)/4*3;size=*outsize;}
    }
    int n=0;
    uint8_t *pin=in;for(;pin<in+insize;pin+=3)
    {
        struct Base64 *p=(struct Base64 *)pin;
        int a1=p->a1;int a2=((p->a21)<<4)+p->a22;int a3=((p->a31)<<2)+p->a32;int a4=p->a4;
        out[n  ]=morn_base64_encode[a1];
        out[n+1]=morn_base64_encode[a2];
        out[n+2]=morn_base64_encode[a3];
        out[n+3]=morn_base64_encode[a4];
        n+=4;
    }
         if(in+insize+1==pin) out[n-1]='=';
    else if(in+insize+2==pin){out[n-1]='=';out[n-2]='=';}
    if(n<size) out[n]=0;
    if(input) mFree(input);
}
void m_Base64Encode(uint8_t *in,int insize,MString *out)
{
    if(insize<0) insize=strlen((char *)in);
    int size=(insize+2)/3*4+1;
    mObjectRedefine(out,NULL,size);
    _Base64Encode(in,insize,out->string,NULL);
}
void mornBase64Encode(MString *str)
{
    int insize=str->size;
    uint8_t *data=mMalloc(insize);
    memcpy(data,str->string,insize);
    int outsize=(insize+2)/3*4+1;
    mObjectRedefine(str,NULL,outsize);
    _Base64Encode(data,insize,str->string,NULL);
    mFree(data);
}

void _Base64Decode(uint8_t *in,int insize,uint8_t *out,int *outsize)
{
    if(insize<0) {insize=strlen((char *)in);} insize=insize/4*4;
    int size=insize/4*3+1;
    
    if(outsize!=NULL)
    {
        if(in==out) *outsize=insize;
        size=size-(in[insize-1]=='=')-(in[insize-2]=='=');
        if(*outsize>size-1) *outsize=size-1;
        else {insize=(*outsize)/3*4;size=*outsize;}
    }
    int n=0;
    uint8_t *pin=in;for(;pin<in+insize;pin+=4)
    {
        struct Base64 *p = (struct Base64 *)(out+n);
        uint8_t a1=morn_base64_decode[pin[0]];
        uint8_t a2=morn_base64_decode[pin[1]];
        uint8_t a3=morn_base64_decode[pin[2]];
        uint8_t a4=morn_base64_decode[pin[3]];
        p->a1=a1;p->a21=a2>>4;p->a22=a2&0x0f;p->a31=a3>>2;p->a32=a3&0x03;p->a4=a4;
        n+=3;
    }
    if(pin[-1]=='=') n--;
    if(pin[-2]=='=') n--;
    if(n<size) out[n]=0;
}
void m_Base64Decode(uint8_t *in,int insize,MString *out)
{
    if(insize<0) insize=strlen((char *)in);
    int size=insize/4*3+1;
    mObjectRedefine(out,NULL,size);
    _Base64Decode(in,insize,out->dataU8,NULL);
}
void mornBase64Decode(MString *str)
{
    int insize=str->size;
    uint8_t *data=mMalloc(insize);
    memcpy(data,str->string,insize);
    int outsize=insize/4*3+1;
    mObjectRedefine(str,NULL,outsize);
    _Base64Decode(data,insize,str->dataU8,NULL);
    mFree(data);
}

struct Base32
{
    uint8_t a21:3;
    uint8_t a1:5;
    
    uint8_t a41:1;
    uint8_t a3:5;
    uint8_t a22:2;
    
    uint8_t a51:4;
    uint8_t a42:4;
    
    uint8_t a71:2;
    uint8_t a6:5;
    uint8_t a52:1;
    
    uint8_t a8:5;
    uint8_t a72:3;
};
char *morn_base32_encode="ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
uint8_t  morn_base32_decode[96]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,27,28,29,30,31,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,0,0,0,0,0};
void _Base32Encode(uint8_t *in,int insize,char *out,int *outsize)
{
    if(insize<0) insize=strlen((char *)in);
    uint8_t *input=NULL;if((uint8_t *)out==in) {input=mMalloc(insize);memcpy(input,in,insize);in=input;}
    
    int size=(insize+4)/5*8+1;
    if(outsize!=NULL)
    {
        if(*outsize>size-1) *outsize=size-1;
        else {insize=(*outsize)/8*5;size=*outsize;}
    }
    int n=0;
    uint8_t *pin=in;for(;pin<in+insize;pin+=5)
    {
        struct Base32 *p=(struct Base32 *)pin;
        int a1=p->a1;int a2=((p->a21)<<2)+p->a22;int a3=p->a3;int a4=((p->a41)<<4)+p->a42;
        int a5=((p->a51)<<1)+p->a52;int a6=p->a6;int a7=((p->a71)<<3)+p->a72;int a8=p->a8;
        
        out[n  ]=morn_base32_encode[a1];
        out[n+1]=morn_base32_encode[a2];
        out[n+2]=morn_base32_encode[a3];
        out[n+3]=morn_base32_encode[a4];
        out[n+4]=morn_base32_encode[a5];
        out[n+5]=morn_base32_encode[a6];
        out[n+6]=morn_base32_encode[a7];
        out[n+7]=morn_base32_encode[a8];
        n+=8;
    }
         if(in+insize+1==pin) out[n-1]='=';
    else if(in+insize+2==pin){out[n-1]='=';out[n-2]='=';out[n-3]='=';}
    else if(in+insize+3==pin){out[n-1]='=';out[n-2]='=';out[n-3]='=';out[n-4]='=';}
    else if(in+insize+4==pin){out[n-1]='=';out[n-2]='=';out[n-3]='=';out[n-4]='=';out[n-5]='=';out[n-6]='=';}
    if(n<size) out[n]=0;
    if(input) mFree(input);
}
void m_Base32Encode(uint8_t *in,int insize,MString *out)
{
    if(insize<0) insize=strlen((char *)in);
    int size=(insize+4)/5*8+1;
    mObjectRedefine(out,NULL,size);
    _Base32Encode(in,insize,out->string,NULL);
}
void mornBase32Encode(MString *str)
{
    int insize=str->size;
    uint8_t *data=mMalloc(insize);
    memcpy(data,str->string,insize);
    int outsize=(insize+4)/5*8+1;
    mObjectRedefine(str,NULL,outsize);
    _Base32Encode(data,insize,str->string,NULL);
    mFree(data);
}

void _Base32Decode(uint8_t *in,int insize,uint8_t *out,int *outsize)
{
    if(insize<0) {insize=strlen((char *)in);} insize=insize/8*8;
    int size=insize/8*5+1;
    
    if(outsize!=NULL)
    {
        if(in==out) *outsize=insize;
        size=size-(in[insize-1]=='=')-(in[insize-2]=='=')-(in[insize-4]=='=')-(in[insize-5]=='=');
        if(*outsize>size-1) *outsize=size-1;
        else {insize=(*outsize)/5*8;size=*outsize;}
    }
    int n=0;
    uint8_t *pin=in;for(;pin<in+insize;pin+=8)
    {
        struct Base32 *p = (struct Base32 *)(out+n);
        uint8_t a1=morn_base64_decode[pin[0]];
        uint8_t a2=morn_base64_decode[pin[1]];
        uint8_t a3=morn_base64_decode[pin[2]];
        uint8_t a4=morn_base64_decode[pin[3]];
        uint8_t a5=morn_base64_decode[pin[4]];
        uint8_t a6=morn_base64_decode[pin[5]];
        uint8_t a7=morn_base64_decode[pin[6]];
        uint8_t a8=morn_base64_decode[pin[7]];
        
        p->a1=a1;p->a21=a2>>2;p->a22=a2&0x03;p->a3=a3;p->a41=a4>>4;p->a42=a4&0x0f;
        p->a51=a5>>1;p->a52=a5&0x01;p->a6=a6;p->a71=a7>>3;p->a72=a7&0x07;p->a8=a8;
        n+=5;
    }
    if(pin[-1]=='=') n--;
    if(pin[-2]=='=') n--;
    if(pin[-4]=='=') n--;
    if(pin[-5]=='=') n--;
    printf("n=%d\n",n);
    if(n<size) out[n]=0;
}
void m_Base32Decode(uint8_t *in,int insize,MString *out)
{
    if(insize<0) insize=strlen((char *)in);
    int size=insize/8*5+1;
    mObjectRedefine(out,NULL,size);
    _Base32Decode(in,insize,out->dataU8,NULL);
}
void mornBase32Decode(MString *str)
{
    int insize=str->size;
    uint8_t *data=mMalloc(insize);
    memcpy(data,str->string,insize);
    int outsize=insize/8*5+1;
    mObjectRedefine(str,NULL,outsize);
    _Base32Decode(data,insize,str->dataU8,NULL);
    mFree(data);
}

uint16_t morn_base16_encode1[256]={0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,0x3630,0x3730,0x3830,0x3930,0x6130,0x6230,0x6330,0x6430,0x6530,0x6630,0x3031,0x3131,0x3231,0x3331,0x3431,0x3531,0x3631,0x3731,0x3831,0x3931,0x6131,0x6231,0x6331,0x6431,0x6531,0x6631,0x3032,0x3132,0x3232,0x3332,0x3432,0x3532,0x3632,0x3732,0x3832,0x3932,0x6132,0x6232,0x6332,0x6432,0x6532,0x6632,0x3033,0x3133,0x3233,0x3333,0x3433,0x3533,0x3633,0x3733,0x3833,0x3933,0x6133,0x6233,0x6333,0x6433,0x6533,0x6633,0x3034,0x3134,0x3234,0x3334,0x3434,0x3534,0x3634,0x3734,0x3834,0x3934,0x6134,0x6234,0x6334,0x6434,0x6534,0x6634,0x3035,0x3135,0x3235,0x3335,0x3435,0x3535,0x3635,0x3735,0x3835,0x3935,0x6135,0x6235,0x6335,0x6435,0x6535,0x6635,0x3036,0x3136,0x3236,0x3336,0x3436,0x3536,0x3636,0x3736,0x3836,0x3936,0x6136,0x6236,0x6336,0x6436,0x6536,0x6636,0x3037,0x3137,0x3237,0x3337,0x3437,0x3537,0x3637,0x3737,0x3837,0x3937,0x6137,0x6237,0x6337,0x6437,0x6537,0x6637,0x3038,0x3138,0x3238,0x3338,0x3438,0x3538,0x3638,0x3738,0x3838,0x3938,0x6138,0x6238,0x6338,0x6438,0x6538,0x6638,0x3039,0x3139,0x3239,0x3339,0x3439,0x3539,0x3639,0x3739,0x3839,0x3939,0x6139,0x6239,0x6339,0x6439,0x6539,0x6639,0x3061,0x3161,0x3261,0x3361,0x3461,0x3561,0x3661,0x3761,0x3861,0x3961,0x6161,0x6261,0x6361,0x6461,0x6561,0x6661,0x3062,0x3162,0x3262,0x3362,0x3462,0x3562,0x3662,0x3762,0x3862,0x3962,0x6162,0x6262,0x6362,0x6462,0x6562,0x6662,0x3063,0x3163,0x3263,0x3363,0x3463,0x3563,0x3663,0x3763,0x3863,0x3963,0x6163,0x6263,0x6363,0x6463,0x6563,0x6663,0x3064,0x3164,0x3264,0x3364,0x3464,0x3564,0x3664,0x3764,0x3864,0x3964,0x6164,0x6264,0x6364,0x6464,0x6564,0x6664,0x3065,0x3165,0x3265,0x3365,0x3465,0x3565,0x3665,0x3765,0x3865,0x3965,0x6165,0x6265,0x6365,0x6465,0x6565,0x6665,0x3066,0x3166,0x3266,0x3366,0x3466,0x3566,0x3666,0x3766,0x3866,0x3966,0x6166,0x6266,0x6366,0x6466,0x6566,0x6666};
uint16_t morn_base16_encode2[256]={0x3030,0x3031,0x3032,0x3033,0x3034,0x3035,0x3036,0x3037,0x3038,0x3039,0x3061,0x3062,0x3063,0x3064,0x3065,0x3066,0x3130,0x3131,0x3132,0x3133,0x3134,0x3135,0x3136,0x3137,0x3138,0x3139,0x3161,0x3162,0x3163,0x3164,0x3165,0x3166,0x3230,0x3231,0x3232,0x3233,0x3234,0x3235,0x3236,0x3237,0x3238,0x3239,0x3261,0x3262,0x3263,0x3264,0x3265,0x3266,0x3330,0x3331,0x3332,0x3333,0x3334,0x3335,0x3336,0x3337,0x3338,0x3339,0x3361,0x3362,0x3363,0x3364,0x3365,0x3366,0x3430,0x3431,0x3432,0x3433,0x3434,0x3435,0x3436,0x3437,0x3438,0x3439,0x3461,0x3462,0x3463,0x3464,0x3465,0x3466,0x3530,0x3531,0x3532,0x3533,0x3534,0x3535,0x3536,0x3537,0x3538,0x3539,0x3561,0x3562,0x3563,0x3564,0x3565,0x3566,0x3630,0x3631,0x3632,0x3633,0x3634,0x3635,0x3636,0x3637,0x3638,0x3639,0x3661,0x3662,0x3663,0x3664,0x3665,0x3666,0x3730,0x3731,0x3732,0x3733,0x3734,0x3735,0x3736,0x3737,0x3738,0x3739,0x3761,0x3762,0x3763,0x3764,0x3765,0x3766,0x3830,0x3831,0x3832,0x3833,0x3834,0x3835,0x3836,0x3837,0x3838,0x3839,0x3861,0x3862,0x3863,0x3864,0x3865,0x3866,0x3930,0x3931,0x3932,0x3933,0x3934,0x3935,0x3936,0x3937,0x3938,0x3939,0x3961,0x3962,0x3963,0x3964,0x3965,0x3966,0x6130,0x6131,0x6132,0x6133,0x6134,0x6135,0x6136,0x6137,0x6138,0x6139,0x6161,0x6162,0x6163,0x6164,0x6165,0x6166,0x6230,0x6231,0x6232,0x6233,0x6234,0x6235,0x6236,0x6237,0x6238,0x6239,0x6261,0x6262,0x6263,0x6264,0x6265,0x6266,0x6330,0x6331,0x6332,0x6333,0x6334,0x6335,0x6336,0x6337,0x6338,0x6339,0x6361,0x6362,0x6363,0x6364,0x6365,0x6366,0x6430,0x6431,0x6432,0x6433,0x6434,0x6435,0x6436,0x6437,0x6438,0x6439,0x6461,0x6462,0x6463,0x6464,0x6465,0x6466,0x6530,0x6531,0x6532,0x6533,0x6534,0x6535,0x6536,0x6537,0x6538,0x6539,0x6561,0x6562,0x6563,0x6564,0x6565,0x6566,0x6630,0x6631,0x6632,0x6633,0x6634,0x6635,0x6636,0x6637,0x6638,0x6639,0x6661,0x6662,0x6663,0x6664,0x6665,0x6666};
void _Base16Encode(uint8_t *in,int insize,char *out,int *outsize)
{
    if(insize<0) insize=strlen((char *)in);
    uint8_t *input=NULL;if((uint8_t *)out==in) {input=mMalloc(insize);memcpy(input,in,insize);in=input;}
    
    int size=insize+insize+1;
    if(outsize!=NULL)
    {
        if(*outsize>size-1) *outsize=size-1;
        else {insize=*outsize/2;size=*outsize;}
    }
    uint16_t *encode=(mEndian()==LITTLE_ENDIAN)?morn_base16_encode1:morn_base16_encode2;
    uint16_t *p= (uint16_t *)out;
    int i=0;for(;i<insize-15;i+=16)
    {
        p[i   ]=encode[in[i   ]];p[i+ 1]=encode[in[i+ 1]];p[i+ 2]=encode[in[i+ 2]];p[i+ 3]=encode[in[i+ 3]];
        p[i+ 4]=encode[in[i+ 4]];p[i+ 5]=encode[in[i+ 5]];p[i+ 6]=encode[in[i+ 6]];p[i+ 7]=encode[in[i+ 7]];
        p[i+ 8]=encode[in[i+ 8]];p[i+ 9]=encode[in[i+ 9]];p[i+10]=encode[in[i+10]];p[i+11]=encode[in[i+11]];
        p[i+12]=encode[in[i+12]];p[i+13]=encode[in[i+13]];p[i+14]=encode[in[i+14]];p[i+15]=encode[in[i+15]];
    }
    for(;i<insize;i++) p[i]=encode[in[i]];
    if(insize+insize<size) out[insize+insize]=0;
    if(input) mFree(input);
}
void m_Base16Encode(uint8_t *in,int insize,MString *out)
{
    if(insize<0) insize=strlen((char *)in);
    int size=insize+insize+1;
    mObjectRedefine(out,NULL,size);
    _Base16Encode(in,insize,out->string,NULL);
}
void mornBase16Encode(MString *str)
{
    int insize=str->size;
    uint8_t *data=mMalloc(insize);
    memcpy(data,str->string,insize);
    int outsize=insize+insize+1;
    mObjectRedefine(str,NULL,outsize);
    _Base16Encode(data,insize,str->string,NULL);
    mFree(data);
}

uint8_t morn_base16_decode[72]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15,0};
void _Base16Decode(uint8_t *in,int insize,uint8_t *out,int *outsize)
{
    if(insize<0) insize=strlen((char *)in);
    int size=insize/2+1;
    
    if(outsize!=NULL)
    {
        if(*outsize>size-1) *outsize=size-1;
        else {insize=*outsize*2;size=*outsize;}
    }
    int n=insize>>1;
    int i=0;for(;i<n;i+=8)
    {
        out[i  ]=(morn_base16_decode[in[i+i   ]]<<4)+morn_base16_decode[in[i+i+ 1]];
        out[i+1]=(morn_base16_decode[in[i+i+ 2]]<<4)+morn_base16_decode[in[i+i+ 3]];
        out[i+2]=(morn_base16_decode[in[i+i+ 4]]<<4)+morn_base16_decode[in[i+i+ 5]];
        out[i+3]=(morn_base16_decode[in[i+i+ 6]]<<4)+morn_base16_decode[in[i+i+ 7]];
        out[i+4]=(morn_base16_decode[in[i+i+ 8]]<<4)+morn_base16_decode[in[i+i+ 9]];
        out[i+5]=(morn_base16_decode[in[i+i+10]]<<4)+morn_base16_decode[in[i+i+11]];
        out[i+6]=(morn_base16_decode[in[i+i+12]]<<4)+morn_base16_decode[in[i+i+13]];
        out[i+7]=(morn_base16_decode[in[i+i+14]]<<4)+morn_base16_decode[in[i+i+15]];
    }
    for(;i<n;i++) out[i]=(morn_base16_decode[in[i+i]]<<4)+morn_base16_decode[in[i+i+1]];
    if(n<size) out[n]=0;
}
void m_Base16Decode(uint8_t *in,int insize,MString *out)
{
    if(insize<0) insize=strlen((char *)in);
    int size=insize/2+1;
    mObjectRedefine(out,NULL,size);
    _Base16Decode(in,insize,out->dataU8,NULL);
}
void mornBase16Decode(MString *str)
{
    int insize=str->size;
    uint8_t *data=mMalloc(insize);
    memcpy(data,str->string,insize);
    int outsize=insize/2+1;
    mObjectRedefine(str,NULL,outsize);
    _Base16Decode(data,insize,str->dataU8,NULL);
    mFree(data);
}

struct HandleBaseCodec
{
    int type;
};
void endBaseCodec(struct HandleBaseCodec *handle) {NULL;}
#define HASH_BaseCodec 0x1a259306
struct HandleBaseCodec *morn_base_codec_handle=NULL;
struct HandleBaseCodec *BaseCodecInit()
{
    if(morn_base_codec_handle!=NULL) return morn_base_codec_handle;
    MHandle *hdl=mHandle("BaseCodec",BaseCodec);
    struct HandleBaseCodec *handle=hdl->handle;
    if(!mHandleValid(hdl))
    {
        handle->type=64;
        mPropertyFunction("BaseCodec","exit",mornObjectRemove,"BaseCodec");
        mPropertyVariate( "BaseCodec","type",&(handle->type),sizeof(int));
        morn_base_codec_handle = handle;
        hdl->valid=1;
    }
    return handle;
}
void _BaseEncode(uint8_t *in,int insize,char *out,int *outsize)
{
    struct HandleBaseCodec *handle=BaseCodecInit();
         if(handle->type==64) _Base64Encode(in,insize,out,outsize);
    else if(handle->type==32) _Base32Encode(in,insize,out,outsize);
    else if(handle->type==16) _Base16Encode(in,insize,out,outsize);
}

void m_BaseEncode(uint8_t *in,int insize,MString *out)
{
    struct HandleBaseCodec *handle=BaseCodecInit();
         if(handle->type==64) m_Base64Encode(in,insize,out);
    else if(handle->type==32) m_Base32Encode(in,insize,out);
    else if(handle->type==16) m_Base16Encode(in,insize,out);
}

void mornBaseEncode(MString *str)
{
    struct HandleBaseCodec *handle=BaseCodecInit();
         if(handle->type==64) mornBase64Encode(str);
    else if(handle->type==32) mornBase32Encode(str);
    else if(handle->type==16) mornBase16Encode(str);
}

void _BaseDecode(uint8_t *in,int insize,uint8_t *out,int *outsize)
{
    struct HandleBaseCodec *handle=BaseCodecInit();
         if(handle->type==64) _Base64Decode(in,insize,out,outsize);
    else if(handle->type==32) _Base32Decode(in,insize,out,outsize);
    else if(handle->type==16) _Base16Decode(in,insize,out,outsize);
}

void m_BaseDecode(uint8_t *in,int insize,MString *out)
{
    struct HandleBaseCodec *handle=BaseCodecInit();
         if(handle->type==64) m_Base64Decode(in,insize,out);
    else if(handle->type==32) m_Base32Decode(in,insize,out);
    else if(handle->type==16) m_Base16Decode(in,insize,out);
}

void mornBaseDecode(MString *str)
{
    struct HandleBaseCodec *handle=BaseCodecInit();
         if(handle->type==64) mornBase64Decode(str);
    else if(handle->type==32) mornBase32Decode(str);
    else if(handle->type==16) mornBase16Decode(str);
}

