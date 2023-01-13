/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_math.h"
#include "morn_ptc.h"

#define fread(Data,Size,Num,Fl) mException(((int)fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error")

#ifdef __GNUC__
#define stricmp strcasecmp
#endif

int morn_dflt=DFLT;

__thread void *morn_test=NULL;

__thread char morn_filename[256];

__thread int morn_data_type;
__thread char morn_data_buff[8];
char morn_char_for_test=-1;

static int morn_rand_seed = -1;
int m_Rand(int floor,int ceiling)
{
    #ifndef _DEBUG
    if(morn_rand_seed == -1)
    {
        morn_rand_seed = time(NULL);
        srand(morn_rand_seed);
    }
    #endif

    if((floor==DFLT)&&(ceiling==DFLT)) {return rand();}
    if(floor==ceiling) return floor;
    int d = ceiling-floor;
    if(d>RAND_MAX) return (((rand()<<15)+rand())%d)+floor;
    return (rand()%d)+floor;
}
 
float mNormalRand(float mean,float delta)
{
    float u = mRand(1,32768)/32768.0f;
    float v = mRand(0,32767)/32767.0f;
    
    float out = mSqrt(0.0-2.0*mLn(u))*mCos(360.0*v);
    return (out*delta+mean);
}

char *morn_string="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
int m_RandString(char *str,int l1,int l2)
{
    int size;if(l1<=0) {size=l2-1;}else if(l2<=0) {size=l1;} else if(l1==l2) {size=l1;} else {size=mRand(l1,l2);}
    mException(size<=0,EXIT,"invalid input");
    for(int i=0;i<size;i++) str[i]=morn_string[mRand(0,62)];
    str[size]=0;
    return size;
}
 
int mCompare(const void *mem1,int size1,const void *mem2,int size2)
{
    if((size1<0)&&(size2<0)) return strcmp((char *)mem1,(char *)mem2);
    if(size1<0) size1 = strlen((char *)mem1);
    if(size2<0) size2 = strlen((char *)mem2);
    int flag = memcmp(mem1,mem2,MIN(size1,size2));
    return (flag!=0)?flag:(size1-size2);
}

// float mInfoGet(MInfo *info,const char *name)
// {
//     for(int i=0;i<8;i++)
//         if(stricmp(&(info->name[i][0]),name)==0)
//             return info->value[i];
      
//     return mNan();
// }

// void mInfoSet(MInfo *info,const char *name,float value)
// {
//     for(int i=0;i<8;i++)
//         if(stricmp(&(info->name[i][0]),name)==0)
//         {
//             info->value[i] = value;
//             return;
//         }
        
//     for(int i=0;i<8;i++)
//         if(info->name[i][0] == 0)
//         {
//             strcpy(&(info->name[i][0]),name);
//             info->value[i] = value;
//             return;
//         }
//     mException(1,EXIT,"no enough space for %s\n",name);
// }



struct HandleObjectCreate
{
    MObject *object;
    MChain *property;
    int64_t reserve[8];
    int writeable;
    
    uint64_t buff1;
    void *buff2;
    int buff_size;
};
#define HASH_ObjectCreate 0xbd1d8878
void endObjectCreate(struct HandleObjectCreate *handle)
{
    mException((handle->object == NULL),EXIT,"invalid object");
    if(handle->buff2!=NULL) mFree(handle->buff2);
    if(handle->property!=NULL) mChainRelease(handle->property);
    memset(handle->object,0,sizeof(MObject));
    // ObjectFree(handle->object);
}
MObject *m_ObjectCreate(void *p,int size)
{
    MObject *object = (MObject *)ObjectAlloc(sizeof(MObject));
    mObjectType(object)=HASH_ObjectCreate;
    MHandle *hdl=mHandle(object,ObjectCreate);
    struct HandleObjectCreate *handle = (struct HandleObjectCreate *)(hdl->handle);
    handle->object = object;

    object->size = size;
    object->object = p;
    if((p==NULL)&&(size>0)) 
    {
        if(size<=sizeof(uint64_t)) p=&(handle->buff1);
        else {handle->buff2=mMalloc(size);p=handle->buff2;handle->buff_size=size;}
        object->object=p;
    }
    else if((p!=NULL)&&(size<=0))
    {
        size = strlen(p);
        object->size = size;
    }
    return object;
}

void m_ObjectRedefine(MObject *object,void *p,int size)
{
    if(size<=0) {if(p!=NULL) {size=strlen(p);} else {size=object->size;}}
    if((p==object->object)&&(size==object->size)) return;
    
    if(p==NULL)
    {
        struct HandleObjectCreate *handle= (struct HandleObjectCreate *)(ObjHandle(object,0)->handle);
        if(handle->buff_size<size) {if(handle->buff2!=NULL) {mFree(handle->buff2);} handle->buff2=mMalloc(size);handle->buff_size = size;}
        object->object = handle->buff2;
        p=object->object;
    }
    
    object->size =size;
    object->object = p;
    
    mHandleReset(object);
}


/*
struct HandleObjectCreate
{
    MObject *object;
    MChain *property;
    int64_t reserve[8];
    int writeable;
    // int size;
};
void endObjectCreate(struct HandleObjectCreate *handle)
{
    mException((handle->object == NULL),EXIT,"invalid object");
    if(handle->property!=NULL) mChainRelease(handle->property);
    mFree(((MList **)(handle->object))-1);
}
#define HASH_ObjectCreate 0xbd1d8878
MObject *m_ObjectCreate(const void *obj)
{
    MList **phandle = (MList **)mMalloc(sizeof(MList *)+sizeof(MObject));
    MObject *object = (MObject *)(phandle+1);

    *phandle = mHandleCreate();
    MHandle *hdl=mHandle(object,ObjectCreate);
    struct HandleObjectCreate *handle = (struct HandleObjectCreate *)(hdl->handle);
    handle->object = object;
    object->object = (void *)obj;
    return object;
}

void mObjectRedefine(MObject *object,const void *obj)
{
    if(object->object != obj)
    {
        object->object = (void *)obj;
        mHandleReset(object);
    }
}

*/
 
void mObjectRelease(MObject *object)
{
    ObjectFree(object);
}

void *mObjectMemory(MObject *object)
{
    struct HandleObjectCreate *handle= (struct HandleObjectCreate *)(ObjHandle(object,0)->handle);
    if(object->size>8) return handle->buff2;
    return &(handle->buff1);
}

struct _HandleObjectCreate
{
    MObject *object;
    MChain *property;
    int64_t reserve[8];
    int writeable;
};
void HandleExchange(void *obj1,void *obj2)
{
    struct HandleList *hl1 = ((struct HandleList *)obj1)-1;
    struct HandleList *hl2 = ((struct HandleList *)obj2)-1;
    
    MHandle *hdl1= (MHandle *)(hl1->list.data[0]);
    MHandle *hdl2= (MHandle *)(hl2->list.data[0]);
    hl1->list.data[0]=hdl2;hl2->list.data[0]=hdl1;

    struct _HandleObjectCreate *handle1=hdl1->handle;
    struct _HandleObjectCreate *handle2=hdl2->handle;
    struct _HandleObjectCreate handle_buff=*handle1;*handle1=*handle2;*handle2=handle_buff;
}

struct Property
{
    void *var;
    void (*func)(void *,void *);
    void *para;
    char value[0];
};

void m_PropertyVariate(MObject *obj,const char *key,void *var,int var_size)
{
    if(var_size<0) var_size = strlen((char *)var)+1;
    
    struct HandleObjectCreate *handle = (struct HandleObjectCreate *)(ObjHandle(obj,0)->handle);
    if(handle->property==NULL) handle->property = mChainCreate();
    
    int vsize=sizeof(struct Property)+var_size;
    struct Property *p = mornMapRead(handle->property,key,DFLT,NULL,&vsize);
    
    if(p!=NULL) memcpy(var,p->value,vsize-sizeof(struct Property));
    else {p=mornMapWrite(handle->property,key,DFLT,NULL,vsize);p->func=NULL;p->para=NULL;}
    p->var = var;
}

void m_PropertyFunction(MObject *obj,const char *key,void *function,void *para)
{
    // func(new,para);
    void (*func)(void *,void *) = function;
    struct HandleObjectCreate *handle = (struct HandleObjectCreate *)(ObjHandle(obj,0)->handle);
    if(handle->property==NULL) handle->property = mChainCreate();

    int vsize=sizeof(struct Property);
    struct Property *p = mornMapRead(handle->property,key,DFLT,NULL,&vsize);
    if(p!=NULL)
    {
        int value_size=vsize-sizeof(struct Property);
        if(value_size>0)
        {
            if(p->var!=NULL)
            {
                if(memcmp(p->var,p->value,value_size)!=0)
                    {func(p->value,para);memcpy(p->value,p->var,value_size);}
            }
            else func(p->value,para);
        }
    }
    else {p = mornMapWrite(handle->property,key,DFLT,NULL,vsize);p->var=NULL;}
    p->func=func;p->para=para;
}

void *m_PropertyWrite(MObject *obj,const char *key,const void *value,int value_size)
{
    int deft_value=1;
    if(value==NULL) {value=&deft_value;value_size=sizeof(int);}
    else if(value_size<=0) value_size=strlen(value)+1;
    
    struct HandleObjectCreate *handle = (struct HandleObjectCreate *)(ObjHandle(obj,0)->handle);
    if(handle->property==NULL) handle->property = mChainCreate();

    int vsize;
    struct Property *p = mornMapRead(handle->property,key,DFLT,NULL,&vsize);
    if(p!=NULL)
    {
        if(value==&deft_value) {if(p->func!=NULL) {p->func(p->value,p->para);} return p->value;}
        if(vsize==value_size+sizeof(struct Property))
        {
            memcpy(p->value,value,value_size);
            if(p->var !=NULL) {if(memcmp(p->var,value,value_size)==0) return p->var;}
            if(p->func!=NULL) p->func(p->value,p->para);
            if(p->var !=NULL) {memcpy(p->var,value,value_size); return p->var;}
            return p->value;
        }
    }
    struct Property *q = mornMapWrite(handle->property,key,DFLT,NULL,sizeof(struct Property)+value_size);
    
    if(value!=NULL) memcpy(q->value,value,value_size);
    if(p!=NULL) {q->var=p->var;q->func=p->func;q->para=p->para;}
    else        {q->var=  NULL;q->func=   NULL;q->para=   NULL;}

    if((q->var!=NULL)&&(value!=NULL)) memcpy(q->var,value,value_size);
    if(q->func!=NULL) q->func(q->value,q->para);
    return q->value;
}

void *m_PropertyRead(MObject *obj,const char *key,void *value,int *value_size)
{
    struct HandleObjectCreate *handle = (struct HandleObjectCreate *)(ObjHandle(obj,0)->handle);
    
    if(handle->property==NULL) return NULL;
    int vsize=0;
    struct Property *q= mornMapRead(handle->property,key,DFLT,NULL,&vsize);
    if(q==NULL) return NULL;
    
    void *p=(q->var!=NULL)?q->var:q->value;
    vsize = vsize-sizeof(struct Property);
    int size=vsize;
    if(value_size!=NULL) {{if(*value_size>0) size=MIN(*value_size,size);}*value_size = vsize;}
    if(value!=NULL) memcpy(value,p,size);
    return p;
}

void *mReserve(MObject *obj,int n)
{
    struct HandleObjectCreate *handle = (struct HandleObjectCreate *)(ObjHandle(obj,0)->handle);
    return (handle->reserve+n);
}
 
MFile *mFileCreate(const char *filename,...)
{
    MFile *file = mObjectCreate(NULL,256);
    
    va_list namepara;
    va_start (namepara,filename);
    vsnprintf(file->filename,256,filename,namepara);
    va_end(namepara);
    
    return file;
}

void mFileRedefine(MFile *file,char *filename,...)
{
    mObjectRedefine(file,NULL,256);
    va_list namepara;
    va_start (namepara,filename);
    vsnprintf(file->filename,256,filename,namepara);
    va_end(namepara);
}

// struct HandleFileCreate
// {
//     char filename[272];
// };
// void endFileCreate(void *info) {}
// #define HASH_FileCreate 0xfdab2bff
// MFile *m_FileCreate0()
// {
//     MFile *file = mObjectCreate();
//     MHandle *hdl = mHandle(file,FileCreate);
//     hdl->valid=1;
//     struct HandleFileCreate *handle = hdl->handle;
//     file->filename = handle->filename+4;
//     handle->filename[0]=1;
//     return file;
// }

// MFile *m_FileCreate(const char *filename,...)
// {
//     MFile *file = m_FileCreate0();
//     if(INVALID_POINTER(filename)) return file;
    
//     va_list namepara;
//     va_start (namepara,filename);
//     vsnprintf(file->filename,256,filename,namepara);
//     va_end(namepara);
    
//     return file;
// }

// void mFileRedefine(MFile *file,char *file_name,...)
// {
//     if(strcmp(file_name,file->filename)==0) return;
//     char filename[256];
//     va_list namepara;
//     va_start (namepara,file_name);
//     vsnprintf(filename,256,file_name,namepara);
//     va_end(namepara);
//     if(strcmp(filename,file->filename)==0) return;
//     strcpy(file->filename,filename);
//     file->filename[-4]=1;
//     mHandleReset(file);
// }

int fsize(FILE *f)
{
    mException(f==NULL,EXIT,"invalid file");
    int l=ftell(f);     fseek(f,0,SEEK_END);
    int size=ftell(f)-l;fseek(f,l,SEEK_SET);
    return size;
}

void mFile(MObject *object,const char *file_name,...)
{
    char filename[256];
    va_list namepara;
    va_start (namepara,file_name);
    vsnprintf(filename,256,file_name,namepara);
    va_end(namepara);
    FILE *f = fopen(filename,"rb");
    int size = fsize(f);
    m_ObjectRedefine(object,NULL,size+1);
    fread(object->string,size,1,f);
    object->string[size]=0;
    fclose(f);
}

// MList *mHandleCreate(void)
// {
//     MList *handle = (MList *)mMalloc(sizeof(MList));
//     handle->num = 0;
//     handle->data = NULL;
    
//     return handle;
// }

// void mHandleRelease(void *obj)
// {
//     if(INVALID_POINTER(obj)) return;
//     MList *hlist = ((MList **)obj)[-1];
//     for(int i=hlist->num-1;i>=0;i--)
//     {
//         MHandle *hdl = (MHandle *)(hlist->data[i]);
//         (hdl->destruct)(hdl->handle);
//         mFree(hdl);
//     }
    
//     mFree(hlist->data);
//     mFree(hlist);
// }

// void mHandleReset(void *obj) 
// {
//     if(INVALID_POINTER(obj)) return;
//     MList *hlist = ((MList **)obj)[-1];
//     for(int i=1;i<hlist->num;i++)
//     {
//         MHandle *hdl = (MHandle *)(hlist->data[i]);
//         hdl->valid = 0;
//     }
// }

void *ObjectAlloc(int size)
{
    struct HandleList *hl = mMalloc(sizeof(struct HandleList)+size);
    hl->list.num = 0;
    hl->list.data = NULL;
    hl->latest_flag = DFLT;
    hl->latest_n = -1;
    hl->valid = 0;
    memset(hl+1,0,size);
    return (void *)(hl+1);
}

void ObjectFree(void *obj)
{
    if(INVALID_POINTER(obj)) return;
    struct HandleList *hl = ((struct HandleList *)obj)-1;
    for(int i=hl->list.num-1;i>=0;i--)
    {
        MHandle *hdl = (MHandle *)(hl->list.data[i]);
        (hdl->destruct)(hdl->handle);
        mFree(hdl);
    }
    mFree(hl->list.data);
    mFree(hl);
}

void mHandleReset(void *obj) 
{
    if(INVALID_POINTER(obj)) return;
    struct HandleList *hl = ((struct HandleList *)obj)-1;
    for(int i=1;i<hl->list.num;i++)
    {
        MHandle *hdl = (MHandle *)(hl->list.data[i]);
        hdl->valid = 0;
    }
}

MHandle *GetHandle(void *obj,int size,unsigned int hash,void (*end)(void *))
{
    mException((obj==NULL)||(size<=0)||(end==NULL),EXIT,"invalid input");
    struct HandleList *hl = ((struct HandleList *)obj)-1;
    if(hl->latest_flag==hash) return (MHandle *)(hl->list.data[hl->latest_n]);

    while(1)
    {
        int v=mAtomicCompare(&hl->valid,0,1);
        if(v==1) break;
        while(1) {if(hl->valid==0) break;}
    }
    
    MList *hlist = (MList *)hl;
    int num = hlist->num;
    for(int i=0;i<num;i++)
    {
        MHandle *handle_data = (MHandle *)(hlist->data[i]);
        if(handle_data->flag == hash)
        {
            hl->latest_n=i;
            hl->latest_flag=hash;
            hl->valid=0;
            return handle_data;
        }
    }
    
    MHandle *Handle_context = (MHandle *)mMalloc(sizeof(MHandle)+size);
    Handle_context->flag    = hash;
    Handle_context->valid   = 0;
    Handle_context->handle  =Handle_context+1;memset(Handle_context->handle,0,size);
    Handle_context->destruct= end;
    if(num%16 == 0)
    {
        void **handle_buff = (void **)mMalloc((num+16)*sizeof(void *));
        if(num>0)
        {
            memcpy(handle_buff,hlist->data,num*sizeof(void *));
            mFree(hlist->data);
        }
        hlist->data = handle_buff;
    }
    hlist->data[num] = Handle_context;
    hlist->num = num+1;
    hl->latest_n=num;
    hl->latest_flag=hash;
    hl->valid=0;
    return (MHandle *)(hlist->data[num]);
}

int mHandleValid(MHandle *hdl)
{
    if(hdl->valid==1) return 1;
    int v=mAtomicCompare(&(hdl->valid),0,-1);
    if(v==1) return 0;
    while(1) {if(hdl->valid==1) return 1;}
}

void endMAF();
int mornObjectMapNodeRelease(char *key,int key_size,void *value,int value_size,void *para)
{
    mObjectRelease(*(MObject **)value);
    return 0;
}

__thread MObject *morn_object=NULL;
MChain *morn_object_map=NULL;
#ifdef _MSC_VER
void mMornEnd(void)
{
    // printf("endendendendendendendendendendend\n");
    if(morn_object!=NULL) mObjectRelease(morn_object);
    morn_object=NULL;
    
    if(morn_object_map!=NULL)
    {
//         if(morn_object_map->chainnode!=NULL)
//         {
//             MChainNode *node = morn_object_map->chainnode->next;
//             while(node!=morn_object_map->chainnode)
//             {
//                 mObjectRelease(*(MObject **)mornMapNodeValue(node));
//                 node=node->next;
//             }
//         }
        mornMapNodeOperate(morn_object_map,mornObjectMapNodeRelease,NULL);
        mChainRelease(morn_object_map);
    }
    morn_object_map = NULL;
    endMAF();
}     
void mMornBegin()
{
    morn_object=mObjectCreate();
    morn_object_map=mChainCreate();
    atexit(mMornEnd);
}
#pragma section(".CRT$XCU",read)
__declspec(allocate(".CRT$XCU")) void (* mornbegin)() = mMornBegin;

#else
__attribute__((constructor)) void mMornBegin()
{
    if(morn_object==NULL) morn_object=mObjectCreate();
    if(morn_object_map==NULL) morn_object_map=mChainCreate();
    // printf("before mai222n\n");
}

__attribute__((destructor)) void mMornEnd()
{
    if(morn_object!=NULL) mObjectRelease(morn_object);
    morn_object=NULL;
    // printf("aaaaaaaaaaa1111\n");
    
    if(morn_object_map!=NULL)
    {
//         if(morn_object_map->chainnode!=NULL)
//         {
//             MChainNode *node = morn_object_map->chainnode->next;
//             while(node!=morn_object_map->chainnode)
//             {
//                 mObjectRelease(*(MObject **)mornMapNodeValue(node));
//                 node=node->next;
//             }
//         }
        mornMapNodeOperate(morn_object_map,mornObjectMapNodeRelease,NULL);
        mChainRelease(morn_object_map);
    }
    morn_object_map = NULL;
    endMAF();
}
#endif
 
MObject *mMornObject(const void *p,int size)
{
    if(p==NULL) 
    {
        if(morn_object==NULL) morn_object=mObjectCreate();
        return morn_object;
    }

    MObject **pobj=NULL;
    if(morn_object_map==NULL) morn_object_map=mChainCreate();
    else pobj = (MObject **)mornMapRead(morn_object_map,p,size,NULL,NULL);
    
    if(pobj==NULL)
    {
        MObject *obj=mObjectCreate(p,size);
        pobj=(MObject **)mornMapWrite(morn_object_map,p,size,NULL,(sizeof(MObject *)));
        *pobj=obj;
    }
    return (*pobj);
}

void mornObjectRemove(void *no_use,char *name)
{
    MObject **pobj = (MObject **)mornMapRead(morn_object_map,name,DFLT,NULL,NULL);
    mObjectRelease(*pobj);
    mornMapNodeDelete(morn_object_map,name,DFLT);
}

__thread char morn_shu[256];
void _CNum(double data,char *out,int *flag)
{
    #if defined(__linux__)
    int s=3;
    int32_t dic[19]={0x00b69be9,0x0080b8e4,0x00a4b8e4,0x0089b8e4,0x009b9be5,0x0094bae4,0x00ad85e5,0x0083b8e4,0x00ab85e5,0x009db9e4,0x00818de5,0x00be99e7,0x00838de5,0x0087b8e4,0x00bfbae4,0x009fb4e8,0x00b982e7,0x00a4b8e4,0x008cbae4};
    #elif defined(_WIN64)||defined(_WIN32)
    int32_t s=2;
    int32_t dic[19]={0x0000e3c1,0x0000bbd2,0x0000feb6,0x0000fdc8,0x0000c4cb,0x0000e5ce,0x0000f9c1,0x0000dfc6,0x0000cbb0,0x0000c5be,0x0000aeca,0x0000d9b0,0x0000a7c7,0x0000f2cd,0x0000dad2,0x0000bab8,0x0000e3b5,0x0000feb6,0x0000bdc1};
    #else
    mException(1,EXIT,"invalid operate system");
    #endif

    if(data<0) {memcpy(out,dic+15,s);out+=s;data=0.0-data;}
    int64_t value = (int64_t)data;mException(ABS(value-data)>1,EXIT,"input data too large");
    double value2 = data-(double)value;
    if(value<=10) {memcpy(out,dic+value,s);out+=s;goto cnum_next;}
    if((value<20)&&(*flag==0)) {memcpy(out,dic+10,s);memcpy(out+s,dic+(value-10),s);out+=s+s;goto cnum_next;}

    if(value>=100000000)
    {
        int64_t v=value/100000000;
        if(v==2)  {memcpy(out,dic+18,s);out+=s;}
        else {_CNum((double)v,out,flag);out+=strlen(out);}
        memcpy(out,dic+14,s);
        out=out+s;
        value = value%100000000;
        *flag=(value!=0);
    }

    if(value>=10000)
    {
        int64_t v=value/10000;
        if(v==2)  {memcpy(out,dic+18,s);out+=s;}
        else {_CNum((double)v,out,flag);out+=strlen(out);}
        memcpy(out,dic+13,s);
        out=out+s;
        value = value%10000;
        *flag=(value!=0);
    }
    
    dic[2]=dic[18];
    
    if(value>=1000) {memcpy(out,dic+(value/1000),s);memcpy(out+s,dic+12,s);out=out+s+s;value=value%1000;*flag=(value!=0);}
    else if(*flag)  {memcpy(out,dic,s);out=out+s;*flag=0;}

    if(value>=100) {memcpy(out,dic+(value/100),s);memcpy(out+s,dic+11,s);out=out+s+s;value=value%100;*flag=(value!=0);}
    else if(*flag)  {memcpy(out,dic,s);out=out+s;*flag=0;}

    dic[2]=dic[17];

    if(value>=10) {memcpy(out,dic+(value/10),s);memcpy(out+s,dic+10,s);out=out+s+s;*flag=1;value=value%10;}
    else if(*flag)  {memcpy(out,dic,s);out=out+s;*flag=0;}

    if(value!=0) {memcpy(out,dic+value,s);out=out+s;}

    cnum_next:
    if(value2>0.0000001)
    {
        memcpy(out,dic+16,s);out=out+s;*flag=1;
        for(int i=0;i<9;i++)
        {
            if(value2<0.0000001) {*flag=0;break;}
            value2 = value2*10;
            value = (int)value2;
            value2=value2-(double)value;if(value2>0.999999) {value+=1;value2=0;}
            memcpy(out,dic+value,s);out=out+s;
        }
        if(*flag)
        {
            value2 = value2*10;
            value = (int)(value2+0.5);
            memcpy(out,dic+value,s);out=out+s;
        }
    }
    *out=0;
}

const char *mShu(double data)
{
    int flag=0;
    _CNum(data,morn_shu,&flag);
    return morn_shu;
}

__thread int morn_layer_order = -1;
__thread int morn_exception = 0;
__thread jmp_buf *morn_jump[32];
void m_Exception(int err,int ID,const char *file,int line,const char *function,const char *message,...)
{
    if(err==0) return;
    char msg[256];
    va_list msgpara;
    va_start (msgpara,message);
    vsnprintf(msg,256,message,msgpara);
    va_end(msgpara);
    mLog(MORN_ERROR,"[%s,line %d,function %s]Error: %s\n",file,line,function,msg);
    if(err >0)
    {
        morn_exception = ID;
        if(morn_layer_order<0)
        {
            #ifdef _MSC_VER
            system("pause");
            #endif
            exit(0);
        }
        longjmp(*(morn_jump[morn_layer_order]),morn_exception);
    }
}

