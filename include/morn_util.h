/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#ifndef _MORN_UNIVERSAL_H_
#define _MORN_UNIVERSAL_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdarg.h>
#include <errno.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define MORN_DEFAULT -1
#define MORN_DFLT -1
#define DFLT -1
#define mDefaltValue(Data,Value) do{if(Data==DFLT) Data=Value;}while(0)

#define MORN_FALSE 0
#define MORN_TRUE  1

#define MORN_FAIL   -1
#define MORN_SUCCESS 0

void mMornBegin();
void mMornEnd();

#define ARG(X) X
#define _VA_ARG_NUM(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,N,M,...) ((N==M+1)?((N==1)?((#A0)[0]!=0):N):DFLT)
#define VANumber(...) ARG(_VA_ARG_NUM(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,-1))
#define _VA_ARG0(A0,...) A0
#define _VA_ARG1(A0,A1,...) A1
#define _VA_ARG2(A0,A1,A2,...) A2
#define _VA_ARG3(A0,A1,A2,A3,...) A3
#define _VA_ARG4(A0,A1,A2,A3,A4,...) A4
#define _VA_ARG5(A0,A1,A2,A3,A4,A5,...) A5
#define _VA_ARG6(A0,A1,A2,A3,A4,A5,A6,...) A6
#define _VA_ARG7(A0,A1,A2,A3,A4,A5,A6,A7,...) A7
#define _VA_ARG8(A0,A1,A2,A3,A4,A5,A6,A7,A8,...) A8
#define _VA_ARG9(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,...) A9
#define _VA_ARG10(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,...) A10
#define _VA_ARG11(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,...) A11
#define _VA_ARG12(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,...) A12
#define _VA_ARG13(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,...) A13
#define _VA_ARG14(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,...) A14
#define _VA_ARG15(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,...) A15
#define _VA0(...) ARG(_VA_ARG0( __VA_ARGS__,DFLT))
#define VA0(...)  ARG(_VA_ARG0( __VA_ARGS__,DFLT)+0)
#define VA1(...)  ARG(_VA_ARG1( __VA_ARGS__,DFLT,DFLT))
#define VA2(...)  ARG(_VA_ARG2( __VA_ARGS__,DFLT,DFLT,DFLT))
#define VA3(...)  ARG(_VA_ARG3( __VA_ARGS__,DFLT,DFLT,DFLT,DFLT))
#define VA4(...)  ARG(_VA_ARG4( __VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA5(...)  ARG(_VA_ARG5( __VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA6(...)  ARG(_VA_ARG6( __VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA7(...)  ARG(_VA_ARG7( __VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA8(...)  ARG(_VA_ARG8( __VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA9(...)  ARG(_VA_ARG9( __VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA10(...) ARG(_VA_ARG10(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA11(...) ARG(_VA_ARG11(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA12(...) ARG(_VA_ARG12(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA13(...) ARG(_VA_ARG13(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA14(...) ARG(_VA_ARG14(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA15(...) ARG(_VA_ARG15(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))

#ifndef ABS
#define ABS(Xin) (((Xin)>0)?(Xin):(0-(Xin)))
#endif

#ifndef MAX
#define MAX(Xin1,Xin2) (((Xin1)>(Xin2))?(Xin1):(Xin2))
#endif

#ifndef MIN
#define MIN(Xin1,Xin2) (((Xin1)<(Xin2))?(Xin1):(Xin2))
#endif

#ifdef _MSC_VER
#define __thread __declspec(thread)
#endif

#define EXIT DFLT

typedef uint8_t  U8 ;
typedef int8_t   S8 ;
typedef uint16_t U16;
typedef int16_t  S16;
typedef uint32_t U32;
typedef int32_t  S32;
typedef uint64_t U64;
typedef int64_t  S64;
typedef float    F32;
typedef double   D64;
typedef intptr_t PTR;

#define MORN_TYPE_U8  0
#define MORN_TYPE_S8  1
#define MORN_TYPE_U16 2
#define MORN_TYPE_S16 3
#define MORN_TYPE_U32 4
#define MORN_TYPE_S32 5
#define MORN_TYPE_U64 6
#define MORN_TYPE_S64 7
#define MORN_TYPE_F32 8
#define MORN_TYPE_D64 9
#define MORN_TYPE_P16 10
#define MORN_TYPE_P32 11
#define MORN_TYPE_P64 12
#if(__STDC_VERSION__>=201112)
#define mDataType(Data) _Generic((Data),\
       uint8_t :MORN_TYPE_U8 , int8_t:MORN_TYPE_S8 ,\
      uint16_t :MORN_TYPE_U16,int16_t:MORN_TYPE_S16,\
      uint32_t :MORN_TYPE_U32,int32_t:MORN_TYPE_S32,\
      uint64_t :MORN_TYPE_U64,int64_t:MORN_TYPE_S64,\
      float    :MORN_TYPE_F32,double :MORN_TYPE_D64,\
      char     :((((char)(-1))>0)?MORN_TYPE_U8:MORN_TYPE_S8),\
   signed long :((sizeof(  signed /long)==4)?MORN_TYPE_S32:MORN_TYPE_S64),\
 unsigned long :((sizeof(unsigned long)==4)?MORN_TYPE_U32:MORN_TYPE_U64),\
      uint8_t *:((sizeof(void *)==4)?MORN_TYPE_U32:MORN_TYPE_U64),\
       int8_t *:((sizeof(void *)==4)?MORN_TYPE_U32:MORN_TYPE_U64),\
     uint16_t *:MORN_TYPE_P16,int16_t *:MORN_TYPE_P16,\
     uint32_t *:MORN_TYPE_P32,int32_t *:MORN_TYPE_P32,\
     uint64_t *:MORN_TYPE_P64,int64_t *:MORN_TYPE_P64,\
     float    *:MORN_TYPE_P32,double  *:MORN_TYPE_P64,\
     char     *:((sizeof(void *)==4)?MORN_TYPE_U32:MORN_TYPE_U64),\
  signed long *:((sizeof(  signed long)==4)?MORN_TYPE_P32:MORN_TYPE_P64),\
unsigned long *:((sizeof(unsigned long)==4)?MORN_TYPE_P32:MORN_TYPE_P64),\
default:DFLT)
#else
extern __thread int  morn_data_type;
extern __thread char morn_data_buff[8];
#define mDataType(Data) (\
    memcpy(morn_data_buff,&(Data),sizeof(Data)),\
    Data=-16,\
    morn_data_type= (sizeof(Data)==1)?((Data<0)?MORN_TYPE_S8 :MORN_TYPE_U8 ):(\
                    (sizeof(Data)==2)?((Data<0)?MORN_TYPE_S16:MORN_TYPE_U16):(\
                    (sizeof(Data)==4)?((*((int32_t *)(&(Data)))!=-16)?MORN_TYPE_F32:((Data<0)?MORN_TYPE_S32:(\
                        ((intptr_t)(Data+1)==((intptr_t)Data)+2)?MORN_TYPE_P16:(\
                        ((intptr_t)(Data+1)==((intptr_t)Data)+4)?MORN_TYPE_P32:(\
                        ((intptr_t)(Data+1)==((intptr_t)Data)+8)?MORN_TYPE_P64:(\
                        MORN_TYPE_U32)))))):(\
                    (sizeof(Data)==8)?((*((int64_t *)(&(Data)))!=-16)?MORN_TYPE_D64:((Data<0)?MORN_TYPE_S64:(\
                        ((intptr_t)(Data+1)==((intptr_t)Data)+2)?MORN_TYPE_P16:(\
                        ((intptr_t)(Data+1)==((intptr_t)Data)+4)?MORN_TYPE_P32:(\
                        ((intptr_t)(Data+1)==((intptr_t)Data)+8)?MORN_TYPE_P64:(\
                        MORN_TYPE_U64)))))):\
                    DFLT))),\
    memcpy(&(Data),morn_data_buff,sizeof(Data)),\
    morn_data_type\
)
#endif

extern __thread char morn_filename[256];

#define PrintVal(Name,V,Type) do{\
         if((Type==MORN_TYPE_S8 )||(Type==MORN_TYPE_U8 )) printf("%s=%d(%c), ",Name,(int)V,(char)V);\
    else if((Type==MORN_TYPE_S16)||(Type==MORN_TYPE_U16)||(Type==MORN_TYPE_S32)||(Type==MORN_TYPE_U32)) printf("%s=%d, ",Name,(int)V);\
    else if((Type==MORN_TYPE_S64)||(Type==MORN_TYPE_U64)) printf("%s=%lld(%x), ",Name,(int64_t)V,(int64_t)V);\
    else if( Type==MORN_TYPE_F32) printf("%s=%f, " ,Name,(float )V);\
    else if( Type==MORN_TYPE_D64) printf("%s=%lf, ",Name,(double)V);\
    else if((Type==MORN_TYPE_P16)||(Type==MORN_TYPE_P32)||(Type==MORN_TYPE_P64)) printf("%s=%p,",Name,(void *)((intptr_t)V));\
}while(0)

#define mDebug(...) do{\
    char *Name[16];int Num=0;int Flag=1;\
    strcpy(morn_filename,# __VA_ARGS__);\
    for(char *p=morn_filename;*p!=0;p++)\
    {\
        if((*p==' ')||(*p==',')) {*p=0;Flag=1;continue;}\
        else if(Flag==1) {Name[Num++]=p;Flag=0;}\
    }\
    printf("morn debug: ");\
    if(Num> 0) PrintVal(Name[ 0], VA0(__VA_ARGS__),mDataType( VA0(__VA_ARGS__)));\
    if(Num> 1) PrintVal(Name[ 1], VA1(__VA_ARGS__),mDataType( VA1(__VA_ARGS__)));\
    if(Num> 2) PrintVal(Name[ 2], VA2(__VA_ARGS__),mDataType( VA2(__VA_ARGS__)));\
    if(Num> 3) PrintVal(Name[ 3], VA3(__VA_ARGS__),mDataType( VA3(__VA_ARGS__)));\
    if(Num> 4) PrintVal(Name[ 4], VA4(__VA_ARGS__),mDataType( VA4(__VA_ARGS__)));\
    if(Num> 5) PrintVal(Name[ 5], VA5(__VA_ARGS__),mDataType( VA5(__VA_ARGS__)));\
    if(Num> 6) PrintVal(Name[ 6], VA6(__VA_ARGS__),mDataType( VA6(__VA_ARGS__)));\
    if(Num> 7) PrintVal(Name[ 7], VA7(__VA_ARGS__),mDataType( VA7(__VA_ARGS__)));\
    if(Num> 8) PrintVal(Name[ 8], VA8(__VA_ARGS__),mDataType( VA8(__VA_ARGS__)));\
    if(Num> 9) PrintVal(Name[ 9], VA9(__VA_ARGS__),mDataType( VA9(__VA_ARGS__)));\
    if(Num>10) PrintVal(Name[10],VA10(__VA_ARGS__),mDataType(VA10(__VA_ARGS__)));\
    if(Num>11) PrintVal(Name[11],VA11(__VA_ARGS__),mDataType(VA11(__VA_ARGS__)));\
    if(Num>12) PrintVal(Name[12],VA12(__VA_ARGS__),mDataType(VA12(__VA_ARGS__)));\
    if(Num>13) PrintVal(Name[13],VA13(__VA_ARGS__),mDataType(VA13(__VA_ARGS__)));\
    if(Num>14) PrintVal(Name[14],VA14(__VA_ARGS__),mDataType(VA14(__VA_ARGS__)));\
    if(Num>15) PrintVal(Name[15],VA15(__VA_ARGS__),mDataType(VA15(__VA_ARGS__)));\
    printf("\n");\
}while(0)

const char *mTimeNowString();
const char *m_TimeString(int64_t time_value,const char *format);
#define mTimeString(...) (\
    (VANumber(__VA_ARGS__)==0)?mTimeNowString():\
    (VANumber(__VA_ARGS__)==1)?m_TimeString(DFLT,(const char *)(VA0(__VA_ARGS__))):\
    (VANumber(__VA_ARGS__)==2)?m_TimeString((int64_t)(VA0(__VA_ARGS__)),(const char *)(VA1(__VA_ARGS__))):\
    NULL\
)
int64_t m_StringTime(char *in,const char *format);
#define mStringTime(...) (\
    (VANumber(__VA_ARGS__)==0)?time(NULL):\
    (VANumber(__VA_ARGS__)==1)?m_StringTime((char *)(VA0(__VA_ARGS__)),NULL):\
    (VANumber(__VA_ARGS__)==2)?m_StringTime((char *)(VA0(__VA_ARGS__)),(const char *)(VA1(__VA_ARGS__))):\
    DFLT\
)

void m_Exception(int err,int ID,const char *file,int line,const char *function,const char *message,...);

#define MORN_DEBUG   16
#define MORN_INFO    32
#define MORN_WARNING 48
#define MORN_ERROR   64
const char *mLogLevel();
#define mLogFormat1(Message) "[%s,line %d,function %s]%s: " Message "\n",__FILE__,__LINE__,__FUNCTION__,mLogLevel()
#define mLogFormat2(Message) "[%s]%s: " Message "\n",mTimeNowString(),mLogLevel()
#define mLogFormat3(Message) "[thread%03d]%s: " Message "\n",mThreadID(),mLogLevel()
#define mLogFormat4(Message) "[%s thread%03d]%s: " Message "\n",mTimeNowString(),mThreadID(),mLogLevel()
#define mLogFormat5(Message) "[%s thread%03d %s,line %d,function %s]%s: " Message "\n",mTimeNowString(),mThreadID(),__FILE__,__LINE__,__FUNCTION__,mLogLevel()
extern __thread int morn_log_level;
extern int morn_log_levelset;
void _mLog(int Level,const char *format,...);
#define mLog(Level,...) do{\
    morn_log_level=Level;if(Level>=morn_log_levelset) _mLog(Level,__VA_ARGS__);\
}while(0)

extern __thread int morn_exception;
extern __thread jmp_buf *morn_jump[32];
extern __thread int morn_layer_order;//=-1;
#define mExceptionBegin() do{\
    morn_layer_order = morn_layer_order+1;\
    morn_exception = 0;\
    jmp_buf buf_jump;\
    morn_jump[morn_layer_order] = &buf_jump;\
    if(setjmp(buf_jump)) goto MORN_EXCEPTION_END;\
}while(0)
#define mExceptionEnd()\
{\
    MORN_EXCEPTION_END:\
    morn_layer_order = morn_layer_order-1;\
}
#ifdef _MSC_VER
#define exit(Flag) do{system("pause");exit(Flag);}while(0)
#endif
#define mError(Flag) (Flag)
#define mWarning(Flag) (0-(Flag))
#define mException(Error,ID,...) do{int Err=Error;if(Err!=0) m_Exception(Err,ID,__FILE__,__LINE__,__FUNCTION__,__VA_ARGS__);}while(0)

#ifdef _MSC_VER
// #include <windows.h>
#define mSleep(T) Sleep(T)
#else
#include <unistd.h>
#define mSleep(T) usleep(T*1000)
// #define mSleep(T) _sleep(T)
#endif

double mTime();
void _mTimerBegin(const char *name);
float _mTimerEnd(const char *name,const char *file,int line,const char *function);
#define mTimerBegin(...) _mTimerBegin((const char *)VA0(__VA_ARGS__))
#define mTimerEnd(...) _mTimerEnd((const char *)VA0(__VA_ARGS__),__FILE__,__LINE__,__FUNCTION__)
#define MORN_CHRONO_CYCLE(n) n
#define MORN_CHRONO_ONCE     0
void mChronoFunction(void *function,void *para,int delay,int mode);
void mChrono();


extern __thread char morn_shu[256];
const char *mShu(double data);

#define INVALID_POINTER(p) (((p)==NULL)||(((intptr_t)(p))==DFLT))

unsigned int mHash(const char *in,int size);

// #define Morn struct{struct MList *handle;int device;}
#define Morn struct MList *handle

// #define RESERVE union{void *p_reserve;int64_t i_reserve;double f_reserve;}

typedef struct MList
{
    int num;
    void **data;
}MList;

MList *ListCreate(int num,void **data);
#define mListCreate(...) (\
    (VANumber(__VA_ARGS__)==0)?ListCreate(DFLT,NULL):\
    (VANumber(__VA_ARGS__)==1)?ListCreate((int)(VA0(__VA_ARGS__)),NULL):\
    (VANumber(__VA_ARGS__)==2)?ListCreate((int)(VA0(__VA_ARGS__)),(void **)(VA1(__VA_ARGS__))):\
    NULL\
)
void mListRelease(MList *list);
void m_ListAppend(MList *list,void **data,int num);
#define mListAppend(List,...) do{\
         if(VANumber(__VA_ARGS__)==0) m_ListAppend(List,NULL,DFLT);\
    else if(VANumber(__VA_ARGS__)==1) m_ListAppend(List,NULL,(intptr_t)VA0(__VA_ARGS__));\
    else if(VANumber(__VA_ARGS__)==2) m_ListAppend(List,(void **)((intptr_t)VA0(__VA_ARGS__)),(intptr_t)VA0(__VA_ARGS__));\
}while(0)
void mListPlace(MList *list,void *data,int num,int size);
void mListClear(MList *list);

void *mListWrite(MList *list,int n,void *data,int size);
void *mListRead(MList *list,int n,void *data,int size);

void mListElementDelete(MList *list,int n);
void *mListElementInsert(MList *list,int n,void *data,int size);
void mListCopy(MList *src,MList *dst);
void mListMerge(MList *list1,MList *list2,MList *dst);

void mListElementOperate(MList *list,void *function,void *para);
void mListElementScreen(MList *list,void *function,void *para);
void mListElementSelect(MList *list,void *function,void *para);
int mListCluster(MList *list,int *group,void *function,void *para);
void mListSort(MList *list,void *function,void *para);
void mListReorder(MList *list);

int *m_ListMatch(MList *src,MList *dst,float thresh,void *function,void *para);
#define mListMatch(src,dst,thresh,function,...) m_ListMatch(src,dst,thresh,function,(VANumber(__VA_ARGS__)==0)?NULL:(void *)((intptr_t)VA0(__VA_ARGS__)))

#define MORN_FILE_TYPE 0
#define MORN_DIR_TYPE 16
void mFileList(MList *list,const char *directory,const char *regular);


// extern void **morn_malloc_ptr[256];
// extern int morn_malloc_num[256];
extern __thread void *morn_test;

#ifdef DEBUG
void *MemoryListSet( int  size,const char *file,int line,const char *func);
void MemoryListUnset(void *ptr,const char *file,int line,const char *func);
#define mMalloc(Size)  MemoryListSet(  Size   ,__FILE__,__LINE__,__FUNCTION__)
#define mFree(Pointer) MemoryListUnset(Pointer,__FILE__,__LINE__,__FUNCTION__)
void MemoryListPrint(int state);
#else
void *mMemAlloc(int size);
void mMemFree(void *p);
void *m_Malloc(int size);
void m_Free(void *p);
#define mMalloc(Size)  m_Malloc(Size)
#define mFree(Pointer) m_Free(Pointer)
// void MemoryListPrint(int state);
#endif

int mCompare(const void *mem1,int size1,const void *mem2,int size2);



typedef struct MSheet
{
    // Morn;
    int row;
    int *col;
    void ***data;
    
    // RESERVE;
}MSheet;
MSheet *SheetCreate(int row,int *col,void ***data);
#define mSheetCreate(...) (\
    (VANumber(__VA_ARGS__)==0)?SheetCreate(DFLT,NULL,NULL):\
    (VANumber(__VA_ARGS__)==1)?SheetCreate(VA0(__VA_ARGS__),NULL,NULL):\
    (VANumber(__VA_ARGS__)==2)?SheetCreate(VA0(__VA_ARGS__),(int *)VA1(__VA_ARGS__),NULL):\
    (VANumber(__VA_ARGS__)==3)?SheetCreate(VA0(__VA_ARGS__),(int *)VA1(__VA_ARGS__),(void ***)VA2(__VA_ARGS__)):\
    NULL\
)
void mSheetRelease(MSheet *sheet);
void mSheetClear(MSheet *sheet);
void mSheetRowAppend(MSheet *sheet,int row);
void mSheetColAppend(MSheet *sheet,int row,int col);
void mSheetPlace(MSheet *sheet,void *data,int row,int col,int size);
MList *mSheetRowList(MSheet *sheet,int row);
void *mSheetWrite(MSheet *sheet,int row,int col,void *data,int size);
void *mSheetRead(MSheet *sheet,int row,int col,void *data,int size);
void m_SheetDelete(MSheet *sheet,int row,int col);
#define mSheetDelete(Sheet,...) do{\
    if(VANumber(__VA_ARGS__)==2) m_SheetDelete(Sheet,(int)VA0(__VA_ARGS__),(int)VA1(__VA_ARGS__));\
    else                         m_SheetDelete(Sheet,(int)VA0(__VA_ARGS__),DFLT);\
}while(0)
void *mSheetInsert(MSheet *sheet,int row,int col,void *data,int size);
void mSheetReorder(MSheet *sheet);

MSheet *mListClassify(MList *list,void *function,void *para);

// int ElementSize(const char *str,int size);
// #define mElementSize(Type) ElementSize(#Type,sizeof(Type))

// typedef struct MString

typedef struct MTable{
    // Morn;
    int row;
    int col;
    int element_size;
    union
    {
        void **data;
        unsigned char **dataU8;
        char **dataS8;
        unsigned short **dataU16;
        short **dataS16;
        unsigned int **dataU32;
        int **dataS32;
        int64_t **dataS64;
        uint64_t **dataU64;
        float **dataF32;
        double **dataD64;
        void ***dataptr;
    };
    
    // RESERVE;
}MTable;

MTable *TableCreate(int row,int col,int element_size,void **data);
#define _TableCreate(Row,Col,Size,...) (TableCreate(Row+0,Col,Size,(void **)ARG(_VA0(__VA_ARGS__,DFLT))))
#define mTableCreate(...) (\
    (VANumber(__VA_ARGS__)==0)?TableCreate(DFLT,DFLT,0,NULL):\
    (VANumber(__VA_ARGS__)==2)?TableCreate(VA0(__VA_ARGS__),VA1(__VA_ARGS__),0,NULL):\
    ARG(_TableCreate(__VA_ARGS__,DFLT,DFLT,DFLT))\
)

void mTableRelease(MTable *tab);

void TableRedefine(MTable *tab,int row,int col,int element_size,void **data);
#define _TableRedefine(Tab,Row,Col,Size,...) TableRedefine(Tab+0,Row,Col,Size,(void **)ARG(_VA0(__VA_ARGS__,DFLT)))
#define mTableRedefine(...) do{\
    int Num_Args = VANumber(__VA_ARGS__);\
    mException((Num_Args==0)||(Num_Args==2),EXIT,"invalid input with argument number");\
         if(Num_Args==3) TableRedefine(VA0(__VA_ARGS__),VA1(__VA_ARGS__),VA2(__VA_ARGS__),DFLT,NULL);\
    else if(Num_Args>=4) ARG(_TableRedefine(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT));\
}while(0)

// #define mTableExchange(Tab1,Tab2) mObjectExchange(Tab1,Tab2,MTable)
#define mTableReset(Tab) mHandleReset(Tab->handle)
void mTableCopy(MTable *src,MTable *dst);
void mTableWipe(MTable *tab);

typedef struct MArray{
    // Morn;
    int num;
    short element_size;
    union
    {
        void *data;
        unsigned char *dataU8;
        char *dataS8;
        unsigned short *dataU16;
        short *dataS16;
        unsigned int *dataU32;
        int *dataS32;
        int64_t *dataS64;
        uint64_t *dataU64;
        float *dataF32;
        double *dataD64;
        void **dataptr;
    };
}MArray;
MArray *ArrayCreate(int num,int element_size,void *data);
#define _ArrayCreate(Num,EelementSize,...) (ArrayCreate(Num+0,EelementSize,(void **)ARG(_VA0(__VA_ARGS__,DFLT))))
#define mArrayCreate(...) (\
    (VANumber(__VA_ARGS__)==0)?ArrayCreate(DFLT,0,NULL):\
    (VANumber(__VA_ARGS__)==1)?ArrayCreate(0,VA0(__VA_ARGS__),NULL):\
    _ArrayCreate(__VA_ARGS__,DFLT,DFLT)\
)
void mArrayRelease(MArray *array);
void ArrayRedefine(MArray *array,int num,int element_size,void *data);
#define mArrayRedefine(Array,...) do{\
    int VAN = VANumber(__VA_ARGS__);\
         if(VAN==1) ArrayRedefine(Array,VA0(__VA_ARGS__),DFLT,NULL);\
    else if(VAN==2) ArrayRedefine(Array,VA0(__VA_ARGS__),VA1(__VA_ARGS__),NULL);\
    else if(VAN==3) ArrayRedefine(Array,VA0(__VA_ARGS__),VA1(__VA_ARGS__),VA2(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input with argument number");\
}while(0)
#define mArrayClear(Array) do{Array->num=0;}while(0)
void mArrayElementDelete(MArray *array,int n);
// void ArrayExpand(MArray *array,int n);
void *m_ArrayPushBack(MArray *arr,void *data);
void *m_ArrayWrite(MArray *arr,intptr_t n,void *data);
#define mArrayWrite(Array,...) ((VANumber(__VA_ARGS__)==1)?m_ArrayPushBack(Array,(void *)_VA0(__VA_ARGS__)):m_ArrayWrite(Array,(intptr_t)VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__)))

void *m_ArrayRead(MArray *array,int n,void *data);
#define mArrayRead(Array,...) (\
    (VANumber(__VA_ARGS__)==0)?m_ArrayRead(Array,DFLT,NULL):\
    (VANumber(__VA_ARGS__)==1)?m_ArrayRead(Array,DFLT,(void *)VA0(__VA_ARGS__)):\
    m_ArrayRead(Array,(intptr_t)VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__)))\

int mStreamRead(MArray *buff,void *data,int num);
int mStreamWrite(MArray *buff,void *data,int num);

int m_Rand(int floor,int ceiling);
#define mRand(...) ((VANumber(__VA_ARGS__)==2)?m_Rand((int)VA0(__VA_ARGS__),(int)VA1(__VA_ARGS__)):m_Rand(DFLT,DFLT))
float mNormalRand(float mean,float delta);
int m_RandString(char *str,int l1,int l2);
#define mRandString(Str,...) (\
    (VANumber(__VA_ARGS__)==0)?m_RandString(Str,4,256):\
    (VANumber(__VA_ARGS__)==1)?m_RandString(Str,VA0(__VA_ARGS__),DFLT):\
    (VANumber(__VA_ARGS__)==2)?m_RandString(Str,VA0(__VA_ARGS__),VA1(__VA_ARGS__)):\
    DFLT\
)

#define MString MObject

#define mString(a) #a
int mStringRegular(const char *str1,const char *str2);
MList *mStringSplit(const char *str_in,const char *flag);
void mStringReplace(char *src,char *dst,const char *replace_in,const char *replace_out);
char *m_StringArgument(int argc,char **argv,const char *flag,const char *format,...);
#define mStringArgument(...) m_StringArgument(__VA_ARGS__,NULL)

typedef struct MChainNode
{
    void *data;
    struct MChainNode *prev;
    struct MChainNode *next;
}MChainNode;

typedef struct MTreeNode
{
    void *data;
    int child_num;
    struct MTreeNode **child;
    struct MTreeNode *parent;
}MTreeNode;

typedef struct MBtreeNode
{
    void *data;
    struct MBtreeNode *left;
    struct MBtreeNode *right;
    struct MBtreeNode *parent;
}MBtreeNode;
#define MORN_LEFT  0
#define MORN_RIGHT 1


// typedef struct MObject
// {
//     union
//     {
//         void *object;
//         MTreeNode  *treenode;
//         MBtreeNode *btreenode;
//         MChainNode *chainnode;
//         char *filename;
//         char *string;
//     };
// }MObject;
// MObject *m_ObjectCreate(const void *obj);
// #define mObjectCreate(...) ((VANumber(__VA_ARGS__)==0)?m_ObjectCreate(NULL):m_ObjectCreate(VA0(__VA_ARGS__)))
// void mObjectRelease(MObject *proc);
// void mObjectRedefine(MObject *object,const void *obj);

typedef struct MObject
{
    int size;
    union
    {
         uint8_t *dataU8;
          int8_t *dataS8;
        uint16_t *dataU16;
         int16_t *dataS16;
        uint32_t *dataU32;
         int32_t *dataS32;
        uint64_t *dataU64;
         int64_t *dataS64;
           float *dataF32;
          double *dataD64;
        void *object;
        MTreeNode  *treenode;
        MBtreeNode *btreenode;
        MChainNode *chainnode;
        char *filename;
        char *string;
    };
}MObject;

MObject *m_ObjectCreate(void *p,int size);
#define mObjectCreate(...) (\
    (VANumber(__VA_ARGS__)==0)?m_ObjectCreate(NULL,DFLT):(\
    (VANumber(__VA_ARGS__)==1)?((((intptr_t)VA0(__VA_ARGS__)<=16384)&&((intptr_t)(VA0(__VA_ARGS__))>0))?m_ObjectCreate(NULL,(intptr_t)(VA0(__VA_ARGS__))):m_ObjectCreate((void *)((intptr_t)VA0(__VA_ARGS__)),DFLT)):(\
    (VANumber(__VA_ARGS__)==2)?m_ObjectCreate((void *)((intptr_t)VA0(__VA_ARGS__)),(intptr_t)VA1(__VA_ARGS__)):\
    NULL))\
)

void m_ObjectRedefine(MObject *object,void *p,int size);
#define mObjectRedefine(...) do{\
    int VAN = VANumber(__VA_ARGS__);\
    MObject *Object = (MObject *)((intptr_t)VA0(__VA_ARGS__));\
         if(VAN==1) m_ObjectRedefine(Object,NULL,DFLT);\
    else if(VAN==2) {intptr_t A=(intptr_t)(VA1(__VA_ARGS__)); if((A<=16384)&&(A>0)) {m_ObjectRedefine(Object,NULL,A);} else {m_ObjectRedefine(Object,(void *)A,DFLT);}}\
    else if(VAN==3) m_ObjectRedefine(Object,(void *)VA1(__VA_ARGS__),(intptr_t)VA2(__VA_ARGS__));\
}while(0)

void mObjectRelease(MObject *object);
void *mObjectMemory(MObject *object);

MObject *mMornObject(const void *p,int size);
void mornObjectRemove(void *no_use,char *name);

void HandleExchange(void *obj1,void *obj2);
#define mObjectExchange(Obj1,Obj2,Type) do{\
    Type Obj_buff=*Obj1;*Obj1 = *Obj2;*Obj2 = Obj_buff;\
    HandleExchange(Obj1,Obj2);\
}while(0)

void *mReserve(MObject *obj,int n);

typedef struct MHandle
{
    volatile unsigned int flag;
    volatile int valid;
    void (*destruct)(void *);
    void *handle;
}MHandle;

MList *mHandleCreate(void);
void mHandleRelease(void *p);
void mHandleReset(void *p);
MHandle *GetHandle(void *p,int size,unsigned int hash,void (*end)(void *));
// inline MObject *m_Object(const void *p,int s) {return (MObject *)((s==sizeof(char))?mMornObject(p,DFLT):p);}
#define mObject(P) ((sizeof(P[0])==sizeof(char))?mMornObject(P,DFLT):(MObject *)(P))
#define mHandle(Obj,Func) GetHandle(mObject(Obj),sizeof(struct Handle##Func),HASH_##Func,(void (*)(void *))(end##Func))
#define ObjHandle(Obj,N) ((MHandle *)(((MList **)Obj)[-1]->data[N]))
#define mReset(Obj) mHandleReset(Obj)

#define mFunction(Obj,func,...) func(Obj,__VA_ARGS__)

#define MChain MObject
#define MTree  MObject
#define MBtree MObject
MChain *mChainCreate();
void mChainRelease(MChain *chain);
void mChainClear(MChain *chain);
MChainNode *mChainNode(MChain *chain,void *data,int size);
void mChainNodeInsert(MChainNode *last,MChainNode *node,MChainNode *next);
void mChainNodeDelete(MChain *chain,MChainNode *node);
// void mChainNodeExchange(MChainNode *node1,MChainNode *node2);
void mChainReorder(MChain *chain);
void mChainMerge(MChain *src1,MChain *src2,MChain *dst);
void mChainNodeOperate(MChain *chain,void *function,void *para);

MBtree *mBtreeCreate();
void mBtreeRelease(MBtree *btree);
MBtreeNode *mBtreeNode(MBtree *btree,void *data,int size);
void mBtreeNodeSet(MBtreeNode *node,MBtreeNode *parent,int order);
void mBtreeOperate(MBtree *tree,void (*func)(MBtreeNode *,void *),void *para,int mode);

MTree *mTreeCreate();
void mTreeRelease(MTree *tree);
MTreeNode *mTreeNode(MTree *tree,void *data,int size);
void mTreeNodeSet(MTreeNode *tree,MTreeNode *child,int order);
void mTreeTraversal(MTree *tree,void (*func)(MTreeNode *,void *),void *para,int mode);
MTreeNode *mTreeDecide(MTree *tree,int (*func)(MTreeNode *,void *),void *para);
MTreeNode *mTreeSearch(MTreeNode *node,int (*func)(MTreeNode *,void *),void *para,int mode);

void  m_PropertyVariate(MObject *obj,const char *key,void *var);
void  m_PropertyFunction(MObject *obj,const char *key,void *function,void *para);
void *m_PropertyWrite(MObject *obj,const char *key,const void *value,int value_size);
void *m_PropertyRead(MObject *obj,const char *key,void *value,int *value_size);
#define mPropertyVariate(Obj,Key,Var) m_PropertyVariate(mObject(Obj),Key,Var)
#define mPropertyFunction(Obj,Key,...) do {\
    if(VANumber(__VA_ARGS__)==1) m_PropertyFunction(mObject(Obj),Key,(void *)(_VA0(__VA_ARGS__,DFLT)),NULL);\
    else                         m_PropertyFunction(mObject(Obj),Key,(void *)(_VA0(__VA_ARGS__,DFLT)),(void *)VA1(__VA_ARGS__,DFLT));\
}while(0)
#define mPropertyWrite(Obj,...) (\
    (VANumber(__VA_ARGS__)==1)?m_PropertyWrite(mObject(Obj),(const char *)(_VA0(__VA_ARGS__,DFLT)),NULL,DFLT):\
    (VANumber(__VA_ARGS__)==2)?m_PropertyWrite(mObject(Obj),(const char *)(_VA0(__VA_ARGS__,DFLT)),(const void *)VA1(__VA_ARGS__),DFLT):\
    (VANumber(__VA_ARGS__)==3)?m_PropertyWrite(mObject(Obj),(const char *)(_VA0(__VA_ARGS__,DFLT)),(const void *)VA1(__VA_ARGS__),(int)VA2(__VA_ARGS__)):\
    NULL\
)
#define mPropertyRead(Obj,...) (\
    (VANumber(__VA_ARGS__)==1)?m_PropertyRead(mObject(Obj),(const char *)VA0(__VA_ARGS__),NULL,NULL):\
    (VANumber(__VA_ARGS__)==2)?m_PropertyRead(mObject(Obj),(const char *)VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),NULL):\
    (VANumber(__VA_ARGS__)==3)?m_PropertyRead(mObject(Obj),(const char *)VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),(int *)VA2(__VA_ARGS__)):\
    NULL\
)

#define MORN_DEVICE DFLT
#define MORN_HOST      0
#define MORN_CL_CPU(N)         ((0<<6)+MIN(MAX(N,0),64))
#define MORN_CL_GPU(N)         ((1<<6)+MIN(MAX(N,0),64))
#define MORN_CL_ACCELERATOR(N) ((2<<6)+MIN(MAX(N,0),64))

typedef struct MMemoryBlock
{
    int device;
    int size;
    void *data;
    
    void *cl_data;
    void *cl_evt;
    int flag;
}MMemoryBlock;
MMemoryBlock *mMemoryBlockCreate(int size,int device);
void mMemoryBlockRelease(MMemoryBlock *block);
MMemoryBlock *MemoryBlockRedefine(MMemoryBlock *block,int size);
MMemoryBlock *mMemoryBlock(void *data);

// cl_context mDeviceContext(int device);
// cl_command_queue mDeviceQueue(int device);


void mMemoryBlockWrite(MMemoryBlock *block);
void mMemoryBlockRead(MMemoryBlock *block);
void mMemoryBlockCopy(MMemoryBlock *block,int device);

extern __thread void *morn_cl_function_para[16];
extern __thread int morn_cl_function_para_size[16];
extern __thread int morn_cl_function_para_num;
extern __thread size_t morn_cl_size[4];
extern __thread int morn_cl_dim;
void CLFunction(const char *source,const char *name,int para_num,void **para,int *para_size);
int CLOUT(MMemoryBlock *block);
int CLIN(MMemoryBlock *block);
int CLINOUT(MMemoryBlock *block);
int CLPARA(void *para,int size);
int CLSize(int n,int s1,int s2,int s3,int s4);
#define CLSIZE(...) CLSize(VANumber(__VA_ARGS__),VA0(__VA_ARGS__),VA1(__VA_ARGS__),VA2(__VA_ARGS__),VA3(__VA_ARGS__))
#define mCLFunction(Source,...) do{\
    morn_cl_function_para_num =0;\
    int Para_Num = VANumber(__VA_ARGS__);\
    int N=0;\
    if(Para_Num> 0) {N+=_VA0(__VA_ARGS__,DFLT);}\
    if(Para_Num> 1) {N+= VA1(__VA_ARGS__);}if(Para_Num> 2) {N+= VA2(__VA_ARGS__);}if(Para_Num> 3) {N+= VA3(__VA_ARGS__);}\
    if(Para_Num> 4) {N+= VA4(__VA_ARGS__);}if(Para_Num> 5) {N+= VA5(__VA_ARGS__);}if(Para_Num> 6) {N+= VA6(__VA_ARGS__);}\
    if(Para_Num> 7) {N+= VA7(__VA_ARGS__);}if(Para_Num> 8) {N+= VA8(__VA_ARGS__);}if(Para_Num> 9) {N+= VA9(__VA_ARGS__);}\
    if(Para_Num>10) {N+=VA10(__VA_ARGS__);}if(Para_Num>11) {N+=VA11(__VA_ARGS__);}if(Para_Num>12) {N+=VA12(__VA_ARGS__);}\
    if(Para_Num>13) {N+=VA13(__VA_ARGS__);}if(Para_Num>14) {N+=VA14(__VA_ARGS__);}if(Para_Num>15) {N+=VA15(__VA_ARGS__);}\
    mException((N!=DFLT-Para_Num)||(morn_cl_function_para_num!=DFLT+Para_Num),EXIT,"invalid input");\
    CLFunction(Source,mString(Source),morn_cl_function_para_num,morn_cl_function_para,morn_cl_function_para_size);\
    morn_cl_function_para_num =0;\
}while(0)

#define MMemory MList
MMemory *mMemoryCreate(int num,int size,int dev);
void mMemoryRelease(MMemory *memory);
void mMemoryRedefine(MMemory *memory,int num,int size,int dev);
void mornMemoryDevice(int *dev,MMemory *memory);
void mMemoryDevice(MMemory *memory,int dev);

int mMemorySize(MMemory *memory);
int mMemoryCheck(MMemory *memory,void *check);
void mMemoryClear(MMemory *memory);

void MemoryCollect(void *data,void *mem);
void MemoryDefrag(MMemory *memory);

void *mMemoryAppend(MMemory *memory,int size);
void mMemoryCopy(MMemory *src,void ***isrc,MMemory *dst,void ***idst,int batch,int *num);
void mMemoryMerge(MMemory *mem1,MMemory *mem2,MMemory *dst);

void mMemoryIndex(MMemory *memory,int row,int col_size,void ***index,int num);
void *mMemoryWrite(MMemory *memory,void *data,int size);


#define MMap MObject
MMap *mMapCreate();
void mMapRelease(MMap *map);
void *mornMapWrite(MChain *map,const void *key,int key_size,const void *value,int value_size);
void *m_MapWrite(MMap *map,const void *key,int key_size,const void *value,int value_size);
#define mMapWrite(Map,Key,...) (\
    (VANumber(__VA_ARGS__)==1)?m_MapWrite(Map,(const void *)Key,DFLT,(const void *)((intptr_t)_VA0(__VA_ARGS__,DFLT)),DFLT):\
    (VANumber(__VA_ARGS__)==3)?m_MapWrite(Map,(const void *)Key,(intptr_t)_VA0(__VA_ARGS__,DFLT),(const void *)VA1(__VA_ARGS__),(intptr_t)VA2(__VA_ARGS__)):\
    NULL\
)
void *mornMapRead(MChain *map,const void *key,int key_size,void *value,int *value_size);
void *m_MapRead(MMap *map,const void *key,int key_size,void *value,int *value_size);
#define mMapRead(Map,...) (\
    (VANumber(__VA_ARGS__)==1)?m_MapRead(Map,((const void *)_VA0(__VA_ARGS__,DFLT)),DFLT,NULL,NULL):\
    (VANumber(__VA_ARGS__)==2)?m_MapRead(Map,((const void *)_VA0(__VA_ARGS__,DFLT)),DFLT,(void *)VA1(__VA_ARGS__),NULL):\
    (VANumber(__VA_ARGS__)==4)?m_MapRead(Map,((const void *)_VA0(__VA_ARGS__,DFLT)),(intptr_t)VA1(__VA_ARGS__),(void *)VA2(__VA_ARGS__),(int *)VA3(__VA_ARGS__)):\
    NULL\
)

void mornMapNodeDelete(MChain *map,const void *key,int key_size);
void m_MapNodeDelete(MMap *map,const void *key,int key_size);
#define mMapNodeDelete(Map,...) do{\
         if(VANumber(__VA_ARGS__)==1) m_MapNodeDelete(Map,(const void *)_VA0(__VA_ARGS__,DFLT),DFLT);\
    else if(VANumber(__VA_ARGS__)==2) m_MapNodeDelete(Map,(const void *)_VA0(__VA_ARGS__,DFLT),(intptr_t)VA1(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input parameter for mMapDelete");\
}while(0)
void mornMapNodeOperate(MChain *map,void *function,void *para);
void mMapNodeOperate(MMap *map,void *function,void *para);
void *mornMapNodeKey(MChainNode *node);
void *mornMapNodeValue(MChainNode *node);
int mornMapNodeKeySize(MChainNode *node);
int mornMapNodeValueSize(MChainNode *node);
int mornMapNodeNumber(MChain *map);
int mMapNodeNumber(MMap *map);
void m_SignalFunction(MChain *map,void *sig,int sig_size,void *function,void *para,int para_size);
#define mSignalFunction(...) do{\
    int VN = VANumber(__VA_ARGS__);\
         if(VA==2) m_SignalFunction(NULL,(void *)VA0(__VA_ARGS__),DFLT,(void *)VA1(__VA_ARGS__),NULL,DFLT);\
    else if(VA==3) m_SignalFunction((MChain *)VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),DFLT,(void *)VA2(__VA_ARGS__),NULL,DFLT);\
    else if(VA==5) m_SignalFunction(NULL,(void *)VA0(__VA_ARGS__),(intptr_t)VA1(__VA_ARGS__),(void *)VA2(__VA_ARGS__),(void *)VA3(__VA_ARGS__),(intptr_t)VA4(__VA_ARGS__));\
    else if(VA==6) m_SignalFunction((MChain *)VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),(intptr_t)VA2(__VA_ARGS__),(void *)VA3(__VA_ARGS__),(void *)VA4(__VA_ARGS__),(intptr_t)VA5(__VA_ARGS__));\
    else mException(1,EXIT,"invalid signal function");\
}while(0)

void m_Signal(MChain *map,void *sig,int sig_size,void *data,int data_size);
#define mSignal(...) do{\
    int VN = VANumber(__VA_ARGS__);\
         if(VA==1) m_Signal(NULL,(void *)VA0(__VA_ARGS__),DFLT,NULL,DFLT);\
    else if(VA==2) m_Signal((MChain *)VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),DFLT,NULL,DFLT);\
    else if(VA==4) m_Signal(NULL,(void *)VA0(__VA_ARGS__),(intptr_t)VA1(__VA_ARGS__),(void *)VA2(__VA_ARGS__),(intptr_t)VA3(__VA_ARGS__));\
    else if(VA==5) m_Signal((MChain *)VA0(__VA_ARGS__),(void *)VA1(__VA_ARGS__),(intptr_t)VA2(__VA_ARGS__),(void *)VA3(__VA_ARGS__),(intptr_t)VA4(__VA_ARGS__));\
    else mException(1,EXIT,"invalid signal");\
}while(0)

#define MFile MObject
MFile *mFileCreate(const char *filename,...);
// MFile *m_FileCreate0();
// MFile *m_FileCreate(const char *filename,...);
// #define mFileCreate(...) ((VANumber(__VA_ARGS__)==0)?m_FileCreate0():m_FileCreate(__VA_ARGS__))
#define mFileRelease  mObjectRelease
void mFileRedefine(MFile *file,char *filename,...);
void mFile(MObject *object,const char *file_name,...);

// typedef struct MFileState
// {
//     char redefined;
//     char changed;
//     char reserve1;
//     char reserve2;
// }MFileState;
// #define mFileState(File) ((MFileState *)(File->filename-4))
int fsize(FILE *f);

#define MProc  MObject
#define mProcCreate   mObjectCreate
#define mProcRelease  mObjectRelease
#define mProcRedefine mObjectRedefine

#define MORN_DESKEY 0x6f676c616e726f6d
void mEncrypt(const char *in_name,const char *out_name,uint64_t key);
void mDecrypt(const char *in_name,const char *out_name,uint64_t key);
void mFileEncrypt(MFile *file,uint64_t key);
void mFileDecrypt(MFile *file,uint64_t key);

// extern __thread char *morn_string_result;

void mINIFile(MFile *file);
MList *mINI();
MList *m_INILoad(MFile *file);
#define mINILoad(...) ((VANumber(__VA_ARGS__)==1)?m_INILoad(_VA0(__VA_ARGS__)):m_INILoad(NULL))
char *m_INIRead(MList *ini,const char *section,const char *key,const char *format,...);
#define mINIRead(P,...) ((sizeof(P[0])==sizeof(MList))?m_INIRead((MList *)P,__VA_ARGS__,NULL):m_INIRead(mINI(),(const char *)P,__VA_ARGS__,NULL))
char *m_INIWrite(MList *ini,const char *section,const char *key,const char *format,...);
#define mINIWrite(P,...) ((sizeof(P[0])==sizeof(MList))?m_INIWrite((MList *)P,__VA_ARGS__,NULL):m_INIWrite(mINI(),(const char *)P,__VA_ARGS__,NULL))
void m_INIDelete(MList *ini,const char *section,const char *key);
#define mINIDelete(P,...) do{\
    int VAN = VANumber(__VA_ARGS__);\
    if(sizeof(P[0])==sizeof(MList))\
    {\
        if(VAN==2) m_INIDelete((MList *)(P),(const char *)_VA0(__VA_ARGS__),(const char *)VA1(__VA_ARGS__));\
        else       m_INIDelete((MList *)(P),(const char *)_VA0(__VA_ARGS__),NULL);\
    }\
    else\
    {\
        if(VAN==1) m_INIDelete(mINI(),(const char *)P,(const char *)_VA0(__VA_ARGS__));\
        else       m_INIDelete(mINI(),(const char *)P,NULL);\
    }\
}while(0)
void mINISave(MList *ini,char *filename);
MList *mINIKey(MFile *file,const char *section);

#define JSON_UNKNOWN     0
#define JSON_KEY_UNKNOWN 1
#define JSON_BOOL        2
#define JSON_KEY_BOOL    3
#define JSON_INT         4
#define JSON_KEY_INT     5
#define JSON_DOUBLE      6
#define JSON_KEY_DOUBLE  7
#define JSON_STRING      8
#define JSON_KEY_STRING  9
#define JSON_LIST       10
#define JSON_KEY_LIST   11
#define JSON_ARRAY      12
#define JSON_KEY_ARRAY  13

struct JSONNode
{
    union
    {
        int8_t   dataBool;
        int32_t  dataS32;
        double   dataD64;
        char    *string;
        uint16_t num;
    };
    char *key;
    char type;
};
struct JSONNode *mJSONLoad(MFile *jsonfile);
struct JSONNode *m_JSONRead(struct JSONNode *node,intptr_t v);
#define mJSONRead(...) ((VANumber(__VA_ARGS__)==1)?m_JSONRead(_VA0(__VA_ARGS__),0):m_JSONRead(_VA0(__VA_ARGS__),(intptr_t)VA1(__VA_ARGS__)))


extern char *morn_json_type[15];
#define mJSONNodeType(Node) morn_json_type[MAX(MIN((Node)->type,14),0)]

void mJSONArray(MArray *array,struct JSONNode *node);


void m_JSONDelete0(MArray *json,int n);
void m_JSONDelete1(MArray *json,char *key);
#define mJSONDelete(Json,V) do{if((intptr_t)(V)<Json->num) m_JSONDelete0(Json,(intptr_t)(V));else m_JSONDelete1(Json,(char *)((intptr_t)(V)));}while(0)


int mMORNSize (MFile *file,int ID);
int mMORNRead (MFile *file,int ID,void **data,int num,int size);
int mMORNWrite(MFile *file,int ID,void **data,int num,int size);
    
#define MORN_TREE_PREORDER_TRAVERSAL DFLT
#define MORN_TREE_POSTORDER_TRAVERSAL   1
#define MORN_TREE_INORDER_TRAVERSAL     2

// uint64_t mIPAddress(const char *addr);
// char *mUDPWrite(const char *address,void *data,int size);
// char *mUDPRead(const char *address,void *data,int *size);
char *m_UDPWrite(MObject *obj,const char *address,void *data,int size);
char *m_UDPRead(MObject *obj,const char *address,void *data,int *size);
#define mUDPWrite(...) (\
    (VANumber(__VA_ARGS__)==3)?m_UDPWrite(     mMornObject("UDP",DFLT),(const char *)_VA0(__VA_ARGS__),(void *)((intptr_t)VA1(__VA_ARGS__)),(intptr_t)VA2(__VA_ARGS__)):\
    (VANumber(__VA_ARGS__)==4)?m_UDPWrite((MObject *)_VA0(__VA_ARGS__),(const char *) VA1(__VA_ARGS__),(void *)((intptr_t)VA2(__VA_ARGS__)),(intptr_t)VA3(__VA_ARGS__)):\
    NULL\
)
#define mUDPRead(...) (\
    (VANumber(__VA_ARGS__)==3)?m_UDPRead(     mMornObject("UDP",DFLT),(const char *)_VA0(__VA_ARGS__),(void *)((intptr_t)VA1(__VA_ARGS__)),(int *)((intptr_t)VA2(__VA_ARGS__))):\
    (VANumber(__VA_ARGS__)==4)?m_UDPRead((MObject *)_VA0(__VA_ARGS__),(const char *) VA1(__VA_ARGS__),(void *)((intptr_t)VA2(__VA_ARGS__)),(int *)((intptr_t)VA3(__VA_ARGS__))):\
    NULL\
)

#ifdef __cplusplus
}
#endif

#endif
