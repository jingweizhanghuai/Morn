/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

// typedef struct MMapData
// {
    // int key_size;
    // int value_size;
    // void *key;
    // void *value;
// }MMapData;

inline int _Compare(const void *mem1,int size1,const void *mem2,int size2)
{
    if(size1!=size2) {return (size1-size2);}
    if(size1==4    ) {return ((*((uint32_t *)mem1))-(*((uint32_t *)mem2)));}
    if(size1==2    ) {return ((*((uint16_t *)mem1))-(*((uint16_t *)mem2)));}
    if(size1==8    ) {return ((*((uint64_t *)mem1))-(*((uint64_t *)mem2)));}
    if(size1==1    ) {return ((*(( uint8_t *)mem1))-(*(( uint8_t *)mem2)));}
                      return memcmp(mem1,mem2,size1);
}

struct HandleMap
{
    int num;
    MChainNode **list;
    int list_num;
};
void endMap(struct HandleMap *handle)
{
    if(handle->list!=NULL) mFree(handle->list);
}
#define HASH_Map 0x8630f641

void _MapListAppend(struct HandleMap *handle)
{
    MChainNode *node0 = handle->list[0];

    int list_num =handle->list_num;
    // printf("list_num=%d,handle->num=%d\n",list_num,handle->num);
    if(list_num*2<handle->num) 
    {
        handle->list_num = list_num*2+1;
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
        list[(int)(k*i)]=node;
        node = node->next;
    }
    list[       0]=node0;
    list[list_num]=node0;
}

MChainNode *_MapNode(struct HandleMap *handle,const void *key,int key_size,int *flag)
{
    MChainNode **list=handle->list;
    int step = (handle->list_num+1)/4;
    int n=handle->list_num/2;
    
    *flag = 1;
    MChainNode *node = list[n];
    int *data = (int *)(node->data);
    int f = _Compare(data+2,data[0],key,key_size);
    // printf("n=%d,key=%d,data=%d,f=%d\n",n,((int *)key)[0],data[2],f);
    
    if(f==0) {*flag=n; return node;}
    while(step!=0)
    {
        if(f<0) n=n+step;
        else    n=n-step;
        
        node = list[n];
        data = (int *)(node->data);
        f = _Compare(data+2,data[0],key,key_size);
        // printf("ggn=%d,key=%d,data=%d,f=%d\n",n,((int *)key)[0],data[2],f);
        
        if(f==0) {*flag=n; return node;}
        step=step/2;
    }

    MChainNode *node0,*node1;
    if(f>0) {n=n-1;node1=node;node0=list[n  ];}
    else          {node0=node;node1=list[n+1];}
    node=node0->next;

    *flag=DFLT;
    int count=0;
    while(node!=node1)
    {
        data = (int *)(node->data);
        f = _Compare(data+2,data[0],key,key_size);
        // printf("ffn=%d,key=%d,data=%d,f=%d\n",n,((int *)key)[0],data[2],f);
        if(f==0) {*flag=-2; break;}
        if(f >0)            break;
        node=node->next;
        count++;
    }
    if(count>16) _MapListAppend(handle);
    else if(count>4)
    {
        if(n==0) list[1]=node1->last->last;
        else     list[n]=node0->next->next;
    }
    // printf("key=%d,data=%d,flag=%d\n",((int *)key)[0],data[2],*flag);
    return node;
}

void *mMapWrite(MChain *map,const void *key,int key_size,const void *value,int value_size)
{
    MChainNode *node;
    int *data;

    MHandle *hdl=mHandle(map,Map);
    struct HandleMap *handle = (struct HandleMap *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->list==NULL)
        {
            handle->list=(MChainNode **)mMalloc(128*sizeof(MChainNode *));
            
            node = mChainNode(map,NULL,2*sizeof(int)+8+8);
            data = (int *)(node->data);
            memset(data,0,2*sizeof(int)+8+8);data[0]=1;data[1]=1;
            map->chainnode = node;

            handle->list[0]=node;
            handle->list[1]=node;
            handle->list_num=1;
            handle->num=1;
        }
        hdl->valid = 1;
    }
    
    if(key_size  <=0) {key_size  = strlen((char *)key  );} int mkey_size  =((key_size  +7)>>3)*(8/sizeof(int));
    if(value_size<=0) {value_size= strlen((char *)value);} int mvalue_size=((value_size+7)>>3)*(8/sizeof(int));

    node = mChainNode(map,NULL,(2+mkey_size+mvalue_size)*sizeof(int));
    data = (int *)(node->data);
    data[0] = key_size;data[1]=value_size;
    memcpy(data+2          ,key  ,key_size  );
    memcpy(data+2+mkey_size,value,value_size);

    int flag;MChainNode *p = _MapNode(handle,key,key_size,&flag);
    if(flag!=DFLT) p->data = node->data;
    else
    {
        handle->num++;
        mChainNodeInsert(NULL,node,p);
        if(handle->num>handle->list_num*2) _MapListAppend(handle);
    }
    return (data+2+mkey_size);
}

void *mMapRead(MChain *map,const void *key,int key_size,void *value,int value_size)
{
    if(key_size<=0) key_size = strlen((char *)key);
    
    MHandle *hdl=mHandle(map,Map);
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
    int *data=(int *)(node->data);
    return (void *)(data+2);
}
void *mMapNodeValue(MChainNode *node)
{
    int *data=(int *)(node->data);
    int mkey_size =((data[0]+7)>>3)*(8/sizeof(int));
    return (void *)(data+2+mkey_size);
}
int mMapNodeKeySize(MChainNode *node)
{
    int *data=(int *)(node->data);
    return data[0];
}
int mMapNodeValueSize(MChainNode *node)
{
    int *data=(int *)(node->data);
    return data[1];
}

void mMapNodeOperate(MChain *map,void *function,void *para)
{
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

void mMapDelete(MChain *map,const void *key,int key_size)
{
    if(key_size<=0) key_size = strlen((char *)key);
    
    MHandle *hdl=mHandle(map,Map);
    struct HandleMap *handle = (struct HandleMap *)(hdl->handle);
    if(hdl->valid == 0) return;
    
    int n;MChainNode *node = _MapNode(handle,key,key_size,&n);
    if(n==DFLT) return;
    
    if(n>=0) handle->list[n]=node->next;
    handle->num--;
    if(node!=map->chainnode)
    {
        node->last->next = node->next;
        node->next->last = node->last;
    }
}
