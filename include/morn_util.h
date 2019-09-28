/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _MORN_UNIVERSAL_H_
#define _MORN_UNIVERSAL_H_

#include <time.h>
#include <stdint.h>
#include <setjmp.h>

#include <pthread.h>

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

#define MORN_FAIL    0
#define MORN_SUCCESS 1

typedef uint8_t  U8;
typedef int8_t   S8;
typedef uint16_t U16;
typedef int16_t  S16;
typedef uint32_t U32;
typedef int32_t  S32;
typedef float    F32;
typedef double   D64;
typedef intptr_t PTR;

#ifndef ABS
#define ABS(Xin) (((Xin)>0)?(Xin):(0-(Xin)))
#endif

#ifndef MAX
#define MAX(Xin1,Xin2) (((Xin1)>(Xin2))?(Xin1):(Xin2))
#endif

#ifndef MIN
#define MIN(Xin1,Xin2) (((Xin1)<(Xin2))?(Xin1):(Xin2))
#endif

#define MORN_LOG_WARNING DFLT
#define MORN_LOG_INFO    0
#define MORN_LOG_ERROR   1
#ifndef WARNING
#define WARNING MORN_LOG_WARNING
#endif
#ifndef INFO
#define INFO    MORN_LOG_INFO
#endif
#ifndef ERROR
#define ERROR   MORN_LOG_ERROR
#endif
extern pthread_mutex_t file_mutex;
extern int morn_log_level;
extern FILE *morn_log_f;
extern int *morn_log_count;
#define mLog(Flag,...) do{\
    if((morn_log_level==MORN_LOG_INFO)||(Flag>0)||\
      ((morn_log_level==MORN_LOG_WARNING)&&(Flag!=0)))\
    {\
        char info1[256];\
        if(Flag<0) sprintf(info1,"[%s,line %d]Warning: in function %s: ",__FILE__,__LINE__,__FUNCTION__);\
        else if(Flag>0) sprintf(info1,"[%s,line %d]Error: in function %s: ",__FILE__,__LINE__,__FUNCTION__);\
        else info1[0]=0;\
        \
        char info2[1024];\
        sprintf(info2,__VA_ARGS__);\
        if(morn_log_f==NULL)\
            printf("%s%s\n\n",info1,info2);\
        else\
        {\
            pthread_mutex_lock(&file_mutex);\
            fprintf(morn_log_f,"%s%s\n\n",info1,info2);\
            (*morn_log_count)+=1;\
            if(*morn_log_count>=16)\
            {\
                fflush(morn_log_f);\
                (*morn_log_count) = 0;\
            }\
            pthread_mutex_unlock(&file_mutex);\
        }\
    }\
}while(0)

#ifdef _MSC_VER
#define mSleep(T) Sleep(T)
#else
// void usleep(int micro_seconds);
#define mSleep(T) usleep(T*1000)
#endif
    
#ifdef _MSC_VER
extern int morn_clock_n;
extern int morn_clock_begin[16];
extern int morn_clock_end[16];
#define mTimerBegin() do{\
    morn_clock_n+= 1;\
    morn_clock_begin[morn_clock_n]=clock();\
}while(0)
#define mTimerEnd() do{\
    morn_clock_end[morn_clock_n] =clock();\
    float Use = ((float)(morn_clock_end[morn_clock_n]-morn_clock_begin[morn_clock_n]))*1000.0f/((float)CLOCKS_PER_SEC);\
    mException((Use<0.0f),EXIT,"invalid timer");\
    morn_clock_n -= 1;\
    mLog(INFO,"[%s,line %d]Timer: in function %s: time use is %fms",__FILE__,__LINE__,__FUNCTION__,Use);\
}while(0)
#else
#include <sys/time.h>
extern int morn_timer_n;
extern struct timeval morn_timer_begin[16];
extern struct timeval morn_timer_end[16];
#ifdef __MINGW32__
#define gettimeofday mingw_gettimeofday
#endif
#define mTimerBegin() do{\
    morn_timer_n+= 1;\
    gettimeofday(&morn_timer_begin[morn_timer_n],NULL);\
}while(0)
#define mTimerEnd() do{\
    gettimeofday(&morn_timer_end[morn_timer_n],NULL);\
    float Use = (morn_timer_end[morn_timer_n].tv_sec - morn_timer_begin[morn_timer_n].tv_sec)*1000.0f + (morn_timer_end[morn_timer_n].tv_usec - morn_timer_begin[morn_timer_n].tv_usec)/1000.0f;\
    mException((Use<0.0f),EXIT,"invalid timer");\
    morn_timer_n -= 1;\
    mLog(INFO,"[%s,line %d]Timer: in function %s: time use is %fms",__FILE__,__LINE__,__FUNCTION__,Use);\
}while(0)
#endif

#define EXIT DFLT
extern int morn_layer_order[16];// = -1;
extern int morn_exception;// = 0;
extern jmp_buf *morn_jump[16][8];
extern pthread_t morn_pthread_ID[16];
#define PthreadOrder(Thread_Order) do{\
    pthread_t Thread_ID = pthread_self();\
    for(Thread_Order=0;Thread_Order<16;Thread_Order++)\
    {\
        if(morn_layer_order[Thread_Order]>=0)\
            if(memcmp(&Thread_ID,morn_pthread_ID+Thread_Order,sizeof(pthread_t))==0)\
                break;\
    }\
    if(Thread_Order == 16)\
    {\
        for(Thread_Order=0;Thread_Order<16;Thread_Order++)\
            if(morn_layer_order[Thread_Order]<0)\
                {morn_pthread_ID[Thread_Order] = Thread_ID;break;}\
    }\
    if(Thread_Order == 16)\
    {\
        printf("[%s,line %d]Error: in function mExceptionBegin: ",__FILE__,__LINE__);\
        printf("invalid thread ID\n");\
        exit(0);\
    }\
}while(0)

#define mExceptionBegin()\
int Thread_Order; PthreadOrder(Thread_Order);\
int Layer_order = morn_layer_order[Thread_Order]+1;\
{\
    morn_layer_order[Thread_Order] = Layer_order;\
    jmp_buf buf_jump;\
    morn_jump[Thread_Order][Layer_order] = &buf_jump;\
    if(setjmp(buf_jump))\
        goto MORN_EXCEPTION_END;\
}

#define mExceptionEnd()\
{\
    MORN_EXCEPTION_END:\
    morn_layer_order[Thread_Order] = Layer_order-1;\
}

#ifdef _MSC_VER
#define exit(Flag) do{system("pause");exit(Flag);}while(0)
#endif
#define mError(Flag) (Flag)
#define mWarning(Flag) (0-(Flag))
#define mException(ERROR,ID,...) do{\
    int Err = ERROR;\
    if(Err!=0) mLog(Err,__VA_ARGS__);\
    if(Err > 0)\
    {\
        morn_exception = ID;\
        int Thread_order; PthreadOrder(Thread_order);\
        if(morn_layer_order[Thread_order] >= 0)\
            longjmp(*(morn_jump[Thread_order][morn_layer_order[Thread_order]]),morn_exception);\
        else\
            exit(0);\
    }\
}while(0)

#define INVALID_POINTER(p) (((p)==NULL)||(((intptr_t)(p))==DFLT))

typedef struct MInfo
{
    char name[8][16];
    float value[8];
}MInfo;
float mInfoGet(MInfo *info,const char *name);
void mInfoSet(MInfo *info,const char *name,float value);

unsigned int mHash(const char *in,int size);

typedef struct MList
{
    int num;
    
    void **data;
    
    struct MList *handle;
    
    MInfo info;
    void *reserve;
}MList;

MList *mListCreate(int num,void **data);
void mListRelease(MList *list);
void mListAppend(MList *list,int num);
void mListPlace(MList *list,int num,int size);
#define mListClear(List) do{List->num = 0;}while(0)

void *mListWrite(MList *list,int n,void *data,int size);
void *mListRead(MList *list,int n,void *data,int size);

void mListElementDelete(MList *list,int n);
void mListElementInsert(MList *list,int n,void *data,int size);
void mListCopy(MList *src,MList *dst);
void mListMerge(MList *list1,MList *list2,MList *dst);

void mListElementOperate(MList *list,void (*func)(void *,void *),void *para);
void mListElementScreen(MList *list,int (*func)(void *,void *),void *para);
void mListElementSelect(MList *list,void (*func)(void *,void *,int *,int *,void *),void *para);
int mListCluster(MList *list,int *group,int (*func)(void *,void *,void *),void *para);
void mListSort(MList *list,int func(void *,void *,void *),void *para);
void mListReorder(MList *list);

int mQueueSize(MList *queue);
void *mQueueWrite(MList *queue,void *data,int size);
void *mQueueRead(MList *queue,void *data,int size);

#define MORN_FILE_TYPE 0
#define MORN_DIR_TYPE 16
void mFileList(MList *list,const char *directory,const char *regular);


// extern void **morn_malloc_ptr[256];
// extern int morn_malloc_num[256];

#ifdef DEBUG
void *MemoryListSet( int  size,const char *file,int line,const char *func);
void MemoryListUnset(void *ptr,const char *file,int line,const char *func);
#define mMalloc(Size)  MemoryListSet(  Size   ,__FILE__,__LINE__,__FUNCTION__)
#define mFree(Pointer) MemoryListUnset(Pointer,__FILE__,__LINE__,__FUNCTION__)
void MemoryListPrint(int state);
#else
void *mMemAlloc(int size);
void mMemFree(void *p);
#define mMalloc(Size)  mMemAlloc(Size)
#define mFree(Pointer) mMemFree(Pointer)
// void MemoryListPrint(int state);
#endif

#define mPointer(Pointer,Size) do{\
    int Size0;\
    if(Pointer == NULL)\
    {\
        Pointer = mMalloc(Size);\
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



typedef struct MHandle
{
    int flag;
    int valid;
    void *handle;
    void (*destruct)(void *);
}MHandle;

MList *mHandleCreate(void);
void mHandleRelease(MList *handle);
void mHandleReset(MList *handle);
#define ObjectHandle(Obj,Func,Hdl) do{\
    Hdl = NULL;\
    if(Obj->handle == NULL)\
        Obj->handle = mHandleCreate();\
    \
    int Num = Obj->handle->num;\
    for(int i=0;i<Num;i++)\
    {\
        MHandle *handle_data = (MHandle *)(Obj->handle->data[i]);\
        if(handle_data->flag == HASH_##Func)\
        {\
            Hdl = handle_data;\
            break;\
        }\
    }\
    \
    if(Hdl==NULL)\
    {\
        MHandle *Handle_context = (MHandle *)mMalloc(sizeof(MHandle));\
        Handle_context->flag = HASH_##Func;\
        Handle_context->valid = 0;\
        Handle_context->handle = mMalloc(sizeof(struct Handle##Func));\
        Handle_context->destruct = end##Func;\
        \
        memset(Handle_context->handle,0,sizeof(struct Handle##Func));\
        if(Num%16 == 0)\
        {\
            void **handle_buff = (void **)mMalloc((Num+16)*sizeof(void *));\
            if(Num>0)\
            {\
                memcpy(handle_buff,Obj->handle->data,Num*sizeof(void *));\
                mFree(Obj->handle->data);\
            }\
            Obj->handle->data = handle_buff;\
        }\
        Obj->handle->data[Num] = Handle_context;\
        Obj->handle->num = Num+1;\
        \
        Hdl = (MHandle *)(Obj->handle->data[Num]);\
    }\
}while(0)

#define MMemory MList
MMemory *mMemoryCreate(int num,int size);
void mMemoryRelease(MMemory *memory);
void mMemoryDataSet(MMemory *memory,char value);

int mMemorySize(MMemory *memory);
int mMemoryCheck(MMemory *memory,void *check);
void *mMemoryAppend(MMemory *memory,int num,int size);
void mMemoryCollect(MMemory *memory,void **data,int num);
void mMemoryCopy(MMemory *src,void **isrc,MMemory *dst,void **idst,int num);
void mMemoryMerge(MMemory *mem1,MMemory *mem2,MMemory *dst);

void mMemoryIndex(MMemory *memory,int num,int size,void *index[]);
void *mMemoryWrite(MMemory *memory,void *data,int size);
    
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
    MHandle *Hdl1= Obj1->handle->data[0];\
    MHandle *Hdl2= Obj2->handle->data[0];\
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
    
    MList *handle;
    
    MInfo info;
    void *reserve;
}MSheet;
MSheet *mSheetCreate(int row,int *col,void ***data);
void mSheetRelease(MSheet *sheet);
#define mSheetClear(Sheet) do{for(int Row=0;Row<Sheet->row;Row++) Sheet->col[Row]=0;Sheet->row=0;}while(0)
void mSheetRowAppend(MSheet *sheet,int row);
void mSheetColAppend(MSheet *sheet,int row,int col);
void *mSheetWrite(MSheet *sheet,int row,int col,void *data,int size);
void *mSheetRead(MSheet *sheet,int row,int col,void *data,int size);
void mSheetElementDelete(MSheet *sheet,int row,int col);
void mSheetElementInsert(MSheet *sheet,int row,int col,void *data,int size);
void mSheetReorder(MSheet *sheet);

typedef struct MTable{
    int row;
    int col;
    
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
    };
    
    MList *handle;
    
    MInfo info;
    void *reserve;
}MTable;

MTable *TableCreate(int row,int col,int element_size,void **data);
#define mTableCreate(Row,Col,Type,Data) TableCreate(Row,Col,sizeof(Type),Data)
void mTableRelease(MTable *tab);
void TableRedefine(MTable *tab,int row,int col,int element_size,void **data);
#define mTableRedefine(Tab,Row,Col,Type,Data) TableRedefine(Tab,Row,Col,sizeof(Type),Data)
#define mTableExchange(Tab1,Tab2) mObjectExchange(Tab1,Tab2,MTable)
#define mTableReset(Tab) mHandleReset(Tab->handle)

typedef struct MArray{
    int num;
    
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
    };
    
    MList *handle;
    
    MInfo info;
    void *reserve;
}MArray;
MArray *ArrayCreate(int num,int element_size,void *data);
#define mArrayCreate(Num,Type,Data) ArrayCreate(Num,sizeof(Type),Data)
void mArrayRelease(MArray *array);
void ArrayRedefine(MArray *array,int num,int element_size,void *data);
#define mArrayRedefine(Array,Num,Type,Data) ArrayRedefine(Array,Num,sizeof(Type),Data)

int mRand(int floor,int ceiling);
float mNormalRand(float mean,float delta);

/*
#define MORN_LEFT_THRESHOLD      0x01
#define MORN_RIGHT_THRESHOLD     0x02
#define MORN_DOUBLE_THRESHOLD    -1
typedef struct MThreshold{
    union
    {
        int thresh_s32;
        float thresh_f32;
        float thresh;
        unsigned char thresh_u8;
        char thresh_s8;
        unsigned short thresh_u16;
        short thresh_s16;
    };
    
    int mode;
    
    union
    {
        int leftvalue_s32;
        float leftvalue_f32;
        float leftvalue;
        unsigned char leftvalue_u8;
        char leftvalue_s8;
        unsigned short leftvalue_u16;
        short leftvalue_s16;
    };
    union
    {
        int rightvalue_s32;
        float rightvalue_f32;
        float rightvalue;
        unsigned char rightvalue_u8;
        char rightvalue_s8;
        unsigned short rightvalue_u16;
        short rightvalue_s16;
    };
}MThreshold;

#define ThresholdData(data,thresh,type) ((data<=thresh->thresh_##type)?(((thresh->mode)&0x01)?(thresh->leftvalue_##type):data):(((thresh->mode)&0x02)?(thresh->rightvalue_##type):data))
*/



int mStringRegular(const char *str1,const char *str2);
char **mStringSplit(const char *str,const char *flag,MList *list);
void mStringReplace(char *src,char *dst,const char *replace_in,const char *replace_out);




// typedef struct MTree
// {
    // void *data;
    // int size;
    
    // int child_num;
    // struct MTree **child;
    // struct MTree *parent;
    
    // MHandleSet *handle;
    // void *reserved;
// }MTree;

char *mStringArgument(int argc,char **argv,const char *flag,int *ok);
/*
#define mStringArgument1(Argc,Argv,Flag,Type,Value,defalt) do{\
    int Flag_len,Argv_len;\
    int I;\
    char *P;\
    char *Value_s;\
    if(Flag!=NULL)\
        Flag_len = strlen(Flag);\
    \
    for(I=1;I<Argc;I++)\
    {\
        if(Argv[I][0] == '-')\
        {\
            if(Flag != NULL)\
            {\
                Argv_len = strlen(Argv[I]);\
                if(strspn(Argv[I],Flag)>=Flag_len)\
                {\
                    if(Argv_len > Flag_len)\
                        P = Argv[I]+Flag_len;\
                    else\
                        P = Argv[I+1];\
                    \
                    break;\
                }\
                if(Argv_len == Flag_len)\
                    I=I+1;\
            }\
        }\
        else\
        {\
            if(Flag == NULL)\
            {\
                P = Argv[I];\
                break;\
            }\
        }\
    }\
    if(I == Argc)\
    {\
        if((strcmp(#Type,"char *")==0)||(strcmp(#Type,"char*")==0))\
        {\
            Value_s = (char *)((int)Value);\
            strcpy(Value_s,(char *)((int)defalt));\
        }\
        else if(strcmp(#Type,"int")==0)\
            *((int *)(&Value)) = (int)defalt;\
        else if(strcmp(#Type,"float")==0)\
            *((float *)(&Value)) = (float)((int)defalt);\
        else\
            mException(1,"invalid argument Type",EXIT);\
    }\
    else\
    {\
        if(strcmp(#Type,"int")==0)\
            *((int *)(&Value)) = atoi(P);\
        else if(strcmp(#Type,"float")==0)\
            *((float *)(&Value)) = atof(P);\
        else if((strcmp(#Type,"char *")==0)||(strcmp(#Type,"char*")==0))\
        {\
            Value_s = (char *)((int)Value);\
            strcpy(Value_s,P);\
        }\
        else\
            mException(1,"invalid argument Type",EXIT);\
    }\
}while(0)
*/

typedef struct MChainNode
{
    void *data;
    // int size;
    
    struct MChainNode *last;
    struct MChainNode *next;
}MChainNode;

typedef struct MTreeNode
{
    void *data;
    // int size;
    
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
        MTreeNode *treenode;
        MChainNode *chainnode;
        char *filename;
    };
    
    MList *handle;
    
    MInfo info;
    void *reserved;
}MObject;
MObject *mObjectCreate(const void *obj);
void mObjectRelease(MObject *proc);
void mObjectRedefine(MObject *object,const void *obj);

#define MChain MObject
#define MTree  MObject
#define MBtree MObject
MChain *mChainCreate();
void mChainRelease(MChain *chain);
MChainNode *mChainNode(MChain *chain,void *data,int size);
void mChainNodeInsert(MChainNode *last,MChainNode *node,MChainNode *next);
void mChainNodeDelete(MChainNode *node);
void mChainNodeExchange(MChainNode *node1,MChainNode *node2);
MChain *mChainMerge(int chain_num,MChain *chain,...);

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


void *mMapWrite(MChain *map,const void *key,int key_size,const void *value,int value_size);
void *mMapRead(MChain *map,const void *key,int key_size,void *value,int value_size);
void mMapDelete(MChain *map,const void *key,int key_size);

#define MFile  MObject
#define mFileCreate   mObjectCreate
#define mFileRelease  mObjectRelease
#define mFileRedefine mObjectRedefine

#define MProc  MObject
#define mProcCreate   mObjectCreate
#define mProcRelease  mObjectRelease
#define mProcRedefine mObjectRedefine

void mLogSet(MFile *file,int level);

#define MORN_DESKEY 0x6f676c616e726f6d
void mEncrypt(const char *in_name,const char *out_name,uint64_t key);
void mDecrypt(const char *in_name,const char *out_name,uint64_t key);
void mFileEncrypt(MFile *file,uint64_t key);
void mFileDecrypt(MFile *file,uint64_t key);

char *mINIRead(MFile *file,const char *section,const char *key);
MList *mINIKey(MFile *file,const char *section);
MList *mINISection(MFile *file);

int mMORNSize(MObject *file,const char *name);
void mMORNRead(MObject *file,const char *name,void **data,int num,int size);
void mMORNWrite(MObject *file,const char *name,void **data,int num,int size);
    
#define MORN_TREE_PREORDER_TRAVERSAL DFLT
#define MORN_TREE_POSTORDER_TRAVERSAL   1
#define MORN_TREE_INORDER_TRAVERSAL     2

#define ThreadRun2(F1,F2) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
}
#define ThreadRun3(F1,F2,F3) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
}
#define ThreadRun4(F1,F2,F3,F4) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
}
#define ThreadRun5(F1,F2,F3,F4,F5) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
}
#define ThreadRun6(F1,F2,F3,F4,F5,F6) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
}
#define ThreadRun7(F1,F2,F3,F4,F5,F6,F7) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
    void thfunc7(void)  {F7 ;} mException(pthread_create(id+6 ,NULL,(void *)thfunc7 ,NULL),EXIT,"createthread failed");\
}
#define ThreadRun8(F1,F2,F3,F4,F5,F6,F7,F8) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
    void thfunc7(void)  {F7 ;} mException(pthread_create(id+6 ,NULL,(void *)thfunc7 ,NULL),EXIT,"createthread failed");\
    void thfunc8(void)  {F8 ;} mException(pthread_create(id+7 ,NULL,(void *)thfunc8 ,NULL),EXIT,"createthread failed");\
}
#define ThreadRun9(F1,F2,F3,F4,F5,F6,F7,F8,F9) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
    void thfunc7(void)  {F7 ;} mException(pthread_create(id+6 ,NULL,(void *)thfunc7 ,NULL),EXIT,"createthread failed");\
    void thfunc8(void)  {F8 ;} mException(pthread_create(id+7 ,NULL,(void *)thfunc8 ,NULL),EXIT,"createthread failed");\
    void thfunc9(void)  {F9 ;} mException(pthread_create(id+8 ,NULL,(void *)thfunc9 ,NULL),EXIT,"createthread failed");\
}
#define ThreadRun10(F1,F2,F3,F4,F5,F6,F7,F8,F9,F10) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
    void thfunc7(void)  {F7 ;} mException(pthread_create(id+6 ,NULL,(void *)thfunc7 ,NULL),EXIT,"createthread failed");\
    void thfunc8(void)  {F8 ;} mException(pthread_create(id+7 ,NULL,(void *)thfunc8 ,NULL),EXIT,"createthread failed");\
    void thfunc9(void)  {F9 ;} mException(pthread_create(id+8 ,NULL,(void *)thfunc9 ,NULL),EXIT,"createthread failed");\
    void thfunc10(void) {F10;} mException(pthread_create(id+9 ,NULL,(void *)thfunc10,NULL),EXIT,"createthread failed");\
}
#define ThreadRun11(F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
    void thfunc7(void)  {F7 ;} mException(pthread_create(id+6 ,NULL,(void *)thfunc7 ,NULL),EXIT,"createthread failed");\
    void thfunc8(void)  {F8 ;} mException(pthread_create(id+7 ,NULL,(void *)thfunc8 ,NULL),EXIT,"createthread failed");\
    void thfunc9(void)  {F9 ;} mException(pthread_create(id+8 ,NULL,(void *)thfunc9 ,NULL),EXIT,"createthread failed");\
    void thfunc10(void) {F10;} mException(pthread_create(id+9 ,NULL,(void *)thfunc10,NULL),EXIT,"createthread failed");\
    void thfunc11(void) {F11;} mException(pthread_create(id+10,NULL,(void *)thfunc11,NULL),EXIT,"createthread failed");\
}
#define ThreadRun12(F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
    void thfunc7(void)  {F7 ;} mException(pthread_create(id+6 ,NULL,(void *)thfunc7 ,NULL),EXIT,"createthread failed");\
    void thfunc8(void)  {F8 ;} mException(pthread_create(id+7 ,NULL,(void *)thfunc8 ,NULL),EXIT,"createthread failed");\
    void thfunc9(void)  {F9 ;} mException(pthread_create(id+8 ,NULL,(void *)thfunc9 ,NULL),EXIT,"createthread failed");\
    void thfunc10(void) {F10;} mException(pthread_create(id+9 ,NULL,(void *)thfunc10,NULL),EXIT,"createthread failed");\
    void thfunc11(void) {F11;} mException(pthread_create(id+10,NULL,(void *)thfunc11,NULL),EXIT,"createthread failed");\
    void thfunc12(void) {F12;} mException(pthread_create(id+11,NULL,(void *)thfunc12,NULL),EXIT,"createthread failed");\
}
#define ThreadRun13(F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,F13) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
    void thfunc7(void)  {F7 ;} mException(pthread_create(id+6 ,NULL,(void *)thfunc7 ,NULL),EXIT,"createthread failed");\
    void thfunc8(void)  {F8 ;} mException(pthread_create(id+7 ,NULL,(void *)thfunc8 ,NULL),EXIT,"createthread failed");\
    void thfunc9(void)  {F9 ;} mException(pthread_create(id+8 ,NULL,(void *)thfunc9 ,NULL),EXIT,"createthread failed");\
    void thfunc10(void) {F10;} mException(pthread_create(id+9 ,NULL,(void *)thfunc10,NULL),EXIT,"createthread failed");\
    void thfunc11(void) {F11;} mException(pthread_create(id+10,NULL,(void *)thfunc11,NULL),EXIT,"createthread failed");\
    void thfunc12(void) {F12;} mException(pthread_create(id+11,NULL,(void *)thfunc12,NULL),EXIT,"createthread failed");\
    void thfunc13(void) {F13;} mException(pthread_create(id+12,NULL,(void *)thfunc13,NULL),EXIT,"createthread failed");\
}
#define ThreadRun14(F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,F13,F14) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
    void thfunc7(void)  {F7 ;} mException(pthread_create(id+6 ,NULL,(void *)thfunc7 ,NULL),EXIT,"createthread failed");\
    void thfunc8(void)  {F8 ;} mException(pthread_create(id+7 ,NULL,(void *)thfunc8 ,NULL),EXIT,"createthread failed");\
    void thfunc9(void)  {F9 ;} mException(pthread_create(id+8 ,NULL,(void *)thfunc9 ,NULL),EXIT,"createthread failed");\
    void thfunc10(void) {F10;} mException(pthread_create(id+9 ,NULL,(void *)thfunc10,NULL),EXIT,"createthread failed");\
    void thfunc11(void) {F11;} mException(pthread_create(id+10,NULL,(void *)thfunc11,NULL),EXIT,"createthread failed");\
    void thfunc12(void) {F12;} mException(pthread_create(id+11,NULL,(void *)thfunc12,NULL),EXIT,"createthread failed");\
    void thfunc13(void) {F13;} mException(pthread_create(id+12,NULL,(void *)thfunc13,NULL),EXIT,"createthread failed");\
    void thfunc14(void) {F14;} mException(pthread_create(id+13,NULL,(void *)thfunc14,NULL),EXIT,"createthread failed");\
}
#define ThreadRun15(F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,F13,F14,F15) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
    void thfunc7(void)  {F7 ;} mException(pthread_create(id+6 ,NULL,(void *)thfunc7 ,NULL),EXIT,"createthread failed");\
    void thfunc8(void)  {F8 ;} mException(pthread_create(id+7 ,NULL,(void *)thfunc8 ,NULL),EXIT,"createthread failed");\
    void thfunc9(void)  {F9 ;} mException(pthread_create(id+8 ,NULL,(void *)thfunc9 ,NULL),EXIT,"createthread failed");\
    void thfunc10(void) {F10;} mException(pthread_create(id+9 ,NULL,(void *)thfunc10,NULL),EXIT,"createthread failed");\
    void thfunc11(void) {F11;} mException(pthread_create(id+10,NULL,(void *)thfunc11,NULL),EXIT,"createthread failed");\
    void thfunc12(void) {F12;} mException(pthread_create(id+11,NULL,(void *)thfunc12,NULL),EXIT,"createthread failed");\
    void thfunc13(void) {F13;} mException(pthread_create(id+12,NULL,(void *)thfunc13,NULL),EXIT,"createthread failed");\
    void thfunc14(void) {F14;} mException(pthread_create(id+13,NULL,(void *)thfunc14,NULL),EXIT,"createthread failed");\
    void thfunc15(void) {F15;} mException(pthread_create(id+14,NULL,(void *)thfunc15,NULL),EXIT,"createthread failed");\
}
#define ThreadRun16(F1,F2,F3,F4,F5,F6,F7,F8,F9,F10,F11,F12,F13,F14,F15,F16) {\
    void thfunc1(void)  {F1 ;} mException(pthread_create(id+0 ,NULL,(void *)thfunc1 ,NULL),EXIT,"createthread failed");\
    void thfunc2(void)  {F2 ;} mException(pthread_create(id+1 ,NULL,(void *)thfunc2 ,NULL),EXIT,"createthread failed");\
    void thfunc3(void)  {F3 ;} mException(pthread_create(id+2 ,NULL,(void *)thfunc3 ,NULL),EXIT,"createthread failed");\
    void thfunc4(void)  {F4 ;} mException(pthread_create(id+3 ,NULL,(void *)thfunc4 ,NULL),EXIT,"createthread failed");\
    void thfunc5(void)  {F5 ;} mException(pthread_create(id+4 ,NULL,(void *)thfunc5 ,NULL),EXIT,"createthread failed");\
    void thfunc6(void)  {F6 ;} mException(pthread_create(id+5 ,NULL,(void *)thfunc6 ,NULL),EXIT,"createthread failed");\
    void thfunc7(void)  {F7 ;} mException(pthread_create(id+6 ,NULL,(void *)thfunc7 ,NULL),EXIT,"createthread failed");\
    void thfunc8(void)  {F8 ;} mException(pthread_create(id+7 ,NULL,(void *)thfunc8 ,NULL),EXIT,"createthread failed");\
    void thfunc9(void)  {F9 ;} mException(pthread_create(id+8 ,NULL,(void *)thfunc9 ,NULL),EXIT,"createthread failed");\
    void thfunc10(void) {F10;} mException(pthread_create(id+9 ,NULL,(void *)thfunc10,NULL),EXIT,"createthread failed");\
    void thfunc11(void) {F11;} mException(pthread_create(id+10,NULL,(void *)thfunc11,NULL),EXIT,"createthread failed");\
    void thfunc12(void) {F12;} mException(pthread_create(id+11,NULL,(void *)thfunc12,NULL),EXIT,"createthread failed");\
    void thfunc13(void) {F13;} mException(pthread_create(id+12,NULL,(void *)thfunc13,NULL),EXIT,"createthread failed");\
    void thfunc14(void) {F14;} mException(pthread_create(id+13,NULL,(void *)thfunc14,NULL),EXIT,"createthread failed");\
    void thfunc15(void) {F15;} mException(pthread_create(id+14,NULL,(void *)thfunc15,NULL),EXIT,"createthread failed");\
    void thfunc16(void) {F16;} mException(pthread_create(id+15,NULL,(void *)thfunc16,NULL),EXIT,"createthread failed");\
}

// int pthread_setconcurrency(int new_level);
#define mThread(Num,...) {\
    pthread_setconcurrency(16);\
    mException((Num>16)||(Num<2),EXIT,"invalid Thread number");\
    pthread_t id[16];\
    ThreadRun##Num(__VA_ARGS__);\
    for(int I=0;I<Num;I++) {pthread_join(id[I],NULL);}\
}

#ifdef __cplusplus
}
#endif

#endif
