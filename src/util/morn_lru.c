#include "morn_util.h"

struct HandleLRU
{
    int num;
    int count;
    void *(*enable)(void *,void *);
    void *enable_para;
    void (*disable)(void *,void *,void *);
    void *disable_para;
};
void endLRU(struct HandleLRU *handle) {NULL;}
#define HASH_LRU 0x41145596
void *mLRU(MChain *chain,void *key)
{
    mException(INVALID_POINTER(chain),EXIT,"invalid input");
    MHandle *hdl;struct HandleLRU *handle;
    if(chain->chainnode==NULL) 
    {
        hdl = mHandle(chain,LRU);
        handle = (struct HandleLRU *)(hdl->handle);
        if(hdl->valid == 0)
        {
            handle->num=256;
            mPropertyVariate(chain,"LRU_number"      ,&(handle->num));
            mPropertyVariate(chain,"LRU_enable"      ,&(handle->enable));
            mPropertyVariate(chain,"LRU_enable_para" ,&(handle->enable_para));
            mPropertyVariate(chain,"LRU_disable"     ,&(handle->enable));
            mPropertyVariate(chain,"LRU_disable_para",&(handle->enable_para));
            mException(handle->enable==NULL,EXIT,"no lru enable function");
            hdl->valid = 1;
        }
    }
    else
    {
        hdl = ObjHandle(chain,2);
        mException(hdl->flag!=HASH_LRU,EXIT,"invalid LRU");
        handle = (struct HandleLRU *)(hdl->handle);
    }
    void *data = handle->enable(key,handle->enable_para);

    MChainNode *node;
    if(handle->count<handle->num)
    {
        node = mChainNode(chain,NULL,DFLT);
        node->data = data;
        if(chain->chainnode!=NULL) mChainNodeInsert(NULL,node,chain->chainnode);
        handle->count++;
    }
    else
    {
        node = chain->chainnode->prev;
        if(handle->disable) handle->disable(key,node->data,handle->disable_para);
        mChainNodeDelete(chain,node);
        node->data = data;
        mChainNodeInsert(NULL,node,chain->chainnode);
    }
    chain->chainnode=node;
    return node;
}

    



/*
void mLruNumber(MChain *chain,int num)
{
    mException(INVALID_POINTER(chain),EXIT,"invalid input");
    if(num<0) num=4096;

    MList *hlist=((MList **)chain)[-1];MHandle *hdl;
    if(hlist->num<3) {hdl = mHandle(chain,LRU);hdl->valid = 1;}
    else {hdl = (MHandle *)(hlist->data[2]);mException(hdl->flag!= HASH_LRU,EXIT,"invalid LRU");}
    struct HandleLRU *handle = (struct HandleLRU *)(hdl->handle);
    if(num==handle->num) return;

    handle->num = num;
    int n= mBinaryFloor(num)/8; if(n>16384) {n=16384;}else if(n<256) {n=256;}
    
    if(handle->group) mSheetRelease(handle->group);
    handle->group = mSheetCreate(n,NULL);
    handle->group_flag = n-1;
    handle->group_byte = (int)(log((double)n)/log(2.0)+1.5);

    MChainNode *node = chain->chainnode;
    if(node == NULL) return;
    do
    {
        int *p = (int *)(node->data);
        int key_size = p[0];
        uint8_t *key = (uint8_t *)(p+2);
        int hash=0;
        for(int i=0;i<MIN(handle->group_byte,key_size);i++)
            hash=hash+hash+key[i];
        hash=hash&(handle->group_flag);

        mSheetWrite(handle->group,hash,DFLT,&node,sizeof(MChainNode *));
        node = node->next;
    }while(node!=chain->chainnode);
}

void mLruEnableFunc(MChain *chain,void *enable,void *enable_para)
{
    MList *hlist=((MList **)chain)[-1];MHandle *hdl;
    mException(INVALID_POINTER(chain),EXIT,"invalid input");
    if(hlist->num<3) {hdl = mHandle(chain,LRU);hdl->valid = 1;}
    else {hdl = (MHandle *)(hlist->data[2]);mException(hdl->flag!= HASH_LRU,EXIT,"invalid LRU");}
    struct HandleLRU *handle = (struct HandleLRU *)(hdl->handle);
    handle->enable = enable;
    handle->enable_para = enable_para;
}

void mLruDisableFunc(MChain *chain,void *disable,void *disable_para)
{
    MList *hlist=((MList **)chain)[-1];MHandle *hdl;
    mException(INVALID_POINTER(chain),EXIT,"invalid input");
    if(hlist->num<3) {hdl = mHandle(chain,LRU);hdl->valid = 1;}
    else {hdl = (MHandle *)(hlist->data[2]);mException(hdl->flag!= HASH_LRU,EXIT,"invalid LRU");}
    struct HandleLRU *handle = (struct HandleLRU *)(hdl->handle);
    handle->disable = disable;
    handle->disable_para = disable_para;
}

inline int _LRUCompare(const void *mem1,int size1,const void *mem2,int size2)
{
    if(size1!=size2) {return (size1-size2);                                }
    if(size1==4    ) {return ((*((uint32_t *)mem1))-(*((uint32_t *)mem2)));}
    if(size1==8    ) {return ((*((uint64_t *)mem1))-(*((uint64_t *)mem2)));}
    if(size1==1    ) {return ((*(( uint8_t *)mem1))-(*(( uint8_t *)mem2)));}
    if(size1==2    ) {return ((*((uint16_t *)mem1))-(*((uint16_t *)mem2)));}
                      return memcmp(mem1,mem2,size1);
}


void *mLruWrite(MChain *chain,void *key,int key_size,void *value,int value_size)
{
    mException(INVALID_POINTER(chain),EXIT,"invalid input");
    if(chain->handle->num<3) {hdl = mHandle(chain,LRU);}
    else {hdl = (MHandle *)(chain->handle->data[2]);mException(hdl->flag!= HASH_LRU,EXIT,"invalid LRU");}
    struct HandleLRU *handle = (struct HandleLRU *)(hdl->handle);
    if(hdl->valid ==0)
    {
        mException(handle->count!=0,EXIT,"invalid input");
        int n=4096;
        handle->num = n;
        handle->group = mSheetCreate(n,NULL);
        handle->group_flag = n-1;
        handle->group_byte = 13;
        hdl->valid = 1;
    }
    int hash=0;
    for(int i=0;i<MIN(handle->group_byte,key_size);i++)
        hash=hash+hash+((uint8_t *)key)[i];
    hash=hash&(handle->group_flag);

    MChainNode **group = (MChainNode **)(handle->group->data[hash]);
    MChainNode *node = NULL;
    int i;for(i=0;i<handle->group->col[hash];i++)
    {
        node = group[i];
        int *p = (int *)(node->data);
        if(_LRUCompare(key,key_size,p+2,p[0])==0) break;
    }

    if(key_size  <=0) {key_size  = strlen((char *)key  )  ;} int mkey_size  =((key_size  +7)>>3)*(8/sizeof(int));
    if(value_size<=0) {value_size= strlen((char *)value)+1;} int mvalue_size=((value_size+7)>>3)*(8/sizeof(int));

    if(i<handle->group->col[hash])
    {
        int *data = (int *)(node->data);
        if(value_size<=data[1]) {memcpy(data+2+mkey_size,value,value_size);return (data+2+mkey_size);}
    }

    MChainNode *pnode = mChainNode(map,NULL,(2+mkey_size+mvalue_size)*sizeof(int));
    data = (int *)(pnode->data);
    data[0] = key_size;data[1]=value_size;
    memcpy(data+2,key,key_size);
    if(value!=NULL) memcpy(data+2+mkey_size,value,value_size);
    if(i<handle->group->col[hash]) {node->data = pnode->data;return (data+2+mkey_size);}

    if(handle->count<handle->num)
    {
        handle->count++;
        mSheetWrite(handle->group,hash,DFLT,&pnode,sizeof(MChainNode *));
        mChainNodeInsert(NULL,pnode,chain->chainnode);chain->chainnode=pnode;
        return (data+2+mkey_size);
    }

    node = chain->chainnode->prev;
    data = (int *)(node->data);
    handle->disable(data+2,data+2+mkey_size,handle->disable_para);
    node->data = pnode->data;return (data+2+mkey_size);
    chain->chainnode=node;
    return (data+2+mkey_size);
}
*/



/*
struct LRUInfo
{
    int num;
    int count;
    MChain *chain;
    MChainNode **group;
    int group_flag;
    int group_n;
    void (*enable)(void *,void *,void *);
    void (*disable)(void *,void *,void *);
};

MObject *mLRUCreate(int num,void *enable,void *disable)
{
    struct LRUInfo *info = mMalloc(sizeof(struct LRUInfo *));
    MObject *lru = mObjectCreate(info);

    info->num = num;
    info->count=0;
    info->chain = mChainCreate();
    
    int group_num= mBinaryFloor(num)/8;
    if(group_num>16384) group_num=16384;
    else if(group_num<256) group_num=256;
    info->group = mMalloc(group_num*sizeof(MChainNode *));
    info->group_flag = group_num-1;
    info->group_n = group_num/256;

    info->enable = (void (*enable)(void *,void *,void *))enable;
    info->disable= (void (*enable)(void *,void *,void *))disable;
    return lru;
}

*/





