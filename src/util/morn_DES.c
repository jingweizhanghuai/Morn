/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "morn_util.h"
#define fread(Data,Size,Num,Fl) mException(((int)fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error")

uint8_t key_map1[56] = {
    57,49,41,33,25,17, 9,
     1,58,50,42,34,26,18,
    10, 2,59,51,43,35,27,
    19,11, 3,60,52,44,36,
    63,55,47,39,31,23,15,
     7,62,54,46,38,30,22,
    14, 6,61,53,45,37,29,
    21,13, 5,28,20,12, 4};
    
uint8_t key_map2[48] = {
    14,17,11,24, 1, 5,
     3,28,15, 6,21,10,
    23,19,12, 4,26, 8,
    16, 7,27,20,13, 2,
    41,52,31,37,47,55,
    30,40,51,45,33,48,
    44,49,39,56,34,53,
    46,42,50,36,29,32};

uint8_t in_map[64] = {
    58,50,42,34,26,18,10,2,
    60,52,44,36,28,20,12,4,
    62,54,46,38,30,22,14,6,
    64,56,48,40,32,24,16,8,
    57,49,41,33,25,17, 9,1,
    59,51,43,35,27,19,11,3,
    61,53,45,37,29,21,13,5,
    63,55,47,39,31,23,15,7};
    
uint8_t des_e[48] = {
    32, 1, 2, 3, 4, 5,
     4, 5, 6, 7, 8, 9,
     8, 9,10,11,12,13,
    12,13,14,15,16,17,
    16,17,18,19,20,21,
    20,21,22,23,24,25,
    24,25,26,27,28,29,
    28,29,30,31,32, 1};
    
uint8_t des_s1[64] = {14,0,4,15,13,7,1,4,2,14,15,2,11,13,8,1,3,10,10,6,6,12,12,11,5,9,9,5,0,3,7,8,4,15,1,12,14,8,8,2,13,4,6,9,2,1,11,7,15,5,12,11,9,3,7,14,3,10,10,0,5,6,0,13};
uint8_t des_s2[64] = {15,3,1,13,8,4,14,7,6,15,11,2,3,8,4,14,9,12,7,0,2,1,13,10,12,6,0,9,5,11,10,5,0,13,14,8,7,10,11,1,10,3,4,15,13,4,1,2,5,11,8,6,12,7,6,12,9,0,3,5,2,14,15,9};
uint8_t des_s3[64] = {10,13,0,7,9,0,14,9,6,3,3,4,15,6,5,10,1,2,13,8,12,5,7,14,11,12,4,11,2,15,8,1,13,1,6,10,4,13,9,0,8,6,15,9,3,8,0,7,11,4,1,15,2,14,12,3,5,11,10,5,14,2,7,12};
uint8_t des_s4[64] = {7,13,13,8,14,11,3,5,0,6,6,15,9,0,10,3,1,4,2,7,8,2,5,12,11,1,12,10,4,14,15,9,10,3,6,15,9,0,0,6,12,10,11,1,7,13,13,8,15,9,1,4,3,5,14,11,5,12,2,7,8,2,4,14};
uint8_t des_s5[64] = {2,14,12,11,4,2,1,12,7,4,10,7,11,13,6,1,8,5,5,0,3,15,15,10,13,3,0,9,14,8,9,6,4,11,2,8,1,12,11,7,10,1,13,14,7,2,8,13,15,6,9,15,12,0,5,9,6,10,3,4,0,5,14,3};
uint8_t des_s6[64] = {12,10,1,15,10,4,15,2,9,7,2,12,6,9,8,5,0,6,13,1,3,13,4,14,14,0,7,11,5,3,11,8,9,4,14,3,15,2,5,12,2,9,8,5,12,15,3,10,7,11,0,14,4,1,10,7,1,6,13,0,11,8,6,13};
uint8_t des_s7[64] = {4,13,11,0,2,11,14,7,15,4,0,9,8,1,13,10,3,14,12,3,9,5,7,12,5,2,10,15,6,8,1,6,1,6,4,11,11,13,13,8,12,1,3,4,7,10,14,7,10,9,15,5,6,0,8,15,0,14,5,2,9,3,2,12};
uint8_t des_s8[64] = {13,1,2,15,8,13,4,8,6,10,15,3,11,7,1,4,10,12,9,5,3,6,14,11,5,0,0,14,12,9,7,2,7,2,11,1,4,14,1,7,9,4,12,10,14,8,2,13,0,15,6,12,10,9,13,0,15,3,3,5,5,6,8,11};

uint8_t des_p[32] = {16, 7,20,21,29,12,28,17,1,15,23,26, 5,18,31,10,2, 8,24,14,32,27, 3, 9,19,13,30, 6,22,11, 4,25};

uint8_t des_out[64] = {
    40,8,48,16,56,24,64,32,
    39,7,47,15,55,23,63,31,
    38,6,46,14,54,22,62,30,
    37,5,45,13,53,21,61,29,
    36,4,44,12,52,20,60,28,
    35,3,43,11,51,19,59,27,
    34,2,42,10,50,18,58,26,
    33,1,41, 9,49,17,57,25};

void PrintBit(uint64_t data,int bit_num,int n)
{
    int i;
    for(i=0;i<bit_num;i++)
    {
        if(i%n==0) printf(" ");
        printf("%d",(int)((data>>(63-i))&0x01));
    }
    printf("\n");
}

uint64_t DesTransform(uint64_t in,uint8_t *map,int bit_num)
{
    uint64_t out = 0;
    int i;
    for(i=0;i<bit_num;i++)
        out += (((in>>(64-map[i]))&0x01)<<(63-i));
    
    return out;
}

void DesKey(uint64_t key,uint64_t *sub_key)
{
    // PrintBit(key,64,8);
    
    uint64_t buff = DesTransform(key,key_map1,56);
    
    // PrintBit(buff,56,7);
    
    uint64_t c = buff&0xFFFFFFF000000000;
    uint64_t d = buff<<28;
    // PrintBit(c,28,7);PrintBit(d,28,7);
    
    #define KEY_SHIFT(in,n) ((((in)<<(n))+((in)>>(28-(n))))&0xFFFFFFF000000000)
    c=KEY_SHIFT(c,1);d=KEY_SHIFT(d,1);buff=c+(d>>28);sub_key[ 0]=DesTransform(buff,key_map2,48);
    // PrintBit(c,28,7);PrintBit(d,28,7);PrintBit(buff,56,7);PrintBit(sub_key[ 0],48,6);
    c=KEY_SHIFT(c,1);d=KEY_SHIFT(d,1);buff=c+(d>>28);sub_key[ 1]=DesTransform(buff,key_map2,48);
    // PrintBit(c,28,7);PrintBit(d,28,7);PrintBit(buff,56,7);PrintBit(sub_key[ 1],48,6);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[ 2]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[ 3]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[ 4]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[ 5]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[ 6]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[ 7]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,1);d=KEY_SHIFT(d,1);buff=c+(d>>28);sub_key[ 8]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[ 9]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[10]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[11]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[12]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[13]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,2);d=KEY_SHIFT(d,2);buff=c+(d>>28);sub_key[14]=DesTransform(buff,key_map2,48);
    c=KEY_SHIFT(c,1);d=KEY_SHIFT(d,1);buff=c+(d>>28);sub_key[15]=DesTransform(buff,key_map2,48);
    // PrintBit(c,28,7);PrintBit(d,28,7);PrintBit(buff,56,7);PrintBit(sub_key[15],48,6);
    
}

uint64_t DesSBox(uint64_t in)
{
    uint64_t out=0;
    uint64_t data;
    data=des_s1[(in>>58)&0x3F];out+=data<<60;
    data=des_s2[(in>>52)&0x3F];out+=data<<56;
    data=des_s3[(in>>46)&0x3F];out+=data<<52;
    data=des_s4[(in>>40)&0x3F];out+=data<<48;
    data=des_s5[(in>>34)&0x3F];out+=data<<44;
    data=des_s6[(in>>28)&0x3F];out+=data<<40;
    data=des_s7[(in>>22)&0x3F];out+=data<<36;
    data=des_s8[(in>>16)&0x3F];out+=data<<32;
    
    return out;
}

void ToBigEndian(uint8_t *in,uint8_t *out,int num)
{
    int i;
    for(i=0;i<num;i++)
        out[i] = in[num-1-i];
}

uint64_t DesEncrypt(uint64_t in,uint64_t key)
{
    uint64_t sub_key[16];
    DesKey(key,sub_key);
    // printf("ddddddddddddd\n");
    
    // PrintBit(key,64,8);
    // for(int i=0;i<16;i++)
        // PrintBit(sub_key[i],48,6);
    
    // printf("%llx\n",in);
    // PrintBit(in,64,8);
    uint64_t data_t = DesTransform(in,in_map,64);
    // PrintBit(data_t,64,8);
    
    
    uint64_t l = data_t&0xFFFFFFFF00000000;
    uint64_t r = data_t<<32;
    uint64_t s;
    int i;
    for(i=0;i<16;i++)
    {
        // printf("%d:\n",i);
        // printf("l is:");PrintBit(l,32,8);
        // printf("r is:");PrintBit(r,32,8);
        s=r;
        r=DesTransform(r,des_e,48);
        // printf("e is:");PrintBit(r,48,6);
        // printf("k is:");PrintBit(sub_key[i],48,6);
        r=r^sub_key[i];
        // printf("q is:");PrintBit(r,48,6);
        r=DesSBox(r);
        // printf("s is:");PrintBit(r,32,4);
        
        r=DesTransform(r,des_p,32);
        // printf("p is:");PrintBit(r,32,8);
        
        r=r^l;
        // printf("c is:");PrintBit(r,32,8);
        l=s;
    }
    // printf("l is:");PrintBit(l,32,8);
    // printf("r is:");PrintBit(r,32,8);
    
    data_t = r+(l>>32);
    // PrintBit(data_t,64,8);
    
    uint64_t out = DesTransform(data_t,des_out,64);
    return out;
}

uint64_t DesDecrypt(uint64_t in,uint64_t key)
{
    uint64_t sub_key[16];
    DesKey(key,sub_key);
    // printf("ddddddddddddd\n");
    
    // PrintBit(key,64,8);
    // for(int i=0;i<16;i++)
        // PrintBit(sub_key[i],48,6);
    
    // printf("%llx\n",in);
    // PrintBit(in,64,8);
    uint64_t data_t=0;
    
    data_t = DesTransform(in,in_map,64);
    // PrintBit(data_t,64,8);
    
    
    uint64_t l = data_t&0xFFFFFFFF00000000;
    uint64_t r = data_t<<32;
    uint64_t s;
    int i;
    for(i=0;i<16;i++)
    {
        // printf("%d:\n",i);
        // printf("l is:");PrintBit(l,32,8);
        // printf("r is:");PrintBit(r,32,8);
        s=r;
        r=DesTransform(r,des_e,48);
        // printf("e is:");PrintBit(r,48,6);
        // printf("k is:");PrintBit(sub_key[15-i],48,6);
        r=r^sub_key[15-i];
        // printf("q is:");PrintBit(r,48,6);
        r=DesSBox(r);
        // printf("s is:");PrintBit(r,32,4);
        
        r=DesTransform(r,des_p,32);
        // printf("p is:");PrintBit(r,32,8);
        
        r=r^l;
        // printf("c is:");PrintBit(r,32,8);
        l=s;
    }
    // printf("l is:");PrintBit(l,32,8);
    // printf("r is:");PrintBit(r,32,8);
    
    data_t = r+(l>>32);
    // PrintBit(data_t,64,8);
    
    uint64_t out = DesTransform(data_t,des_out,64);
    return out;
}

#ifndef DESKEY
#define DESKEY MORN_DESKEY
#endif

void mEncrypt(const char *in_name,const char *out_name,uint64_t key)
{
    int i;
    if((key==(uint64_t)DFLT)||(key==0)) key = MORN_DESKEY;
    
    FILE *fr = fopen( in_name,"rb");mException((fr==NULL),EXIT,"cannot open file %s\n", in_name);
    fseek(fr,0,SEEK_END);int file_size = ftell(fr);fseek(fr,0,SEEK_SET);
    int size = (file_size>>3);mException((size==0),EXIT,"invalid file size");
    
    uint64_t *data_in = (uint64_t *)mMalloc((size+1)*sizeof(uint64_t));
    uint64_t *data_out= (uint64_t *)mMalloc((size+1)*sizeof(uint64_t));
    fread(data_in,file_size,1,fr);fclose(fr);
    
    #pragma omp parallel for
    for(i=0;i<size;i++)
        data_out[i] = DesEncrypt(data_in[i],key);
    data_out[size]=data_in[size];
    
    FILE *fw = fopen(out_name,"wb");mException((fw==NULL),EXIT,"cannot open file %s\n",out_name);
    fwrite(data_out,file_size,1,fw);fclose(fw);
}

void mDecrypt(const char *in_name,const char *out_name,uint64_t key)
{
    int i;
    if((key==(uint64_t)DFLT)||(key==0)) key = MORN_DESKEY;
    
    FILE *fr = fopen( in_name,"rb");mException((fr==NULL),EXIT,"cannot open file %s\n", in_name);
    fseek(fr,0,SEEK_END);int file_size = ftell(fr);fseek(fr,0,SEEK_SET);
    int size = (file_size>>3);mException((size==0),EXIT,"invalid file size");
    
    uint64_t *data_in = (uint64_t *)mMalloc((size+1)*sizeof(uint64_t));
    uint64_t *data_out= (uint64_t *)mMalloc((size+1)*sizeof(uint64_t));
    fread(data_in,file_size,1,fr);fclose(fr);
    
    #pragma omp parallel for
    for(i=0;i<size;i++)
        data_out[i] = DesDecrypt(data_in[i],key);
    data_out[size]=data_in[size];
    
    FILE *fw = fopen(out_name,"wb");mException((fw==NULL),EXIT,"cannot open file %s\n",out_name);
    fwrite(data_out,file_size,1,fw);fclose(fw);
}

struct HandleFileDecrypt
{
    char name_out[256];
};
void endFileDecrypt(void *info)
{
    struct HandleFileDecrypt *handle = (struct HandleFileDecrypt *)info;
    remove(handle->name_out);
}
#define HASH_FileDecrypt 0x528629ba
void mFileDecrypt(MFile *file,uint64_t key)
{
    MHandle *hdl=mHandle(file,FileDecrypt);
    struct HandleFileDecrypt *handle = (struct HandleFileDecrypt *)(hdl->handle);
    if(hdl->valid == 1) return;
   
    char name_in[256];strcpy(name_in,file->filename);
    
    char name[32];sprintf(handle->name_out,"./%stmp",tmpnam(name));
    mObjectRedefine(file,handle->name_out);hdl->valid = 1;
    
    mDecrypt(name_in,handle->name_out,key);
}

struct HandleFileEncrypt
{
    char name_out[256];
};
void endFileEncrypt(void *info)
{
    struct HandleFileEncrypt *handle = (struct HandleFileEncrypt *)info;
    remove(handle->name_out);
}
#define HASH_FileEncrypt 0x64d9b684
void mFileEncrypt(MFile *file,uint64_t key)
{
    MHandle *hdl=mHandle(file,FileEncrypt);
    struct HandleFileEncrypt *handle = (struct HandleFileEncrypt *)(hdl->handle);
    if(hdl->valid == 1) return;
   
    char name_in[256];strcpy(name_in,file->filename);
    
    char name[32];sprintf(handle->name_out,"./%stmp",tmpnam(name));
    mObjectRedefine(file,handle->name_out);hdl->valid = 1;
    
    mEncrypt(name_in,handle->name_out,key);
}

