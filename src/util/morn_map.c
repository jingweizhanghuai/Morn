/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

struct HandleMap
{
    MBtreeNode *btree;
    int btree_num;
    MChainNode *begin;
    MChainNode *end;
    int     num;
    int     count;
};
#define HASH_Map 0x8630f641

void BtreeGenerate(struct HandleMap *handle)
{
    int i,j;
    MBtreeNode *btree;
    int num = handle->num;
    
    int n=1;for(i=0;;i++){int c = 2*(n+1)-1;if(c>num) break;n = c;}
    float step = (float)n/(float)num;
 
    if(n!=handle->btree_num)
    {
        if(n>127)
        {
            mFree(handle->btree);
            handle->btree = (MBtreeNode *)mMalloc(n*sizeof(MBtreeNode));
        }
        handle->btree_num = n;
        btree = handle->btree;
        
        int s = (n+1)>>2;
        int idx0 = n>>1;
        for(i=0;s>=1;i++)
        {
            for(j=idx0;j<n;j=j+s+s+s+s)
            {
                btree[j].left = &(btree[j-s]);
                btree[j].right= &(btree[j+s]);
            }
            idx0=idx0-s;
            s=s>>1;
        }
        for(j=0;j<n;j=j+2)
        {
            btree[j].left = NULL;
            btree[j].right= NULL;
        }
    }
    btree = handle->btree;
    MChainNode *cnode = handle->begin;
    for(i=0;i<num;i++)
    {
        btree[(int)(i*step)].data = cnode;
        cnode=cnode->next;
    }
}

MChainNode *MapNode(struct HandleMap *handle,const void *key,int key_size,int *flag_out)
{
    int i;
    *flag_out = 0;
    
    MBtreeNode *bnode = &(handle->btree[(handle->btree_num)>>1]);
    MChainNode *cnode = (MChainNode *)(bnode->data);
    int *data = (int *)(cnode->data);
    int flag= mCompare(data+2,data[0],key,key_size);
    while(bnode->left != NULL)
    {
        if(flag>0) bnode = bnode->left;
        else       bnode = bnode->right;
        cnode = (MChainNode *)(bnode->data);data = (int *)(cnode->data);
        flag = mCompare(data+2,data[0],key,key_size);
        if(flag == 0) return cnode;
    }
    
    MChainNode *p=cnode;
    int flag0=flag;
    for(i=1;;i++)
    {
        if(flag>0)
        {
            if(cnode==handle->begin) 
            {
                data = (int *)(cnode->data);
                flag= mCompare(data+2,data[0],key,key_size);
                break;
            }
            cnode = cnode->last;
        }
        else
        {
            if(cnode==handle->end) 
            {
                data = (int *)(cnode->data);
                flag = mCompare(data+2,data[0],key,key_size);
                break;
            }
            cnode = cnode->next;
        }
        
        data = (int *)(cnode->data);
        flag = mCompare(data+2,data[0],key,key_size);
        if(flag==0) return cnode;
        if((flag<0)!=(flag0<0)) break;
    }
    *flag_out = 0-flag;
    if(i>=8)
    {
        if(flag>0)
        {
            for(i=-1;bnode+i>=handle->btree;i--)
            {
                p=p->last;if(bnode[i].data==p)break;
                p=p->last;if(bnode[i].data==p)break;
                bnode[i].data = p;
            }
        }
        else
        {
            for(i=1;bnode+i<handle->btree+handle->btree_num;i++)
            {
                p=p->next;if(bnode[i].data==p)break;
                p=p->next;if(bnode[i].data==p)break;
                bnode[i].data = p;
            }
        }
    }
    
    return cnode;
}

void endMap(void *info)
{
    struct HandleMap *handle = (struct HandleMap *)info;
    if(handle->btree != NULL) mFree(handle->btree);
}
void *mMapWrite(MChain *map,const void *key,int key_size,const void *value,int value_size)
{
    if(key_size  <=0) {key_size  = strlen((char *)key  );} int mkey_size  =((key_size  +7)>>3)*(8/sizeof(int));
    if(value_size<=0) {value_size= strlen((char *)value);} int mvalue_size=((value_size+7)>>3)*(8/sizeof(int));
    
    MChainNode *chain_node = mChainNode(map,NULL,(2+mkey_size+mvalue_size)*sizeof(int));
    int *data = (int *)(chain_node->data);
    data[0] = key_size;data[1]=value_size;
    memcpy(data+2          ,key  ,key_size  );
    memcpy(data+2+mkey_size,value,value_size);
    
    MHandle *hdl=mHandle(map,Map);
    struct HandleMap *handle = (struct HandleMap *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->btree == NULL) handle->btree = (MBtreeNode *)mMalloc(127*sizeof(MBtreeNode));
        
        hdl->valid = 1;
        
        if(handle->num == 0)
        {
            map->chainnode = chain_node;
            chain_node->next = chain_node;
            chain_node->last = chain_node;
            handle->begin = chain_node;
            handle->end = chain_node;
            
            handle->btree_num = 1;
            MBtreeNode *btree_node = handle->btree;
            btree_node->data = chain_node;
            btree_node->left = NULL;
            btree_node->right = NULL;
            
            handle->num = 1;
            handle->count=1;
            return (data+2+mkey_size);
        }
    }
    
    int flag;MChainNode *cnode = MapNode(handle,key,key_size,&flag);
    
    if(flag==0) cnode->data=chain_node->data;
    else
    {
             if(flag< 0) {mChainNodeInsert(NULL,chain_node,cnode); if(handle->begin==cnode) handle->begin=chain_node;}
        else if(flag> 0) {mChainNodeInsert(cnode,chain_node,NULL); if(handle->end  ==cnode) handle->end  =chain_node;}
        
        handle->num  +=1;
        handle->count+=1;
    
        if(handle->count>=MAX(8,(handle->num)>>2))
        {
            map->chainnode = handle->begin;
            BtreeGenerate(handle);
            handle->count = 0;
        }
    }
    
    return (data+2+mkey_size);
}

void *mMapRead(MChain *map,const void *key,int key_size,void *value,int value_size)
{
    if(key_size<=0) key_size = strlen((char *)key);
    
    MHandle *hdl=mHandle(map,Map);
    struct HandleMap *handle = (struct HandleMap *)(hdl->handle);
    if(hdl->valid == 0) return NULL;
    
    int flag;MChainNode *cnode = MapNode(handle,key,key_size,&flag);
    
    if(flag!=0) return NULL;
    
    int mkey_size =((key_size  +7)>>3)*(8/sizeof(int));
    
    int *data = (int *)(cnode->data);
    if(value!=NULL)
    {
        if(value_size <=0) value_size= data[1];
        else               value_size= MIN(value_size,data[1]);
        memcpy(value,data+2+mkey_size,value_size);
    }
    return (data+2+mkey_size);
}
/*
void mMapDelete(MChain *map,const void *key,int key_size)
{
    int i;
    if(key_size<=0) key_size = strlen((char *)key);
    
    MHandle *hdl; ObjectHandle(map,Map,hdl);
    struct HandleMap *handle = (struct HandleMap *)(hdl->handle);
    if(hdl->valid == 0) return;
    
    MBtreeNode *bnode = &(handle->btree[(handle->btree_num)>>1]);
    MChainNode *cnode=NULL; int *data=NULL;
    
    int flag=1;int flag0;
    while(bnode != NULL)
    {
        cnode = (MChainNode *)(bnode->data);data = (int *)(cnode->data);
        flag = mCompare(data+2,data[0],key,key_size);
        if(flag == 0) {bnode->data=cnode->next;goto MapDelete_next;}
        if(flag>0) bnode = bnode->left;
        else       bnode = bnode->right;
    }
    
    flag0=flag;
    for(i=1;i<9;i++)
    {
        if(flag>0)
        {
            if(cnode==handle->begin) 
            {
                data = (int *)(cnode->data);
                flag= mCompare(data+2,data[0],key,key_size);
                if(flag==0) goto MapDelete_next;
            }
            cnode = cnode->last;
        }
        else
        {
            if(cnode==handle->end) 
            {
                data = (int *)(cnode->data);
                flag = mCompare(data+2,data[0],key,key_size);
                if(flag==0) goto MapDelete_next;
            }
            cnode = cnode->next;
        }
        
        data = (int *)(cnode->data);
        flag = mCompare(data+2,data[0],key,key_size);
        if(flag==0) goto MapDelete_next;
        if((flag<0)!=(flag0<0)) return;
    }
    if(i==9) return;
    MapDelete_next:
    if(cnode==handle->begin) handle->begin= cnode->next;
    if(cnode==handle->end  ) handle->end  = cnode->last;
    map->chainnode = handle->begin;
    mChainNodeDelete(cnode);
    handle->num -=1;
}
*/

