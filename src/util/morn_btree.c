/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_util.h"

struct HandleBtreeCreate
{
    MMemory *memory;
};
void endBtreeCreate(void *info)
{
    struct HandleBtreeCreate *handle = (struct HandleBtreeCreate *)info;
    if(handle->memory != NULL)
        mMemoryRelease(handle->memory);
}
#define HASH_BtreeCreate 0x14c697eb
MBtree *mBtreeCreate()
{
    MBtree *btree = mObjectCreate();
    
    mHandle(btree,BtreeCreate);
    
    return btree;
}

void mBtreeRelease(MBtree *btree)
{
    mObjectRelease(btree);
}

MBtreeNode *mBtreeNode(MBtree *btree,void *data,int size)
{
    MHandle *hdl = ObjHandle(btree,1);
    mException((hdl->flag != HASH_BtreeCreate),EXIT,"invalid input btree");
    struct HandleBtreeCreate *handle = (struct HandleBtreeCreate *)(hdl->handle);
    if(handle->memory == NULL) handle->memory = mMemoryCreate(DFLT,DFLT,MORN_HOST);
    
    MBtreeNode *node = (MBtreeNode *)mMemoryWrite(handle->memory,NULL,sizeof(MBtreeNode));
    memset(node,0,sizeof(MBtreeNode));
    node->data = mMemoryWrite(handle->memory,data,size);
    
    return node;
}

void mBtreeNodeSet(MBtreeNode *node,MBtreeNode *parent,int order)
{
    if(node->parent != 0)
    {
             if(node->parent->left == node) node->parent->left = NULL;
        else if(node->parent->right== node) node->parent->right= NULL;
        node->parent = parent;
    }
    if(order==DFLT)
    {
             if(parent->left == NULL) parent->left = node;
        else if(parent->right== NULL) parent->right= node;
        else mException(1,EXIT,"invalid child order");
    }
    else if(order == MORN_LEFT)
    {
        mException(parent->left!=NULL,EXIT,"invalid child order");
        parent->left = node;
    }
    else if(order == MORN_RIGHT)
    {
        mException(parent->right!=NULL,EXIT,"invalid child order");
        parent->right = node;
    }
}

void BtreeOperate(MBtreeNode *tree,void (*func)(MBtreeNode *,void *),void *para,int mode)
{
    if(INVALID_POINTER(tree)) return;
    
    if(mode == MORN_TREE_PREORDER_TRAVERSAL)
    {
        func(tree,para);
        BtreeOperate(tree->left ,func,para,mode);
        BtreeOperate(tree->right,func,para,mode);
    }
    else if(mode == MORN_TREE_POSTORDER_TRAVERSAL)
    {
        BtreeOperate(tree->left ,func,para,mode);
        BtreeOperate(tree->right,func,para,mode);
        func(tree,para);
    }
    else if(mode == MORN_TREE_INORDER_TRAVERSAL)
    {
        BtreeOperate(tree->left ,func,para,mode);
        func(tree,para);
        BtreeOperate(tree->right,func,para,mode);
    }
    else
        mException(1,EXIT,"undefined operate");
}
void mBtreeOperate(MBtree *tree,void (*func)(MBtreeNode *,void *),void *para,int mode)
{
    BtreeOperate(tree->btreenode,func,para,mode);
}

MBtreeNode *mBTreeDecide(MBtree *btree,int (*func)(MBtreeNode *,void *),void *para)
{
    MBtreeNode *node = NULL;
    MBtreeNode *p = btree->btreenode;
    while(p!=NULL)
    {
        node = p;
        int flag = func(node,para);
        p = (flag==MORN_LEFT)?node->left:node->right;
    }
    return node;
}

