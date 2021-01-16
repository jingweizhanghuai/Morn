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

#include <pthread.h>

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

#define ARG(X) X
#define _VA_ARG_NUM(A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,N,M,...) ((N==M+1)?((N==1)?((#A0)[0]!=0):N):DFLT)
#define VA_ARG_NUM(...) ARG(_VA_ARG_NUM(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0,-1))
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
#define VA_ARG0(...)  ARG(_VA_ARG0(__VA_ARGS__,DFLT)+0)
#define VA_ARG1(...)  ARG(_VA_ARG1(__VA_ARGS__,DFLT,DFLT))
#define VA_ARG2(...)  ARG(_VA_ARG2(__VA_ARGS__,DFLT,DFLT,DFLT))
#define VA_ARG3(...)  ARG(_VA_ARG3(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG4(...)  ARG(_VA_ARG4(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG5(...)  ARG(_VA_ARG5(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG6(...)  ARG(_VA_ARG6(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG7(...)  ARG(_VA_ARG7(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG8(...)  ARG(_VA_ARG8(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG9(...)  ARG(_VA_ARG9(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG10(...) ARG(_VA_ARG10(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG11(...) ARG(_VA_ARG11(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG12(...) ARG(_VA_ARG12(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG13(...) ARG(_VA_ARG13(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG14(...) ARG(_VA_ARG14(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))
#define VA_ARG15(...) ARG(_VA_ARG15(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT,DFLT))

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
extern __thread int  morn_data_type;
extern __thread char morn_data_buff[8];
#define mDataType(P) (\
    memcpy(morn_data_buff,(P),sizeof((P)[0])),\
    (P)[0]=-2,\
    morn_data_type=(sizeof((P)[0])==1)?(((int64_t)((P)[0])!=-2)?MORN_TYPE_U8 :MORN_TYPE_S8 ):(\
                   (sizeof((P)[0])==2)?(((int64_t)((P)[0])!=-2)?MORN_TYPE_U16:MORN_TYPE_S16):(\
                   (sizeof((P)[0])==4)?((*((int32_t *)(P))==-2)?(((int64_t)((P)[0])!=-2)?MORN_TYPE_U32:MORN_TYPE_S32):MORN_TYPE_F32):(\
                   (sizeof((P)[0])==8)?((*((int64_t *)(P))==-2)?(((P)[0]/2==-1)?MORN_TYPE_S64:MORN_TYPE_U64):MORN_TYPE_D64):DFLT))),\
    memcpy((P),morn_data_buff,sizeof((P)[0])),\
    morn_data_type\
)

extern __thread char morn_filename[256];

const char *mTimeNowString();
const char *m_TimeString(int64_t time_value,const char *format);
#define mTimeString(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==0)?mTimeNowString():\
    (VA_ARG_NUM(__VA_ARGS__)==1)?m_TimeString(DFLT,(const char *)(VA_ARG0(__VA_ARGS__))):\
    (VA_ARG_NUM(__VA_ARGS__)==2)?m_TimeString((int64_t)(VA_ARG0(__VA_ARGS__)),(const char *)(VA_ARG1(__VA_ARGS__))):\
    NULL\
)
int64_t m_StringTime(char *in,const char *format);
#define mStringTime(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==0)?time(NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==1)?m_StringTime((char *)(VA_ARG0(__VA_ARGS__)),NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==2)?m_StringTime((char *)(VA_ARG0(__VA_ARGS__)),(const char *)(VA_ARG1(__VA_ARGS__))):\
    DFLT\
)


int mThreadID();

void m_Exception(int err,int ID,const char *file,int line,const char *function,const char *message,...);

#define MORN_DEBUG    0
#define MORN_INFO    16
#define MORN_WARNING 32
#define MORN_ERROR   48
#define MORN_LOG_CONSOLE  (~1)
#define MORN_LOG_FILE     (~2)
#define MORN_LOG_CUSTOM   (~4)
#define MORN_LOG_FUNCTION (~8)
void m_LogSet(int levelset,int output,const char *filename,int64_t filesize,void *function,void *para);
#define mLogSet(...) do{\
    int N = VA_ARG_NUM(__VA_ARGS__);\
    if(N==1)\
    {\
        if((intptr_t)VA_ARG0(__VA_ARGS__)<=MORN_ERROR) m_LogSet((intptr_t)VA_ARG0(__VA_ARGS__),DFLT,NULL,DFLT,NULL,NULL);\
        else                                           m_LogSet(DFLT,DFLT,(const char *)VA_ARG0(__VA_ARGS__),DFLT,NULL,NULL);\
    }\
    else if(N==2)\
    {\
        if((intptr_t)VA_ARG0(__VA_ARGS__)>=0) m_LogSet((intptr_t)VA_ARG0(__VA_ARGS__),DFLT,(const char *)VA_ARG1(__VA_ARGS__),DFLT,NULL,NULL);\
        else                                  m_LogSet(DFLT,(intptr_t)VA_ARG0(__VA_ARGS__),(const char *)VA_ARG1(__VA_ARGS__),DFLT,NULL,NULL);\
    }\
    else if(N==3) m_LogSet((intptr_t)VA_ARG0(__VA_ARGS__),(intptr_t)VA_ARG1(__VA_ARGS__),(const char *)VA_ARG2(__VA_ARGS__),DFLT,NULL,NULL);\
    else if(N==4)\
    {\
        int Output = (intptr_t)VA_ARG1(__VA_ARGS__);\
        if((Output|MORN_LOG_FUNCTION)==MORN_LOG_FUNCTION)\
            m_LogSet((intptr_t)VA_ARG0(__VA_ARGS__),Output,    (const char *)VA_ARG2(__VA_ARGS__),(int64_t)VA_ARG3(__VA_ARGS__),NULL,NULL);\
        else\
            m_LogSet((intptr_t)VA_ARG0(__VA_ARGS__),Output,NULL,DFLT,(void *)VA_ARG2(__VA_ARGS__),(void *)VA_ARG3(__VA_ARGS__));\
    }\
    else if(N==6) m_LogSet((intptr_t)VA_ARG0(__VA_ARGS__),(intptr_t)VA_ARG1(__VA_ARGS__),(const char *)VA_ARG2(__VA_ARGS__),(int64_t)VA_ARG3(__VA_ARGS__),(void *)VA_ARG4(__VA_ARGS__),(void *)VA_ARG5(__VA_ARGS__));\
    else m_Exception(1,EXIT,__FILE__,__LINE__,__FUNCTION__,"invalid log set");\
}while(0)
const char *mLogLevel();
#define mLogFormat1(Message) "[%s,line %d,function %s]%s: " Message "\n",__FILE__,__LINE__,__FUNCTION__,mLogLevel()
#define mLogFormat2(Message) "[%s]%s: " Message "\n",mTimeNowString(),mLogLevel()
#define mLogFormat3(Message) "[thread%03d]%s: " Message "\n",mThreadID(),mLogLevel()
#define mLogFormat4(Message) "[%s thread%03d]%s: " Message "\n",mTimeNowString(),mThreadID(),mLogLevel()
#define mLogFormat5(Message) "[%s thread%03d %s,line %d,function %s]%s: " Message "\n",mTimeNowString(),mThreadID(),__FILE__,__LINE__,__FUNCTION__,mLogLevel()
#define mLogFormat(N,Message) mLogFormat##N(Message)
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

void _mTimerBegin(const char *name);
float _mTimerEnd(const char *name,const char *file,int line,const char *function);
#define mTimerBegin(...) _mTimerBegin((const char *)VA_ARG0(__VA_ARGS__))
#define mTimerEnd(...) _mTimerEnd((const char *)VA_ARG0(__VA_ARGS__),__FILE__,__LINE__,__FUNCTION__)

extern __thread char morn_shu[256];
const char *mShu(double data);

#define INVALID_POINTER(p) (((p)==NULL)||(((intptr_t)(p))==DFLT))

typedef struct MInfo
{
    char name[8][16];
    float value[8];
}MInfo;
float mInfoGet(MInfo *info,const char *name);
void mInfoSet(MInfo *info,const char *name,float value);

unsigned int mHash(const char *in,int size);

// struct Morn
// {
//     int device;
//     struct MList *handle;
//     MInfo info;
// };

#define Morn struct{int device;struct MList *handle;MInfo info;}

typedef struct MList
{
    int num;
    void **data;
    Morn;
    void *reserve;
}MList;

MList *ListCreate(int num,void **data);
#define mListCreate(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==0)?ListCreate(DFLT,NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==1)?ListCreate((int)(VA_ARG0(__VA_ARGS__)),NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==2)?ListCreate((int)(VA_ARG0(__VA_ARGS__)),(void **)(VA_ARG1(__VA_ARGS__))):\
    NULL\
)
void mListRelease(MList *list);
void mListAppend(MList *list,int num);
void mListPlace(MList *list,void *data,int num,int size);
void mListClear(MList *list);
// #define mListClear(List) do{List->num = 0;}while(0)

void *mListWrite(MList *list,int n,void *data,int size);
void *mListRead(MList *list,int n,void *data,int size);

void mListElementDelete(MList *list,int n);
void mListElementInsert(MList *list,int n,void *data,int size);
void mListCopy(MList *src,MList *dst);
void mListMerge(MList *list1,MList *list2,MList *dst);

void mListElementOperate(MList *list,void *function,void *para);
void mListElementScreen(MList *list,void *function,void *para);
void mListElementSelect(MList *list,void *function,void *para);
int mListCluster(MList *list,int *group,void *function,void *para);
void mListSort(MList *list,void *function,void *para);
void mListReorder(MList *list);

int mQueueSize(MList *queue);
void *mQueueWrite(MList *queue,void *data,int size);
void *mQueueRead(MList *queue,void *data,int size);

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

/*
#define mPointer(Pointer,Size) do{\
    int Size0;\
    if(Pointer == NULL)\
    {\
        Pointer = mMalloc(Ssize);\
    }\
    else\
    {\
        Size0 = *(((int *)Pointer)-1);\
        if(Size>Size0)\
        {\
            mFree(Pointer);\
            Pointer = mMalloc(Size);\
        }\
    }\
}while(0)
*/

    
int mCompare(const void *mem1,int size1,const void *mem2,int size2);

#define mObjectExchange(Obj1,Obj2,Type) do{\
    Type Obj_buff;\
    Obj_buff = *Obj1;\
    *Obj1 = *Obj2;\
    *Obj2 = Obj_buff;\
    \
    Obj2->handle = Obj1->handle;\
    Obj1->handle = Obj_buff.handle;\
    \
    MHandle *Hdl1= (MHandle *)(Obj1->handle->data[0]);\
    MHandle *Hdl2= (MHandle *)(Obj2->handle->data[0]);\
    Obj1->handle->data[0] = Hdl2;\
    Obj2->handle->data[0] = Hdl1;\
    \
    void *Object_buff;\
    Object_buff = *(void **)(Hdl1->handle);\
    *(void **)(Hdl1->handle) = *(void **)(Hdl2->handle);\
    *(void **)(Hdl2->handle) = Object_buff;\
}while(0)

typedef struct MSheet
{
    int row;
    int *col;
    void ***data;
    MInfo *row_info;
    
    Morn;
    void *reserve;
}MSheet;
MSheet *SheetCreate(int row,int *col,void ***data);
#define mSheetCreate(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==0)?SheetCreate(DFLT,NULL,NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==1)?SheetCreate(VA_ARG0(__VA_ARGS__),NULL,NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==2)?SheetCreate(VA_ARG0(__VA_ARGS__),(int *)VA_ARG1(__VA_ARGS__),NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==3)?SheetCreate(VA_ARG0(__VA_ARGS__),(int *)VA_ARG1(__VA_ARGS__),(void ***)VA_ARG2(__VA_ARGS__)):\
    NULL\
)
void mSheetRelease(MSheet *sheet);
void mSheetClear(MSheet *sheet);
void mSheetRowAppend(MSheet *sheet,int row);
void mSheetColAppend(MSheet *sheet,int row,int col);
void mSheetPlace(MSheet *sheet,void *data,int row,int col,int size);
void *mSheetWrite(MSheet *sheet,int row,int col,void *data,int size);
void *mSheetRead(MSheet *sheet,int row,int col,void *data,int size);
void mSheetElementDelete(MSheet *sheet,int row,int col);
void mSheetElementInsert(MSheet *sheet,int row,int col,void *data,int size);
void mSheetReorder(MSheet *sheet);

MSheet *mListClassify(MList *list,void *function,void *para);

// int ElementSize(const char *str,int size);
// #define mElementSize(Type) ElementSize(#Type,sizeof(Type))

typedef struct MTable{
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
        float **dataF32;
        double **dataD64;
        void ***dataptr;
    };
    
    Morn;
    void *reserve;
}MTable;

MTable *TableCreate(int row,int col,int element_size,void **data);
#define _TableCreate(Row,Col,Size,...) (TableCreate(Row+0,Col,Size,(void **)ARG(_VA_ARG0(__VA_ARGS__,DFLT))))
#define mTableCreate(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==0)?TableCreate(DFLT,DFLT,0,NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==2)?TableCreate(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),0,NULL):\
    ARG(_TableCreate(__VA_ARGS__,DFLT,DFLT,DFLT))\
)

void mTableRelease(MTable *tab);

void TableRedefine(MTable *tab,int row,int col,int element_size,void **data);
#define _TableRedefine(Tab,Row,Col,Size,...) TableRedefine(Tab+0,Row,Col,Size,(void **)ARG(_VA_ARG0(__VA_ARGS__,DFLT)))
#define mTableRedefine(...) do{\
    int Num_Args = VA_ARG_NUM(__VA_ARGS__);\
    mException((Num_Args==0)||(Num_Args==2),EXIT,"invalid input with argument number");\
         if(Num_Args==3) TableRedefine(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__),DFLT,NULL);\
    else if(Num_Args>=4) ARG(_TableRedefine(__VA_ARGS__,DFLT,DFLT,DFLT,DFLT));\
}while(0)

#define mTableExchange(Tab1,Tab2) mObjectExchange(Tab1,Tab2,MTable)
#define mTableReset(Tab) mHandleReset(Tab->handle)

typedef struct MArray{
    int num;
    int element_size;
    union
    {
        void *data;
        unsigned char *dataU8;
        char *dataS8;
        unsigned short *dataU16;
        short *dataS16;
        unsigned int *dataU32;
        int *dataS32;
        float *dataF32;
        double *dataD64;
        void **dataptr;
    };
    
    Morn;
    void *reserve;
}MArray;
MArray *ArrayCreate(int num,int element_size,void *data);
#define _ArrayCreate(Num,EelementSize,...) (ArrayCreate(Num+0,EelementSize,(void **)ARG(_VA_ARG0(__VA_ARGS__,DFLT))))
#define mArrayCreate(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==0)?ArrayCreate(DFLT,0,NULL):\
    (VA_ARG_NUM(__VA_ARGS__)==1)?ArrayCreate(VA_ARG0(__VA_ARGS__),0,NULL):\
    _ArrayCreate(__VA_ARGS__,DFLT,DFLT)\
)
void mArrayRelease(MArray *array);
void ArrayRedefine(MArray *array,int num,int element_size,void *data);
#define _ArrayRedefine(Array,Num,EelementSize,...) (ArrayRedefine(Array+0,Num,EelementSize,(void **)ARG(_VA_ARG0(__VA_ARGS__,DFLT))));
#define mArrayRedefine(...) do{\
    int Num_Args = VA_ARG_NUM(__VA_ARGS__);\
    mException((Num_Args==0),EXIT,"invalid input with argument number");\
    if(Num_Args==2) ArrayRedefine(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),DFLT,NULL);\
    else if(Num_Args>=3) ARG(_ArrayRedefine(__VA_ARGS__,DFLT,DFLT,DFLT));\
}while(0)

int mStreamRead(MArray *buff,void *data,int num);
int mStreamWrite(MArray *buff,void *data,int num);

int m_Rand(int floor,int ceiling);
#define mRand(...) ((VA_ARG_NUM(__VA_ARGS__)==2)?m_Rand((int)VA_ARG0(__VA_ARGS__),(int)VA_ARG1(__VA_ARGS__)):m_Rand(DFLT,DFLT))
float mNormalRand(float mean,float delta);
int m_RandString(char *str,int l1,int l2);
#define mRandString(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==1)?m_RandString(VA_ARG0(__VA_ARGS__),4,256):\
    (VA_ARG_NUM(__VA_ARGS__)==2)?m_RandString(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),DFLT):\
    (VA_ARG_NUM(__VA_ARGS__)==3)?m_RandString(VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__)):\
    DFLT\
)

#define mString(a) #a
int mStringRegular(const char *str1,const char *str2);
MList *mStringSplit(const char *str_in,const char *flag);
void mStringReplace(char *src,char *dst,const char *replace_in,const char *replace_out);
char *m_StringArgument(int argc,char **argv,const char *flag,char *format,...);
#define mStringArgument(...) m_StringArgument(__VA_ARGS__,NULL)

typedef struct MChainNode
{
    void *data;
    struct MChainNode *last;
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


typedef struct MObject
{
    union
    {
        void *object;
        MTreeNode  *treenode;
        MBtreeNode *btreenode;
        MChainNode *chainnode;
        char *filename;
    };
    
    Morn;
    void *reserved;
}MObject;
MObject *mObjectCreate(const void *obj);
void mObjectRelease(MObject *proc);
void mObjectRedefine(MObject *object,const void *obj);

MObject *mMornObject(void *p,int size);

typedef struct MHandle
{
    unsigned int flag;
    int valid;
    void *handle;
    void (*destruct)(void *);
}MHandle;

MList *mHandleCreate(void);
void mHandleRelease(MList *handle);
void mHandleReset(MList *handle);
MHandle *GetHandle(MList *handle,int size,unsigned int hash,void (*end)(void *));
// #define _Object(obj) ((obj==NULL)?(((void **)(&obj)==(void **)(&morn_object))?(obj=(void *)mObjectCreate(NULL)):NULL):obj)
#define mHandle(Obj,Func) GetHandle((Obj)->handle,sizeof(struct Handle##Func),HASH_##Func,(void (*)(void *))(end##Func))
#define mReset(Obj) mHandleReset(Obj->handle)


#define mFunction(Obj,func,...) func(Obj,__VA_ARGS__)

#define MChain MObject
#define MTree  MObject
#define MBtree MObject
MChain *mChainCreate();
void mChainRelease(MChain *chain);
MChainNode *mChainNode(MChain *chain,void *data,int size);
void mChainNodeInsert(MChainNode *last,MChainNode *node,MChainNode *next);
void mChainNodeDelete(MChain *chain,MChainNode *node);
// void mChainNodeExchange(MChainNode *node1,MChainNode *node2);
void mChainReorder(MChain *chain);
void mChainMerge(MChain *src1,MChain *src2,MChain *dst);

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

#define MORN_DEVICE DFLT
#define MORN_HOST      0
#define MORN_CL_CPU(N)         ((0<<6)+MIN(MAX(N,0),64))
#define MORN_CL_GPU(N)         ((1<<6)+MIN(MAX(N,0),64))
#define MORN_CL_ACCELERATOR(N) ((2<<6)+MIN(MAX(N,0),64))

typedef struct MMemoryBlock
{
    void *data;
    int size;
    int device;
    
    void *cl_data;//cl_mem cl_data;
    void *cl_evt; //cl_event cl_evt;
    int flag;
}MMemoryBlock;
MMemoryBlock *mMemoryBlockCreate(int size,int device);
void mMemoryBlockRelease(MMemoryBlock *block);
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
#define CLSIZE(...) CLSize(VA_ARG_NUM(__VA_ARGS__),VA_ARG0(__VA_ARGS__),VA_ARG1(__VA_ARGS__),VA_ARG2(__VA_ARGS__),VA_ARG3(__VA_ARGS__))
#define mCLFunction(Source,...) do{\
    morn_cl_function_para_num =0;\
    int Para_Num = VA_ARG_NUM(__VA_ARGS__);\
    int N=0;\
    if(Para_Num> 0) {N+=_VA_ARG0(__VA_ARGS__,DFLT);}\
    if(Para_Num> 1) {N+= VA_ARG1(__VA_ARGS__);}if(Para_Num> 2) {N+= VA_ARG2(__VA_ARGS__);}if(Para_Num> 3) {N+= VA_ARG3(__VA_ARGS__);}\
    if(Para_Num> 4) {N+= VA_ARG4(__VA_ARGS__);}if(Para_Num> 5) {N+= VA_ARG5(__VA_ARGS__);}if(Para_Num> 6) {N+= VA_ARG6(__VA_ARGS__);}\
    if(Para_Num> 7) {N+= VA_ARG7(__VA_ARGS__);}if(Para_Num> 8) {N+= VA_ARG8(__VA_ARGS__);}if(Para_Num> 9) {N+= VA_ARG9(__VA_ARGS__);}\
    if(Para_Num>10) {N+=VA_ARG10(__VA_ARGS__);}if(Para_Num>11) {N+=VA_ARG11(__VA_ARGS__);}if(Para_Num>12) {N+=VA_ARG12(__VA_ARGS__);}\
    if(Para_Num>13) {N+=VA_ARG13(__VA_ARGS__);}if(Para_Num>14) {N+=VA_ARG14(__VA_ARGS__);}if(Para_Num>15) {N+=VA_ARG15(__VA_ARGS__);}\
    mException((N!=DFLT-Para_Num)||(morn_cl_function_para_num!=DFLT+Para_Num),EXIT,"invalid input");\
    CLFunction(Source,mString(Source),morn_cl_function_para_num,morn_cl_function_para,morn_cl_function_para_size);\
    morn_cl_function_para_num =0;\
}while(0)

#define MMemory MList
MMemory *mMemoryCreate(int num,int size,int dev);
void mMemoryRelease(MMemory *memory);
void mMemoryRedefine(MMemory *memory,int num,int size,int dev);
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

void *m_MapWrite(MChain *map,const void *key,int key_size,const void *value,int value_size);
#define mMapWrite(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==3)?m_MapWrite(VA_ARG0(__VA_ARGS__),(const void *)VA_ARG1(__VA_ARGS__),DFLT,(const void *)VA_ARG2(__VA_ARGS__),DFLT):\
    (VA_ARG_NUM(__VA_ARGS__)==5)?m_MapWrite(VA_ARG0(__VA_ARGS__),(const void *)VA_ARG1(__VA_ARGS__),(intptr_t)VA_ARG2(__VA_ARGS__),(const void *)VA_ARG3(__VA_ARGS__),(intptr_t)VA_ARG4(__VA_ARGS__)):\
    NULL\
)
void *m_MapRead(MChain *map,const void *key,int key_size,void *value,int value_size);
#define mMapRead(...) (\
    (VA_ARG_NUM(__VA_ARGS__)==2)?m_MapRead(VA_ARG0(__VA_ARGS__),(const void *)VA_ARG1(__VA_ARGS__),DFLT,NULL,DFLT):\
    (VA_ARG_NUM(__VA_ARGS__)==3)?m_MapRead(VA_ARG0(__VA_ARGS__),(const void *)VA_ARG1(__VA_ARGS__),DFLT,(void *)VA_ARG2(__VA_ARGS__),DFLT):\
    (VA_ARG_NUM(__VA_ARGS__)==5)?m_MapRead(VA_ARG0(__VA_ARGS__),(const void *)VA_ARG1(__VA_ARGS__),(intptr_t)VA_ARG2(__VA_ARGS__),(void *)VA_ARG3(__VA_ARGS__),(intptr_t)VA_ARG4(__VA_ARGS__)):\
    NULL\
)
void m_MapDelete(MChain *map,const void *key,int key_size);
#define mMapNodeDelete(...) do{\
         if(VA_ARG_NUM(__VA_ARGS__)==2) m_MapDelete(VA_ARG0(__VA_ARGS__),(const void *)VA_ARG1(__VA_ARGS__),DFLT);\
    else if(VA_ARG_NUM(__VA_ARGS__)==3) m_MapDelete(VA_ARG0(__VA_ARGS__),(const void *)VA_ARG1(__VA_ARGS__),(intptr_t)VA_ARG2(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input parameter for mMapDelete");\
}while(0)
void mMapNodeOperate(MChain *map,void *function,void *para);
void *mMapNodeKey(MChainNode *node);
void *mMapNodeValue(MChainNode *node);
int mMapNodeKeySize(MChainNode *node);
int mMapNodeValueSize(MChainNode *node);
int mMapNodeNumber(MChain *map);


#define MFile  MObject
MFile *mFileCreate(const char *filename,...);
#define mFileRelease  mObjectRelease
void mFileRedefine(MFile *file,char *filename,...);
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

extern __thread char *morn_string_result;
char *m_INIRead(MObject *file,const char *section,const char *key,const char *format,...);
#define mINIRead(...) m_INIRead(__VA_ARGS__,NULL)

MList *mINIKey(MFile *file,const char *section);
MList *mINISection(MFile *file);

void mJSONLoad(MTree *tree,char *filename,...);
char *mJSONName(MTreeNode *node);
char *mJSONValue(MTreeNode *node);
void mJSONSearch(MTree *tree,MList *result,char *name);
MTreeNode *m_JSONNode(MTreeNode *treenode,const char *name,const char *format,...);
#define mJSONNode(...) m_JSONNode(__VA_ARGS__,NULL)

int mMORNSize (MFile *file,int ID);
int mMORNRead (MFile *file,int ID,void **data,int num,int size);
int mMORNWrite(MFile *file,int ID,void **data,int num,int size);
    
#define MORN_TREE_PREORDER_TRAVERSAL DFLT
#define MORN_TREE_POSTORDER_TRAVERSAL   1
#define MORN_TREE_INORDER_TRAVERSAL     2

void m_ThreadPool(MList *pool,void (*func)(void *),void *para,int para_size,int *flag,float priority);
#define mThreadPool(Pool,Func,...) do{\
    int VAN=VA_ARG_NUM(__VA_ARGS__);\
         if(VAN==0) m_ThreadPool(Pool,Func,NULL,0,NULL,0.0f);\
    else if(VAN==2) m_ThreadPool(Pool,Func,(void *)VA_ARG0(__VA_ARGS__),(int)VA_ARG1(__VA_ARGS__),NULL,0.0f);\
    else if(VAN==3) m_ThreadPool(Pool,Func,(void *)VA_ARG0(__VA_ARGS__),(int)VA_ARG1(__VA_ARGS__),(int *)VA_ARG2(__VA_ARGS__),0.0f);\
    else if(VAN==4) m_ThreadPool(Pool,Func,(void *)VA_ARG0(__VA_ARGS__),(int)VA_ARG1(__VA_ARGS__),(int *)VA_ARG2(__VA_ARGS__),(float)VA_ARG3(__VA_ARGS__));\
    else mException(1,EXIT,"invalid input");\
}while(0)

// uint64_t mIPAddress(const char *addr);
int mUDPSend(const char *address,void *data,int size);
int mUDPRecive(const char *address,void *data,int size);

void mNULL(int *p);
// #if defined(__cplusplus)
#define THREAD_FUNC(F,P) pthread_create(TID++,NULL,(void *(*)(void *))(F),(void *)(P))
// #elif defined(__GNUC__)
// #define THREAD_FUNC(F,...) do{void *thfunc(void *para){F(__VA_ARGS__);return NULL;}printf("1111\n");pthread_create(TID++,NULL,thfunc,NULL);printf("1221\n");}while(0)
// #else
// #define THREAD_FUNC(F,P) pthread_create(TID++,NULL,(void *(*)(void *))(F),(void *)(P))
// #endif
#define _mThread(N,F0,F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,F13,F14,F15,...) do{\
    mException(N<2,EXIT,"invalid Thread number");\
    pthread_t ThreadID[16];\
    pthread_t *TID = ThreadID;\
    THREAD_FUNC F0;THREAD_FUNC F1;\
    if(N> 2) THREAD_FUNC F2;if(N> 3) THREAD_FUNC F3 ;if(N> 4) THREAD_FUNC F4 ;if(N> 5) THREAD_FUNC F5 ;if(N> 6) THREAD_FUNC F6 ;if(N> 7) THREAD_FUNC F7 ;if(N> 8) THREAD_FUNC F8 ;\
    if(N> 9) THREAD_FUNC F9;if(N>10) THREAD_FUNC F10;if(N>11) THREAD_FUNC F11;if(N>12) THREAD_FUNC F12;if(N>13) THREAD_FUNC F13;if(N>14) THREAD_FUNC F14;if(N>15) THREAD_FUNC F15;\
    for(int I=0;I<N;I++) {pthread_join(ThreadID[I],NULL);}\
}while(0)
#define mThread(...) ARG(_mThread(VA_ARG_NUM(__VA_ARGS__),__VA_ARGS__,(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL),(mNULL,NULL)))

void *mProcTopicWrite(const char *msgname,void *data,int size);
void *mProcTopicRead(const char *msgname,void *data,int *size);
void *mProcMessageWrite(const char *dstname,void *data,int size);
void *mProcMessageRead(const char *dstname,void *data,int *size);


#ifdef __cplusplus
}
#endif

#endif
