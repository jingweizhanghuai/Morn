/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build_msvc: cl.exe -O2 -nologo -I ..\include\ test_datatype.c ..\lib\x64_msvc\libmorn.lib
// build_mingw: gcc test_datatype.c -o test_datatype.exe -lmorn
#include "morn_util.h"

// #define type_check(Data) (_Generic((Data),int:"int",float:"float",default:"unknown"))
// int V;
// #define type_check(Data) (V=Data,memcmp(&Data,&V,sizeof(int))?"int":"float")
// int main()
// {
//     int   a;printf("a type is %s\n",type_check(a));
//     float b;printf("b type is %s\n",type_check(b));
//     return 0;
// }

int main1()
{
    char *data_type[25]={"unsigned char","signed char","unsigned short","short","unsigned int","int",
                         "unsigned long long","signed long long","float","double",
                         "pointer of bit16","pointer of bit32","pointer of bit64","pointer of bit8",
                         "pointer of unsigned char","pointer of signed char","pointer of char",
                         "pointer of unsigned short","pointer of signed short",
                         "pointer of unsigned int","pointer of signed int","pointer of float",
                         "pointer of unsigned long long","pointer of signed long long","pointer of double"};
                
                int8_t a1 =DFLT;printf("a1  type is %s\n",data_type[mDataType(a1 )]);
               int16_t a2 =DFLT;printf("a2  type is %s\n",data_type[mDataType(a2 )]);
               int32_t a3 =DFLT;printf("a3  type is %s\n",data_type[mDataType(a3 )]);
               int64_t a4 =DFLT;printf("a4  type is %s\n",data_type[mDataType(a4 )]);
               uint8_t a5 =DFLT;printf("a5  type is %s\n",data_type[mDataType(a5 )]);
              uint16_t a6 =DFLT;printf("a6  type is %s\n",data_type[mDataType(a6 )]);
              uint32_t a7 =DFLT;printf("a7  type is %s\n",data_type[mDataType(a7 )]);
              uint64_t a8 =DFLT;printf("a8  type is %s\n",data_type[mDataType(a8 )]);
                 float a9 =DFLT;printf("a9  type is %s\n",data_type[mDataType(a9 )]);
                double a10=DFLT;printf("a10 type is %s\n",data_type[mDataType(a10)]);
                  char a11=DFLT;printf("a11 type is %s\n",data_type[mDataType(a11)]);
           signed char a12=DFLT;printf("a12 type is %s\n",data_type[mDataType(a12)]);
         unsigned char a13=DFLT;printf("a13 type is %s\n",data_type[mDataType(a13)]);
                 short a14=DFLT;printf("a14 type is %s\n",data_type[mDataType(a14)]);
          signed short a15=DFLT;printf("a15 type is %s\n",data_type[mDataType(a15)]);
        unsigned short a16=DFLT;printf("a16 type is %s\n",data_type[mDataType(a16)]);
                   int a17=DFLT;printf("a17 type is %s\n",data_type[mDataType(a17)]);
            signed int a18=DFLT;printf("a18 type is %s\n",data_type[mDataType(a18)]);
          unsigned int a19=DFLT;printf("a19 type is %s\n",data_type[mDataType(a19)]);
                signed a20=DFLT;printf("a20 type is %s\n",data_type[mDataType(a20)]);
              unsigned a21=DFLT;printf("a21 type is %s\n",data_type[mDataType(a21)]);
                  long a22=DFLT;printf("a22 type is %s\n",data_type[mDataType(a22)]);
           signed long a23=DFLT;printf("a23 type is %s\n",data_type[mDataType(a23)]);
         unsigned long a24=DFLT;printf("a24 type is %s\n",data_type[mDataType(a24)]);
             long long a25=DFLT;printf("a25 type is %s\n",data_type[mDataType(a25)]);
      signed long long a26=DFLT;printf("a26 type is %s\n",data_type[mDataType(a26)]);
    unsigned long long a27=DFLT;printf("a27 type is %s\n",data_type[mDataType(a27)]);
              intptr_t a28=DFLT;printf("a28 type is %s\n",data_type[mDataType(a28)]);

              int16_t *b1 =NULL;printf("b1  type is %s\n",data_type[mDataType(b1 )]);
              int32_t *b2 =NULL;printf("b2  type is %s\n",data_type[mDataType(b2 )]);
              int64_t *b3 =NULL;printf("b3  type is %s\n",data_type[mDataType(b3 )]);
             uint16_t *b4 =NULL;printf("b4  type is %s\n",data_type[mDataType(b4 )]);
             uint32_t *b5 =NULL;printf("b5  type is %s\n",data_type[mDataType(b5 )]);
             uint64_t *b6 =NULL;printf("b6  type is %s\n",data_type[mDataType(b6 )]);
                float *b7 =NULL;printf("b7  type is %s\n",data_type[mDataType(b7 )]);
               double *b8 =NULL;printf("b8  type is %s\n",data_type[mDataType(b8 )]);
                 char *b9 =NULL;printf("b9  type is %s\n",data_type[mDataType(b9 )]);
          signed char *b10=NULL;printf("b10 type is %s\n",data_type[mDataType(b10)]);
        unsigned char *b11=NULL;printf("b11 type is %s\n",data_type[mDataType(b11)]);
                short *b12=NULL;printf("b12 type is %s\n",data_type[mDataType(b12)]);
         signed short *b13=NULL;printf("b13 type is %s\n",data_type[mDataType(b13)]);
       unsigned short *b14=NULL;printf("b14 type is %s\n",data_type[mDataType(b14)]);
                  int *b15=NULL;printf("b15 type is %s\n",data_type[mDataType(b15)]);
           signed int *b16=NULL;printf("b16 type is %s\n",data_type[mDataType(b16)]);
         unsigned int *b17=NULL;printf("b17 type is %s\n",data_type[mDataType(b17)]);
               signed *b18=NULL;printf("b18 type is %s\n",data_type[mDataType(b18)]);
             unsigned *b19=NULL;printf("b19 type is %s\n",data_type[mDataType(b19)]);
                 long *b20=NULL;printf("b20 type is %s\n",data_type[mDataType(b20)]);
          signed long *b21=NULL;printf("b21 type is %s\n",data_type[mDataType(b21)]);
        unsigned long *b22=NULL;printf("b22 type is %s\n",data_type[mDataType(b22)]);
            long long *b23=NULL;printf("b23 type is %s\n",data_type[mDataType(b23)]);
     signed long long *b24=NULL;printf("b24 type is %s\n",data_type[mDataType(b24)]);
   unsigned long long *b25=NULL;printf("b25 type is %s\n",data_type[mDataType(b25)]);
             intptr_t *b26=NULL;printf("b26 type is %s\n",data_type[mDataType(b26)]);
    
    return 0;
}