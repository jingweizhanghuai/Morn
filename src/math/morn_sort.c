/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_math.h"

void AscSortDataU8(U8 *data,int num)
{
    int bucket_num[256];memset(bucket_num,0,256*sizeof(int));
    for(int i=0;i<num;i++) bucket_num[data[i]]++;
    int n=0;
    for(int i=0;i<256;i++)
    {
        if(bucket_num[i]>0)
        {
            memset(data+n,i,bucket_num[i]);
            n=n+bucket_num[i];
        }
    }
}


#define AscSortData(Type,Data,Num) {\
	Type Buff;\
    if(Data[0]>Data[Num-1]) {Buff=Data[0];Data[0]=Data[Num-1];Data[Num-1]=Buff;}\
    if(Num==2) return;\
    \
	Buff=Data[Num>>1];Data[Num>>1]=Data[1];Data[1]=Buff;\
         if(Data[1]<Data[    0]) {Buff=Data[    0];Data[    0]=Data[1];}\
    else if(Data[1]>Data[Num-1]) {Buff=Data[Num-1];Data[Num-1]=Data[1];}\
    if(Num==3) {Data[1]=Buff;return;}\
    \
    int I=1;int J=Num-2;\
    while(1)\
    {\
        while(Data[J]>=Buff){J=J-1;if(J==I) goto AscSortData_next;}\
        Data[I] = Data[J];   I=I+1;if(J==I) goto AscSortData_next; \
        while(Data[I]<=Buff){I=I+1;if(J==I) goto AscSortData_next;}\
        Data[J] = Data[I];   J=J-1;if(J==I) goto AscSortData_next; \
    }\
AscSortData_next:\
    Data[I] = Buff;\
    if(    I  >1) AscSortData##Type(Data    ,    I  );\
    if(Num-I-1>1) AscSortData##Type(Data+I+1,Num-I-1);\
}
// void AscSortDataU8 (U8  *data,int num) {AscSortData(U8 ,data,num);}
void AscSortDataS8 (S8  *data,int num) {AscSortData(S8 ,data,num);}
void AscSortDataU16(U16 *data,int num) {AscSortData(U16,data,num);}
void AscSortDataS16(S16 *data,int num) {AscSortData(S16,data,num);}
void AscSortDataU32(U32 *data,int num) {AscSortData(U32,data,num);}
void AscSortDataS32(S32 *data,int num) {AscSortData(S32,data,num);}
void AscSortDataU64(U64 *data,int num) {AscSortData(U64,data,num);}
void AscSortDataS64(S64 *data,int num) {AscSortData(S64,data,num);}
void AscSortDataF32(F32 *data,int num) {AscSortData(F32,data,num);}
void AscSortDataD64(D64 *data,int num) {AscSortData(D64,data,num);}

#define AscSortIndex(Type,Data,Index,Num) {\
    Type Buff;int Buff_index;\
    if(Data[0]>Data[Num-1])\
    {\
        Buff      = Data[0]; Data[0]= Data[Num-1]; Data[Num-1]=Buff;\
        Buff_index=Index[0];Index[0]=Index[Num-1];Index[Num-1]=Buff_index;\
    }\
    if(Num==2) return;\
    \
	Buff=Data[Num>>1];Buff_index=Index[Num>>1];Data[Num>>1]=Data[1];Index[Num>>1]=Index[1];Data[1]=Buff;Index[1]=Buff_index;\
         if(Data[1]<Data[    0]) {Buff=Data[    0];Data[    0]=Data[1];Buff_index=Index[    0];Index[    0]=Index[1];}\
    else if(Data[1]>Data[Num-1]) {Buff=Data[Num-1];Data[Num-1]=Data[1];Buff_index=Index[Num-1];Index[Num-1]=Index[1];}\
    if(Num==3) {Data[1]=Buff;Index[1]=Buff_index; return;}\
    \
    int I=1;int J=Num-2;\
    while(1)\
    {\
        while(Data[J]>=Buff)             {J=J-1;if(J==I) goto AscSortIndex_next;}\
        Data[I]=Data[J];Index[I]=Index[J];I=I+1;if(J==I) goto AscSortIndex_next; \
        while(Data[I]<=Buff)             {I=I+1;if(J==I) goto AscSortIndex_next;}\
        Data[J]=Data[I];Index[J]=Index[I];J=J-1;if(J==I) goto AscSortIndex_next; \
    }\
\
AscSortIndex_next:\
    Data[I] = Buff;Index[I] = Buff_index;\
    if(    I  >1) AscSortIndex##Type(Data    ,Index    ,    I  );\
    if(Num-I-1>1) AscSortIndex##Type(Data+I+1,Index+I+1,Num-I-1);\
}
void AscSortIndexU8 (U8  *data,int *index,int num) {AscSortIndex(U8 ,data,index,num);}
void AscSortIndexS8 (S8  *data,int *index,int num) {AscSortIndex(S8 ,data,index,num);}
void AscSortIndexU16(U16 *data,int *index,int num) {AscSortIndex(U16,data,index,num);}
void AscSortIndexS16(S16 *data,int *index,int num) {AscSortIndex(S16,data,index,num);}
void AscSortIndexU32(U32 *data,int *index,int num) {AscSortIndex(U32,data,index,num);}
void AscSortIndexS32(S32 *data,int *index,int num) {AscSortIndex(S32,data,index,num);}
void AscSortIndexU64(U64 *data,int *index,int num) {AscSortIndex(U64,data,index,num);}
void AscSortIndexS64(S64 *data,int *index,int num) {AscSortIndex(S64,data,index,num);}
void AscSortIndexF32(F32 *data,int *index,int num) {AscSortIndex(F32,data,index,num);}
void AscSortIndexD64(D64 *data,int *index,int num) {AscSortIndex(D64,data,index,num);}

#define AscSort(Type,DataIn,Index_in,DataOut,Index_out,Num) {\
    Type *Data_in = (Type *)(DataIn);Type *Data_out = (Type *)(DataOut);\
    mException((INVALID_POINTER(Data_in)),EXIT,"invalid input");\
    if((Data_out!=NULL)&&(Data_out!=Data_in)) {memcpy(Data_out,Data_in,Num*sizeof(Type));Data_in=Data_out;}\
    if((Index_in==NULL)&&(Index_out==NULL)) {if(Num>1) AscSortData##Type(Data_in,Num); return;}\
    \
    if(Index_in==NULL) {for(int I=0;I<Num;I++)Index_out[I] = I;Index_in = Index_out;}\
    else if((Index_out!=NULL)&&(Index_in != Index_out))\
        {memcpy(Index_out,Index_in,Num*sizeof(int));Index_in = Index_out;}\
    if(Num>1) {AscSortIndex##Type(Data_in,Index_in,Num);return;}\
}
void mAscSortU8 (U8  *data_in,int *index_in,U8  *data_out,int *index_out,int num) {AscSort(U8 ,data_in,index_in,data_out,index_out,num);}
void mAscSortS8 (S8  *data_in,int *index_in,S8  *data_out,int *index_out,int num) {AscSort(S8 ,data_in,index_in,data_out,index_out,num);}
void mAscSortU16(U16 *data_in,int *index_in,U16 *data_out,int *index_out,int num) {AscSort(U16,data_in,index_in,data_out,index_out,num);}
void mAscSortS16(S16 *data_in,int *index_in,S16 *data_out,int *index_out,int num) {AscSort(S16,data_in,index_in,data_out,index_out,num);}
void mAscSortU32(U32 *data_in,int *index_in,U32 *data_out,int *index_out,int num) {AscSort(U32,data_in,index_in,data_out,index_out,num);}
void mAscSortS32(S32 *data_in,int *index_in,S32 *data_out,int *index_out,int num) {AscSort(S32,data_in,index_in,data_out,index_out,num);}
void mAscSortU64(U64 *data_in,int *index_in,U64 *data_out,int *index_out,int num) {AscSort(U64,data_in,index_in,data_out,index_out,num);}
void mAscSortS64(S64 *data_in,int *index_in,S64 *data_out,int *index_out,int num) {AscSort(S64,data_in,index_in,data_out,index_out,num);}
void mAscSortF32(F32 *data_in,int *index_in,F32 *data_out,int *index_out,int num) {AscSort(F32,data_in,index_in,data_out,index_out,num);}
void mAscSortD64(D64 *data_in,int *index_in,D64 *data_out,int *index_out,int num) {AscSort(D64,data_in,index_in,data_out,index_out,num);}


#define DescSortData(Type,Data,Num) {\
    Type Buff;\
    if(Data[0]<Data[Num-1]) {Buff=Data[0];Data[0]=Data[Num-1];Data[Num-1]=Buff;}\
    if(Num==2) return;\
    \
	Buff=Data[Num>>1];Data[Num>>1]=Data[1];Data[1]=Buff;\
         if(Data[1]>Data[    0]) {Buff=Data[    0];Data[    0]=Data[1];}\
    else if(Data[1]<Data[Num-1]) {Buff=Data[Num-1];Data[Num-1]=Data[1];}\
    if(Num==3) {Data[1]=Buff;return;}\
    \
    int I=1;int J=Num-2;\
    while(1)\
    {\
        while(Data[J]<=Buff){J=J-1;if(J==I) goto DescSortData_next;}\
        Data[I] = Data[J];   I=I+1;if(J==I) goto DescSortData_next; \
        while(Data[I]>=Buff){I=I+1;if(J==I) goto DescSortData_next;}\
        Data[J] = Data[I];   J=J-1;if(J==I) goto DescSortData_next; \
    }\
DescSortData_next:\
    Data[I] = Buff;\
    if(    I  >1) DescSortData##Type(Data    ,    I  );\
    if(Num-I-1>1) DescSortData##Type(Data+I+1,Num-I-1);\
}
void DescSortDataU8 (U8  *data,int num) {DescSortData(U8 ,data,num);}
void DescSortDataS8 (S8  *data,int num) {DescSortData(S8 ,data,num);}
void DescSortDataU16(U16 *data,int num) {DescSortData(U16,data,num);}
void DescSortDataS16(S16 *data,int num) {DescSortData(S16,data,num);}
void DescSortDataU32(U32 *data,int num) {DescSortData(U32,data,num);}
void DescSortDataS32(S32 *data,int num) {DescSortData(S32,data,num);}
void DescSortDataU64(U64 *data,int num) {DescSortData(U64,data,num);}
void DescSortDataS64(S64 *data,int num) {DescSortData(S64,data,num);}
void DescSortDataF32(F32 *data,int num) {DescSortData(F32,data,num);}
void DescSortDataD64(D64 *data,int num) {DescSortData(D64,data,num);}

#define DescSortIndex(Type,Data,Index,Num) {\
    Type Buff;int Buff_index;\
    if(Data[0]<Data[Num-1])\
    {\
        Buff      = Data[0]; Data[0]= Data[Num-1]; Data[Num-1]=Buff;\
        Buff_index=Index[0];Index[0]=Index[Num-1];Index[Num-1]=Buff_index;\
    }\
    if(Num==2) return;\
    \
	Buff=Data[Num>>1];Buff_index=Index[Num>>1];Data[Num>>1]=Data[1];Index[Num>>1]=Index[1];Data[1]=Buff;Index[1]=Buff_index;\
         if(Data[1]>Data[    0]) {Buff=Data[    0];Data[    0]=Data[1];Buff_index=Index[    0];Index[    0]=Index[1];}\
    else if(Data[1]<Data[Num-1]) {Buff=Data[Num-1];Data[Num-1]=Data[1];Buff_index=Index[Num-1];Index[Num-1]=Index[1];}\
    if(Num==3) {Data[1]=Buff;Index[1]=Buff_index; return;}\
    \
    int I=1;int J=Num-2;\
    while(1)\
    {\
        while(Data[J]<=Buff)             {J=J-1;if(J==I) goto DescSortIndex_next;}\
        Data[I]=Data[J];Index[I]=Index[J];I=I+1;if(J==I) goto DescSortIndex_next; \
        while(Data[I]>=Buff)             {I=I+1;if(J==I) goto DescSortIndex_next;}\
        Data[J]=Data[I];Index[J]=Index[I];J=J-1;if(J==I) goto DescSortIndex_next; \
    }\
\
DescSortIndex_next:\
    Data[I] = Buff;Index[I] = Buff_index;\
    if(    I  >1) DescSortIndex##Type(Data    ,Index    ,    I  );\
    if(Num-I-1>1) DescSortIndex##Type(Data+I+1,Index+I+1,Num-I-1);\
}
void DescSortIndexU8 (U8  *data,int *index,int num) {DescSortIndex(U8 ,data,index,num);}
void DescSortIndexS8 (S8  *data,int *index,int num) {DescSortIndex(S8 ,data,index,num);}
void DescSortIndexU16(U16 *data,int *index,int num) {DescSortIndex(U16,data,index,num);}
void DescSortIndexS16(S16 *data,int *index,int num) {DescSortIndex(S16,data,index,num);}
void DescSortIndexU32(U32 *data,int *index,int num) {DescSortIndex(U32,data,index,num);}
void DescSortIndexS32(S32 *data,int *index,int num) {DescSortIndex(S32,data,index,num);}
void DescSortIndexU64(U64 *data,int *index,int num) {DescSortIndex(U64,data,index,num);}
void DescSortIndexS64(S64 *data,int *index,int num) {DescSortIndex(S64,data,index,num);}
void DescSortIndexF32(F32 *data,int *index,int num) {DescSortIndex(F32,data,index,num);}
void DescSortIndexD64(D64 *data,int *index,int num) {DescSortIndex(D64,data,index,num);}

#define DescSort(Type,DataIn,Index_in,DataOut,Index_out,Num) {\
    Type *Data_in = (Type *)(DataIn);Type *Data_out = (Type *)(DataOut);\
    mException((INVALID_POINTER(Data_in)),EXIT,"invalid input");\
    if((Data_out!=NULL)&&(Data_out!=Data_in)) {memcpy(Data_out,Data_in,Num*sizeof(Type));Data_in=Data_out;}\
    if((Index_in==NULL)&&(Index_out==NULL)) {if(Num>1) DescSortData##Type(Data_in,Num); return;}\
    \
    if(Index_in==NULL) {for(int I=0;I<Num;I++)Index_out[I] = I;Index_in = Index_out;}\
    else if((Index_out!=NULL)&&(Index_in != Index_out))\
        {memcpy(Index_out,Index_in,Num*sizeof(int));Index_in = Index_out;}\
    if(Num>1) {DescSortIndex##Type(Data_in,Index_in,Num);return;}\
}
void mDescSortU8 (U8  *data_in,int *index_in,U8  *data_out,int *index_out,int num) {DescSort(U8 ,data_in,index_in,data_out,index_out,num);}
void mDescSortS8 (S8  *data_in,int *index_in,S8  *data_out,int *index_out,int num) {DescSort(S8 ,data_in,index_in,data_out,index_out,num);}
void mDescSortU16(U16 *data_in,int *index_in,U16 *data_out,int *index_out,int num) {DescSort(U16,data_in,index_in,data_out,index_out,num);}
void mDescSortS16(S16 *data_in,int *index_in,S16 *data_out,int *index_out,int num) {DescSort(S16,data_in,index_in,data_out,index_out,num);}
void mDescSortU32(U32 *data_in,int *index_in,U32 *data_out,int *index_out,int num) {DescSort(U32,data_in,index_in,data_out,index_out,num);}
void mDescSortS32(S32 *data_in,int *index_in,S32 *data_out,int *index_out,int num) {DescSort(S32,data_in,index_in,data_out,index_out,num);}
void mDescSortU64(U64 *data_in,int *index_in,U64 *data_out,int *index_out,int num) {DescSort(U64,data_in,index_in,data_out,index_out,num);}
void mDescSortS64(S64 *data_in,int *index_in,S64 *data_out,int *index_out,int num) {DescSort(S64,data_in,index_in,data_out,index_out,num);}
void mDescSortF32(F32 *data_in,int *index_in,F32 *data_out,int *index_out,int num) {DescSort(F32,data_in,index_in,data_out,index_out,num);}
void mDescSortD64(D64 *data_in,int *index_in,D64 *data_out,int *index_out,int num) {DescSort(D64,data_in,index_in,data_out,index_out,num);}

#define T_D64 double
#define T_F32 double
#define T_S64 int64_t
#define T_U64 uint64_t
#define T_S32 int
#define T_U32 int
#define T_S16 int
#define T_U16 int
#define T_S8  int
#define T_U8  int

#define D_D64 0.000000000001
#define D_F32 0.000000000001
#define D_S64 1
#define D_U64 1
#define D_S32 1
#define D_U32 1
#define D_S16 1
#define D_U16 1
#define D_S8  1
#define D_U8  1

#define DataMinSubset(Type,Data,NumIn,NumOut) {\
    Type Min,Max;\
    if(Data[0]<Data[NumIn-1]) {Min=Data[0];Max=Data[NumIn-1];}\
    else                      {Max=Data[0];Min=Data[NumIn-1];}\
    Min=MIN(Min,Data[NumIn/2]);Max=MAX(Max,Data[NumIn/2]);\
    if(Min==Max) for(int I=1;I<NumIn-1;I++) {Min=MIN(Min,Data[I]);Max=MAX(Max,Data[I]);}\
    if(Min==Max) return Max;\
    double K=(double)NumOut/(double)NumIn;if(K<0.1) {K=0.1;} else if(K>0.9) {K=0.9;}\
    T_##Type Thresh=(Type)((double)(Max-Min)*K+Min);\
    if(Thresh==Max) Thresh-=D_##Type;\
    \
    int I=0;int J=NumIn-1;\
    while(1)\
    {\
        while(Data[I]<=Thresh) I=I+1;\
        while(Data[J]> Thresh) J=J-1;\
        if(I>J) break;\
        Type Buff=Data[I];Data[I]=Data[J];Data[J]=Buff;\
        I=I+1;J=J-1;\
    }\
    \
    if(I >NumOut) return DataMinSubset##Type(Data,        I,NumOut  );\
    if(I <NumOut) return DataMinSubset##Type(Data+I,NumIn-I,NumOut-I);\
    Max=Data[0];for(int I=1;I<NumOut;I++) Max=MAX(Max,Data[I]);\
    return Max;\
}
U8  DataMinSubsetU8 (U8  *data,int num_in,int num_out) {DataMinSubset(U8 ,data,num_in,num_out);}
S8  DataMinSubsetS8 (S8  *data,int num_in,int num_out) {DataMinSubset(S8 ,data,num_in,num_out);}
U16 DataMinSubsetU16(U16 *data,int num_in,int num_out) {DataMinSubset(U16,data,num_in,num_out);}
S16 DataMinSubsetS16(S16 *data,int num_in,int num_out) {DataMinSubset(S16,data,num_in,num_out);}
U32 DataMinSubsetU32(U32 *data,int num_in,int num_out) {DataMinSubset(U32,data,num_in,num_out);}
S32 DataMinSubsetS32(S32 *data,int num_in,int num_out) {DataMinSubset(S32,data,num_in,num_out);}
U64 DataMinSubsetU64(U64 *data,int num_in,int num_out) {DataMinSubset(U64,data,num_in,num_out);}
S64 DataMinSubsetS64(S64 *data,int num_in,int num_out) {DataMinSubset(S64,data,num_in,num_out);}
F32 DataMinSubsetF32(F32 *data,int num_in,int num_out) {DataMinSubset(F32,data,num_in,num_out);}
D64 DataMinSubsetD64(D64 *data,int num_in,int num_out) {DataMinSubset(D64,data,num_in,num_out);}

#define IndexMinSubset(Type,Data,Index,NumIn,NumOut) {\
    Type Min,Max;\
    if(Data[0]<Data[NumIn-1]) {Min=Data[0];Max=Data[NumIn-1];}\
    else                      {Max=Data[0];Min=Data[NumIn-1];}\
    Min=MIN(Min,Data[NumIn/2]);Max=MAX(Max,Data[NumIn/2]);\
    if(Min==Max) for(int I=1;I<NumIn-1;I++) {Min=MIN(Min,Data[I]);Max=MAX(Max,Data[I]);}\
    if(Min==Max) return Max;\
    double K=(double)NumOut/(double)NumIn;if(K<0.1) {K=0.1;} else if(K>0.9) {K=0.9;}\
    T_##Type Thresh=(Type)((double)(Max-Min)*K+Min);\
    if(Thresh==Max) Thresh-=D_##Type;\
    \
    int I=0;int J=NumIn-1;\
    while(1)\
    {\
        while(Data[I]<=Thresh) I=I+1;\
        while(Data[J]> Thresh) J=J-1;\
        if(I>J) break;\
        Type Buff= Data[I]; Data[I]= Data[J]; Data[J]= Buff;\
        int IBuff=Index[I];Index[I]=Index[J];Index[J]=IBuff;\
        I=I+1;J=J-1;\
    }\
    \
    if(I >NumOut) return IndexMinSubset##Type(Data  ,Index  ,      I,NumOut  );\
    if(I <NumOut) return IndexMinSubset##Type(Data+I,Index+I,NumIn-I,NumOut-I);\
    Max=Data[0];for(int I=1;I<NumOut;I++) Max=MAX(Max,Data[I]);\
    return Max;\
}

U8  IndexMinSubsetU8 (U8  *data,int *index,int num_in,int num_out) {IndexMinSubset(U8 ,data,index,num_in,num_out);}
S8  IndexMinSubsetS8 (S8  *data,int *index,int num_in,int num_out) {IndexMinSubset(S8 ,data,index,num_in,num_out);}
U16 IndexMinSubsetU16(U16 *data,int *index,int num_in,int num_out) {IndexMinSubset(U16,data,index,num_in,num_out);}
S16 IndexMinSubsetS16(S16 *data,int *index,int num_in,int num_out) {IndexMinSubset(S16,data,index,num_in,num_out);}
U32 IndexMinSubsetU32(U32 *data,int *index,int num_in,int num_out) {IndexMinSubset(U32,data,index,num_in,num_out);}
S32 IndexMinSubsetS32(S32 *data,int *index,int num_in,int num_out) {IndexMinSubset(S32,data,index,num_in,num_out);}
U64 IndexMinSubsetU64(U64 *data,int *index,int num_in,int num_out) {IndexMinSubset(U64,data,index,num_in,num_out);}
S64 IndexMinSubsetS64(S64 *data,int *index,int num_in,int num_out) {IndexMinSubset(S64,data,index,num_in,num_out);}
F32 IndexMinSubsetF32(F32 *data,int *index,int num_in,int num_out) {IndexMinSubset(F32,data,index,num_in,num_out);}
D64 IndexMinSubsetD64(D64 *data,int *index,int num_in,int num_out) {IndexMinSubset(D64,data,index,num_in,num_out);}

#define MinSubset(Type,Data_In,IndexIn,NumIn,Data_Out,IndexOut,NumOut) {\
    Type *DataIn=(Type *)Data_In;Type *DataOut=(Type *)Data_Out;\
    mException((DataIn==NULL),EXIT,"invalid input");\
    mException((NumOut>=NumIn)||(NumIn<=0)||(NumOut<=0),EXIT,"invalid input with number in is %d number out is %d",NumIn,NumOut);\
    \
    Type *Buff;\
    if((DataOut==DataIn)||(DataOut==NULL)) {Buff=DataIn;}\
    else                                    Buff=(Type *)mMalloc(NumIn*sizeof(Type));\
    \
    Type Thresh;\
    \
    if((IndexIn==NULL)&&(IndexOut==NULL))\
        Thresh=DataMinSubset##Type(Buff,NumIn,NumOut);\
    else\
    {\
        if(IndexOut==IndexIn) IndexOut=NULL;\
        \
        int *IndexBuff;\
        if(IndexOut==NULL) IndexBuff=IndexIn;\
        else\
        {\
            IndexBuff = (int *)mMalloc(NumIn*sizeof(int));\
            if(IndexIn==NULL) {for(int I=0;I<NumIn;I++) IndexBuff[I]=I;IndexIn=IndexBuff;}\
            else               memcpy(IndexBuff,IndexIn,NumIn*sizeof(int));\
        }\
        \
        Thresh=IndexMinSubset##Type(Buff,IndexBuff,NumIn,NumOut);\
        if(IndexOut != NULL) {memcpy(IndexOut,IndexBuff,NumOut*sizeof(int));mFree(IndexBuff);}\
    }\
    if(Buff!=DataIn) {memcpy(DataOut,Buff,NumOut*sizeof(Type));mFree(Buff);}\
    return Thresh;\
}

U8  mMinSubsetU8 (U8  *data_in,int *index_in,int num_in,U8  *data_out,int *index_out,int num_out) {MinSubset(U8 ,data_in,index_in,num_in,data_out,index_out,num_out);}
S8  mMinSubsetS8 (S8  *data_in,int *index_in,int num_in,S8  *data_out,int *index_out,int num_out) {MinSubset(S8 ,data_in,index_in,num_in,data_out,index_out,num_out);}
U16 mMinSubsetU16(U16 *data_in,int *index_in,int num_in,U16 *data_out,int *index_out,int num_out) {MinSubset(U16,data_in,index_in,num_in,data_out,index_out,num_out);}
S16 mMinSubsetS16(S16 *data_in,int *index_in,int num_in,S16 *data_out,int *index_out,int num_out) {MinSubset(S16,data_in,index_in,num_in,data_out,index_out,num_out);}
U32 mMinSubsetU32(U32 *data_in,int *index_in,int num_in,U32 *data_out,int *index_out,int num_out) {MinSubset(U32,data_in,index_in,num_in,data_out,index_out,num_out);}
S32 mMinSubsetS32(S32 *data_in,int *index_in,int num_in,S32 *data_out,int *index_out,int num_out) {MinSubset(S32,data_in,index_in,num_in,data_out,index_out,num_out);}
U64 mMinSubsetU64(U64 *data_in,int *index_in,int num_in,U64 *data_out,int *index_out,int num_out) {MinSubset(U64,data_in,index_in,num_in,data_out,index_out,num_out);}
S64 mMinSubsetS64(S64 *data_in,int *index_in,int num_in,S64 *data_out,int *index_out,int num_out) {MinSubset(S64,data_in,index_in,num_in,data_out,index_out,num_out);}
F32 mMinSubsetF32(F32 *data_in,int *index_in,int num_in,F32 *data_out,int *index_out,int num_out) {MinSubset(F32,data_in,index_in,num_in,data_out,index_out,num_out);}
D64 mMinSubsetD64(D64 *data_in,int *index_in,int num_in,D64 *data_out,int *index_out,int num_out) {MinSubset(D64,data_in,index_in,num_in,data_out,index_out,num_out);}

#define DataMaxSubset(Type,Data,NumIn,NumOut) {\
    Type Min,Max;\
    if(Data[0]<Data[NumIn-1]) {Min=Data[0];Max=Data[NumIn-1];}\
    else                      {Max=Data[0];Min=Data[NumIn-1];}\
    Min=MIN(Min,Data[NumIn/2]);Max=MAX(Max,Data[NumIn/2]);\
    if(Min==Max) for(int I=1;I<NumIn-1;I++) {Min=MIN(Min,Data[I]);Max=MAX(Max,Data[I]);}\
    if(Min==Max) return Max;\
    double K=(double)NumOut/(double)NumIn;if(K<0.1) {K=0.1;} else if(K>0.9) {K=0.9;}\
    T_##Type Thresh=Max-(Type)((double)(Max-Min)*K);\
    if(Thresh==Min) Thresh+=D_##Type;\
    \
    int I=0;int J=NumIn-1;\
    while(1)\
    {\
        while(Data[I]>=Thresh) I=I+1;\
        while(Data[J]< Thresh) J=J-1;\
        if(I>J) break;\
        Type Buff=Data[I];Data[I]=Data[J];Data[J]=Buff;\
        I=I+1;J=J-1;\
    }\
    \
    if(I >NumOut) return DataMaxSubset##Type(Data,        I,NumOut  );\
    if(I <NumOut) return DataMaxSubset##Type(Data+I,NumIn-I,NumOut-I);\
    Min=Data[0];for(int I=1;I<NumOut;I++) Min=MIN(Min,Data[I]);\
    return Min;\
}
U8  DataMaxSubsetU8 (U8  *data,int num_in,int num_out) {DataMaxSubset(U8 ,data,num_in,num_out);}
S8  DataMaxSubsetS8 (S8  *data,int num_in,int num_out) {DataMaxSubset(S8 ,data,num_in,num_out);}
U16 DataMaxSubsetU16(U16 *data,int num_in,int num_out) {DataMaxSubset(U16,data,num_in,num_out);}
S16 DataMaxSubsetS16(S16 *data,int num_in,int num_out) {DataMaxSubset(S16,data,num_in,num_out);}
U32 DataMaxSubsetU32(U32 *data,int num_in,int num_out) {DataMaxSubset(U32,data,num_in,num_out);}
S32 DataMaxSubsetS32(S32 *data,int num_in,int num_out) {DataMaxSubset(S32,data,num_in,num_out);}
U64 DataMaxSubsetU64(U64 *data,int num_in,int num_out) {DataMaxSubset(U64,data,num_in,num_out);}
S64 DataMaxSubsetS64(S64 *data,int num_in,int num_out) {DataMaxSubset(S64,data,num_in,num_out);}
F32 DataMaxSubsetF32(F32 *data,int num_in,int num_out) {DataMaxSubset(F32,data,num_in,num_out);}
D64 DataMaxSubsetD64(D64 *data,int num_in,int num_out) {DataMaxSubset(D64,data,num_in,num_out);}

#define IndexMaxSubset(Type,Data,Index,NumIn,NumOut) {\
    Type Min,Max;\
    if(Data[0]<Data[NumIn-1]) {Min=Data[0];Max=Data[NumIn-1];}\
    else                      {Max=Data[0];Min=Data[NumIn-1];}\
    Min=MIN(Min,Data[NumIn/2]);Max=MAX(Max,Data[NumIn/2]);\
    if(Min==Max) for(int I=1;I<NumIn-1;I++) {Min=MIN(Min,Data[I]);Max=MAX(Max,Data[I]);}\
    if(Min==Max) return Max;\
    double K=(double)NumOut/(double)NumIn;if(K<0.1) {K=0.1;} else if(K>0.9) {K=0.9;}\
    T_##Type Thresh=Max-(Type)((double)(Max-Min)*K);\
    if(Thresh==Min) Thresh+=D_##Type;\
    \
    int I=0;int J=NumIn-1;\
    while(1)\
    {\
        while(Data[I]>=Thresh) I=I+1;\
        while(Data[J]< Thresh) J=J-1;\
        if(I>J) break;\
        Type Buff= Data[I]; Data[I]= Data[J]; Data[J]= Buff;\
        int IBuff=Index[I];Index[I]=Index[J];Index[J]=IBuff;\
        I=I+1;J=J-1;\
    }\
    \
    if(I >NumOut) return IndexMaxSubset##Type(Data  ,Index  ,      I,NumOut  );\
    if(I <NumOut) return IndexMaxSubset##Type(Data+I,Index+I,NumIn-I,NumOut-I);\
    Min=Data[0];for(int I=1;I<NumOut;I++) Min=MIN(Min,Data[I]);\
    return Min;\
}

U8  IndexMaxSubsetU8 (U8  *data,int *index,int num_in,int num_out) {IndexMaxSubset(U8 ,data,index,num_in,num_out);}
S8  IndexMaxSubsetS8 (S8  *data,int *index,int num_in,int num_out) {IndexMaxSubset(S8 ,data,index,num_in,num_out);}
U16 IndexMaxSubsetU16(U16 *data,int *index,int num_in,int num_out) {IndexMaxSubset(U16,data,index,num_in,num_out);}
S16 IndexMaxSubsetS16(S16 *data,int *index,int num_in,int num_out) {IndexMaxSubset(S16,data,index,num_in,num_out);}
U32 IndexMaxSubsetU32(U32 *data,int *index,int num_in,int num_out) {IndexMaxSubset(U32,data,index,num_in,num_out);}
S32 IndexMaxSubsetS32(S32 *data,int *index,int num_in,int num_out) {IndexMaxSubset(S32,data,index,num_in,num_out);}
U64 IndexMaxSubsetU64(U64 *data,int *index,int num_in,int num_out) {IndexMaxSubset(U64,data,index,num_in,num_out);}
S64 IndexMaxSubsetS64(S64 *data,int *index,int num_in,int num_out) {IndexMaxSubset(S64,data,index,num_in,num_out);}
F32 IndexMaxSubsetF32(F32 *data,int *index,int num_in,int num_out) {IndexMaxSubset(F32,data,index,num_in,num_out);}
D64 IndexMaxSubsetD64(D64 *data,int *index,int num_in,int num_out) {IndexMaxSubset(D64,data,index,num_in,num_out);}

#define MaxSubset(Type,Data_In,IndexIn,NumIn,Data_Out,IndexOut,NumOut) {\
    Type *DataIn=(Type *)Data_In;Type *DataOut=(Type *)Data_Out;\
    mException((DataIn==NULL),EXIT,"invalid input");\
    mException((NumOut>=NumIn)||(NumIn<=0)||(NumOut<=0),EXIT,"invalid input with number in is %d number out is %d",NumIn,NumOut);\
    \
    Type *Buff;\
    if((DataOut==DataIn)||(DataOut==NULL)) {Buff=DataIn;}\
    else                                    Buff=(Type *)mMalloc(NumIn*sizeof(Type));\
    \
    Type Thresh;\
    if((IndexIn==NULL)&&(IndexOut==NULL))\
        Thresh=DataMaxSubset##Type(Buff,NumIn,NumOut);\
    else\
    {\
        if(IndexOut==IndexIn) IndexOut=NULL;\
        \
        int *IndexBuff;\
        if(IndexOut==NULL) IndexBuff=IndexIn;\
        else\
        {\
            IndexBuff = (int *)mMalloc(NumIn*sizeof(int));\
            if(IndexIn==NULL) {for(int I=0;I<NumIn;I++) IndexBuff[I]=I;IndexIn=IndexBuff;}\
            else               memcpy(IndexBuff,IndexIn,NumIn*sizeof(int));\
        }\
        \
        Thresh=IndexMaxSubset##Type(Buff,IndexBuff,NumIn,NumOut);\
        if(IndexOut != NULL) {memcpy(IndexOut,IndexBuff,NumOut*sizeof(int));mFree(IndexBuff);}\
    }\
    \
    if(Buff!=DataIn) {memcpy(DataOut,Buff,NumOut*sizeof(Type));mFree(Buff);}\
    return Thresh;\
}
 
U8  mMaxSubsetU8 (U8  *data_in,int *index_in,int num_in,U8  *data_out,int *index_out,int num_out) {MaxSubset(U8 ,data_in,index_in,num_in,data_out,index_out,num_out);}
S8  mMaxSubsetS8 (S8  *data_in,int *index_in,int num_in,S8  *data_out,int *index_out,int num_out) {MaxSubset(S8 ,data_in,index_in,num_in,data_out,index_out,num_out);}
U16 mMaxSubsetU16(U16 *data_in,int *index_in,int num_in,U16 *data_out,int *index_out,int num_out) {MaxSubset(U16,data_in,index_in,num_in,data_out,index_out,num_out);}
S16 mMaxSubsetS16(S16 *data_in,int *index_in,int num_in,S16 *data_out,int *index_out,int num_out) {MaxSubset(S16,data_in,index_in,num_in,data_out,index_out,num_out);}
U32 mMaxSubsetU32(U32 *data_in,int *index_in,int num_in,U32 *data_out,int *index_out,int num_out) {MaxSubset(U32,data_in,index_in,num_in,data_out,index_out,num_out);}
S32 mMaxSubsetS32(S32 *data_in,int *index_in,int num_in,S32 *data_out,int *index_out,int num_out) {MaxSubset(S32,data_in,index_in,num_in,data_out,index_out,num_out);}
U64 mMaxSubsetU64(U64 *data_in,int *index_in,int num_in,U64 *data_out,int *index_out,int num_out) {MaxSubset(U64,data_in,index_in,num_in,data_out,index_out,num_out);}
S64 mMaxSubsetS64(S64 *data_in,int *index_in,int num_in,S64 *data_out,int *index_out,int num_out) {MaxSubset(S64,data_in,index_in,num_in,data_out,index_out,num_out);}
F32 mMaxSubsetF32(F32 *data_in,int *index_in,int num_in,F32 *data_out,int *index_out,int num_out) {MaxSubset(F32,data_in,index_in,num_in,data_out,index_out,num_out);}
D64 mMaxSubsetD64(D64 *data_in,int *index_in,int num_in,D64 *data_out,int *index_out,int num_out) {MaxSubset(D64,data_in,index_in,num_in,data_out,index_out,num_out);}
