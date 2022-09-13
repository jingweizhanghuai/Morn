/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"

struct DicNode
{
    struct DicNode *next;
    int key_size;
    int value_size;
    char *value;
    char key[0];
};

struct HandleDictionary
{
    struct DicNode **dic;
    MMemory *memory;
};
struct HandleDictionary *morn_dictionary_handle=NULL;
void endDictionary(struct HandleDictionary *handle)
{
    if(handle->dic!=NULL) free(handle->dic);
    if(handle->memory!= NULL) mMemoryRelease(handle->memory);
    morn_dictionary_handle=NULL;
}

#define HASH_Dictionary 0x6f4ddad5

void *m_DictionaryRead(const void *key,int key_size,void *value,int *value_size)
{
    struct HandleDictionary *handle = morn_dictionary_handle;
    if(handle==NULL) return NULL;
    
    if(key_size<0) key_size=strlen(key);
    char *k=(char *)key;
    int i0=0;int i4=key_size-1;int i2=i4/2;int i1=i2/2;int i3=i2+i1;
    int hash=((int)((k[i0]<<16)+(k[i1]<<12)+(k[i2]<<8)+(k[i3]<<4)+(k[i4])))&0x0ffffff;
    
    struct DicNode *node = handle->dic[hash];
    while(node!=NULL)
    {
        if(node->key_size==key_size) 
        {
            if(memcmp(node->key,key,key_size)==0)
            {
                int vsize=node->value_size;
                if(value_size!=NULL) 
                {
                    if(*value_size>0) vsize=MIN(vsize,*value_size);
                    *value_size=node->value_size;
                }
                if(value!=NULL) memcpy(value,node->value,vsize);
                return node->value;
            }
        }
        node=node->next;
    }
    return NULL;
}

void *m_DictionaryWrite(const void *key,int key_size,const void *value,int value_size)
{
    struct HandleDictionary *handle = morn_dictionary_handle;
    if(handle==NULL)
    {
        MHandle *hdl = mHandle("Dictionary",Dictionary);
        handle = hdl->handle;

        if(handle->dic==NULL) handle->dic = malloc(65536*256*sizeof(struct DicNode));
        memset(handle->dic,0,65536*256*sizeof(struct DicNode));
        
        if(handle->memory==NULL) handle->memory = mMemoryCreate(DFLT,DFLT,MORN_HOST);

        mPropertyFunction("Dictionary","exit",mornObjectRemove,"Dictionary");
        
        hdl->valid =1;
        morn_dictionary_handle = handle;
    }
    
    if(value_size<0) value_size=strlen(value);
    if(key_size<0) key_size=strlen(key);
    char *k=(char *)key;
    int i0=0;int i4=key_size-1;int i2=i4/2;int i1=i2/2;int i3=i2+i1;
    int hash=((int)((k[i0]<<16)+(k[i1]<<12)+(k[i2]<<8)+(k[i3]<<4)+(k[i4])))&0x0ffffff;
    
    int ksize = ((key_size+7)>>3)<<3;

    struct DicNode *node = handle->dic[hash];
    while(node!=NULL)
    {
        if(node->key_size==key_size) 
        {
            if(memcmp(node->key,key,key_size)==0)
            {
                if(value==NULL) return node->value;
                
                if(node->value_size>=value_size)
                {
                    memcpy(node->value,value,value_size);
                    node->value_size=value_size;
                    return node->value;
                }
                node->value=mMemoryWrite(handle->memory,(void *)value,value_size);
                return node->value;
            }
        }
        node=node->next;
    }

    node = mMemoryWrite(handle->memory,NULL,sizeof(struct DicNode)+ksize+value_size);
    node->key_size=key_size;
    node->value_size=value_size;
    memcpy(node->key,key,key_size);
    node->value=node->key+ksize;
    memcpy(node->value,value,value_size);

    struct DicNode *node0 = handle->dic[hash];
    if(node0==NULL) {handle->dic[hash]=node;node->next=NULL;}
    else {node->next=node0->next; node0->next=node;}
    
    return node->value;
}

void m_DictionaryNodeDelete(const void *key,int key_size)
{
    struct HandleDictionary *handle = morn_dictionary_handle;
    if(handle==NULL) return;
    
    if(key_size<0) key_size=strlen(key);
    char *k=(char *)key;
    int i0=0;int i4=key_size-1;int i2=i4/2;int i1=i2/2;int i3=i2+i1;
    int hash=((int)((k[i0]<<16)+(k[i1]<<12)+(k[i2]<<8)+(k[i3]<<4)+(k[i4])))&0x0ffffff;
    
    struct DicNode *node = handle->dic[hash];
    if(node==NULL) return;
    if(node->key_size==key_size)if(memcmp(node->key,key,key_size)==0) {handle->dic[hash]=node->next;return;}
    struct DicNode *node0=node;node=node->next;
    while(node!=NULL)
    {
        if(node->key_size==key_size)
            if(memcmp(node->key,key,key_size)==0)
                {node0->next=node->next;return;}
        node0=node;node=node->next;
    }
}



