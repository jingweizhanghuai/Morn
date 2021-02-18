/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_ptc.h"

inline int _Compare(const void *mem1,int size1,const void *mem2,int size2)
{
    if(size1!=size2) {return (size1-size2);                                }
    if(size1==4    ) {return ((*((uint32_t *)mem1))-(*((uint32_t *)mem2)));}
    if(size1==8    ) {return ((*((uint64_t *)mem1))-(*((uint64_t *)mem2)));}
    if(size1==1    ) {return ((*(( uint8_t *)mem1))-(*(( uint8_t *)mem2)));}
    if(size1==2    ) {return ((*((uint16_t *)mem1))-(*((uint16_t *)mem2)));}
                      return memcmp(mem1,mem2,size1);
}

struct HandleMap
{
    int num;
    MChainNode **list;
    int list_num;
    int list_valid;
    pthread_rwlock_t rwlock;
};
void endMap(struct HandleMap *handle)
{
    if(handle->list!=NULL) mFree(handle->list);
    pthread_rwlock_destroy(&(handle->rwlock));
}
#define HASH_Map 0x8630f641

void _MapListAppend(struct HandleMap *handle)
{
    if(handle->list_valid == 1) return;
    MChainNode *node0 = handle->list[0];
    
    int list_num =handle->list_num;
    if(list_num/2>=handle->num)
    {
        handle->list_num = MAX(list_num/4,2);
        list_num=handle->list_num;
    }
    else if(list_num*2<=handle->num) 
    {
        handle->list_num = list_num*2;
        list_num=handle->list_num;
        if(list_num>128)
        {
            mFree(handle->list);
            handle->list = (MChainNode **)mMalloc((list_num+1)*sizeof(MChainNode *));
        }
    }
    MChainNode **list = handle->list;
    
    float k=(float)(list_num)/(float)(handle->num);
    
    MChainNode *node = node0;
    for(int i=0;i<handle->num;i++)
    {
        int idx=(int)(k*i);list[idx]=node;
        node = node->next;
    }
    list[       0]=node0;
    list[list_num]=node0;
    handle->list_valid = 1;
}

MChainNode *_MapNode(struct HandleMap *handle,const void *key,int key_size,int *flag)
{
    pthread_rwlock_rdlock(&(handle->rwlock));

    *flag=DFLT;
    MChainNode **list=handle->list;
    if(handle->num==1) {pthread_rwlock_unlock(&(handle->rwlock));return list[0];}
    
    int step = (handle->list_num+1)/4;
    int n=MAX(handle->list_num/2,1);

    MChainNode *node = list[n];
    int *data = (int *)(node->data);
    int f = _Compare(data+2,data[0],key,key_size);
    if(f==0) {*flag=n; pthread_rwlock_unlock(&(handle->rwlock)); return node;}
    
    while(step!=0)
    {
        if(f<0) n=n+step;
        else    n=n-step;
        
        node = list[n];data = (int *)(node->data);
        f = _Compare(data+2,data[0],key,key_size);
        
        if(f==0) {*flag=n; pthread_rwlock_unlock(&(handle->rwlock)); return node;}
        step=step>>1;
    }

    MChainNode *node0=node,*node1=node;
    // if(f>0) {n=n-1;node1=node;node0=list[n  ];}
    // else    {      node0=node;node1=list[n+1];}
    if(f>0) {do{n=n-1;node0=list[n];}while(node0==node);node1=node;      }
    else    {do{n=n+1;node1=list[n];}while(node1==node);node0=node;n=n-1;}
    // if(f>0) {while(n>=0              ){n--;node0=list[n];if(node0!=node)       break; }}
    // else    {while(n<handle->list_num){n++;node1=list[n];if(node1!=node){n=n-1;break;}}}
    node=node0->next;
    
    int count=0;
    while(node!=node1)
    {
        data = (int *)(node->data);
        f = _Compare(data+2,data[0],key,key_size);
        if(f==0) {*flag=-2; break;}
        if(f >0)            break;
        count++;if(count>16)break;
        node=node->next;
    }
    
    if(count>16)
    {
        handle->list_valid=0;
        pthread_rwlock_unlock(&(handle->rwlock));
        
        pthread_rwlock_wrlock(&(handle->rwlock));
        _MapListAppend(handle);
        pthread_rwlock_unlock(&(handle->rwlock));
        return _MapNode(handle,key,key_size,flag);
    }
    if(count>4)
    {
        if(n==0) list[1]=node1->last->last;
        else     list[n]=node0->next->next;
    }
    pthread_rwlock_unlock(&(handle->rwlock));
    return node;
}

void *m_MapWrite(MChain *map,const void *key,int key_size,const void *value,int value_size)
{
    mException(INVALID_POINTER(map),EXIT,"invalid input map");
    mException(INVALID_POINTER(key),EXIT,"invalid input map key");
    mException(INVALID_POINTER(value)&&(value_size<=0),EXIT,"invalid input map value");
    MChainNode *node;
    int *data;
    
    MHandle *hdl;
    if(map->handle->num<3) hdl = mHandle(map,Map);
    else {hdl = (MHandle *)(map->handle->data[2]);mException(hdl->flag!= HASH_Map,EXIT,"invalid map");}
    struct HandleMap *handle = (struct HandleMap *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->list==NULL)
        {
            handle->list=(MChainNode **)mMalloc(128*sizeof(MChainNode *));
            node = mChainNode(map,NULL,2*sizeof(int)+8+8);
            data = (int *)(node->data);
            memset(data,0,2*sizeof(int)+8+8);data[0]=0;data[1]=0;
            map->chainnode = node;

            handle->list[0]=node;
            handle->list[1]=node;
            handle->list_num=1;
            handle->num=1;
            
        }
        hdl->valid = 1;
    }
    if(key_size  <=0) {key_size  = strlen((char *)key  )  ;} int mkey_size  =((key_size  +7)>>3)*(8/sizeof(int));
    if(value_size<=0) {value_size= strlen((char *)value)+1;} int mvalue_size=((value_size+7)>>3)*(8/sizeof(int));

    node = mChainNode(map,NULL,(2+mkey_size+mvalue_size)*sizeof(int));
    data = (int *)(node->data);
    data[0] = key_size;data[1]=value_size;
    memcpy(data+2,key,key_size);
    if(value!=NULL) memcpy(data+2+mkey_size,value,value_size);

    int flag;MChainNode *p = _MapNode(handle,key,key_size,&flag);
    if(flag!=DFLT) p->data = node->data;
    else
    {
        pthread_rwlock_wrlock(&(handle->rwlock));
        handle->num++;
        mChainNodeInsert(NULL,node,p);
        if(handle->num>=handle->list_num*2)
        {
            handle->list_valid = 0;
            _MapListAppend(handle);
        }
        pthread_rwlock_unlock(&(handle->rwlock));
    }
    return (data+2+mkey_size);
}

void *m_MapRead(MChain *map,const void *key,int key_size,void *value,int value_size)
{
    mException(INVALID_POINTER(map),EXIT,"invalid input map");
    if(map->handle->num<3) return NULL;
    
    mException(INVALID_POINTER(key),EXIT,"invalid input map key");
    if(key_size<=0) key_size = strlen((char *)key);

    MHandle *hdl = (MHandle *)(map->handle->data[2]);
    mException(hdl->flag!= HASH_Map,EXIT,"invalid map");
    struct HandleMap *handle = (struct HandleMap *)(hdl->handle);
    if(hdl->valid == 0) return NULL;
    
    int flag;MChainNode *node = _MapNode(handle,key,key_size,&flag);
    if(flag==DFLT) return NULL;
    
    int mkey_size =((key_size  +7)>>3)*(8/sizeof(int));
    int *data = (int *)(node->data);
    if(value!=NULL)
    {
        if(value_size <=0) value_size= data[1];
        else               value_size= MIN(value_size,data[1]);
        memcpy(value,data+2+mkey_size,value_size);
    }
    return (data+2+mkey_size);
}

void *mMapNodeKey(MChainNode *node)
{
    mException(INVALID_POINTER(node),EXIT,"invalid map node");
    int *data=(int *)(node->data);
    return (void *)(data+2);
}
void *mMapNodeValue(MChainNode *node)
{
    mException(INVALID_POINTER(node),EXIT,"invalid map node");
    int *data=(int *)(node->data);
    int mkey_size =((data[0]+7)>>3)*(8/sizeof(int));
    return (void *)(data+2+mkey_size);
}
int mMapNodeKeySize(MChainNode *node)
{
    mException(INVALID_POINTER(node),EXIT,"invalid map node");
    int *data=(int *)(node->data);
    return data[0];
}
int mMapNodeValueSize(MChainNode *node)
{
    mException(INVALID_POINTER(node),EXIT,"invalid map node");
    int *data=(int *)(node->data);
    return data[1];
}

int mMapNodeNumber(MChain *map)
{
    if(map->handle->num<3) return 0;
    MHandle *hdl = (MHandle *)(map->handle->data[2]);
    mException(hdl->flag!= HASH_Map,EXIT,"invalid map");
    struct HandleMap *handle = (struct HandleMap *)(hdl->handle);
    return (handle->num-1);
}

void mMapNodeOperate(MChain *map,void *function,void *para)
{
    mException(INVALID_POINTER(map),EXIT,"invalid input map");
    mException(INVALID_POINTER(function),EXIT,"invalid input map function");
    void (*func)(void *,int,void *,int,void *) = function;
    mException(INVALID_POINTER(map)||(func==NULL),EXIT,"invalid input");
    if(map->chainnode==NULL) return;
    MChainNode *node = map->chainnode->next;
    while(node!=map->chainnode)
    {
        int *data=(int *)(node->data);
        int mkey_size =((data[0]+7)>>3)*(8/sizeof(int));
        func((void *)(data+2),data[0],(void *)(data+2+mkey_size),data[1],para);
        node = node->next;
    }
}

void m_MapDelete(MChain *map,const void *key,int key_size)
{
    mException(INVALID_POINTER(map),EXIT,"invalid input map");
    mException(INVALID_POINTER(key),EXIT,"invalid input map key");
    if(key_size<=0) key_size = strlen((char *)key);

    MHandle *hdl = (MHandle *)(map->handle->data[2]);
    mException(hdl->flag!= HASH_Map,EXIT,"invalid map");
    struct HandleMap *handle = (struct HandleMap *)(hdl->handle);
    if(hdl->valid == 0) return;

    int n;MChainNode *node = _MapNode(handle,key,key_size,&n);
    if(n==DFLT) return;
    
    pthread_rwlock_wrlock(&(handle->rwlock));
    if(n>=0) handle->list[n]=(node->last!=map->chainnode)?node->last:node->next;
    for(int m = n+1;handle->list[m]==node;m++)handle->list[m]=handle->list[n];
    for(int m = n-1;handle->list[m]==node;m--)handle->list[m]=handle->list[n];
    
    node->last->next = node->next;
    node->next->last = node->last;
    handle->num--;
    if(handle->num*2<=handle->list_num)
    {
        handle->list_valid = 0;
        _MapListAppend(handle);
    }
    pthread_rwlock_unlock(&(handle->rwlock));
}
