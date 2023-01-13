/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_util.h"

typedef struct TreeNode
{
    void *data;
    struct MTreeNode **child;
    struct MTreeNode *parent;
    int child_num;

    int16_t node_cap;
    int16_t ID;
    MGraph *graph;
}TreeNode;

struct HandleTreeCreate
{
    struct TreeNode *node;
    int node_num;
    int node_cap;
    MMemory *memory;
};
void endTreeCreate(struct HandleTreeCreate *handle)
{
    for(int i=0;i<handle->node_num;i++)
        mFree(handle->node[i].child);
    if(handle->memory != NULL) mMemoryRelease(handle->memory);
    if(handle->node   != NULL) mFree(handle->node);
}
#define HASH_TreeCreate 0x14c697eb
MTree *mTreeCreate()
{
    MTree *tree = mObjectCreate();
    mObjectType(tree)=HASH_TreeCreate;
    mHandle(tree,TreeCreate);
    
    return tree;
}

void TreeNodeRelease(MTreeNode *node)
{
    for(int i=0;i<node->child_num;i++)
        TreeNodeRelease(node->child[i]);
    
    if(node->child!=NULL) mFree(node->child);
}
void mTreeRelease(MTree *tree)
{
    if(tree->object != NULL)
        TreeNodeRelease((MTreeNode *)(tree->object));
    
    mObjectRelease(tree);
}

void _TreeMemoryCollect(MTree *tree)
{
    NULL;
}

#define HASH_TreeNode 0xa8930197
MTreeNode *mTreeNode(MTree *tree,void *data,int size)
{
    MHandle *hdl = ObjHandle(tree,0);
    mException((hdl->flag != HASH_TreeCreate),EXIT,"invalid input tree");
    struct HandleTreeCreate *handle =(struct HandleTreeCreate *)(hdl->handle);
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(DFLT,DFLT,MORN_HOST);
    
    MTreeNode *node = (MTreeNode *)mMemoryWrite(handle->memory,NULL,sizeof(MTreeNode)+2*sizeof(int)+size);
    memset(node,0,sizeof(MTreeNode));
    int *info = (int *)(node+1);info[0]=0;info[1]=HASH_TreeNode;
    node->data = (info+2);
    if(data!=NULL) memcpy(node->data,data,size);
    else           memset(node->data,   0,size);

    handle->node_num++;
    // if(handle->node_num%1024==0)
    // {
    //     _TreeMemoryCollect(tree);
    // }
    
    return node;
}

void mTreeNodeInsert(MTreeNode *tree,MTreeNode *child,int order)
{
    child->parent = tree;
    
    int num0 = tree->child_num;
    if(order<0) order=num0;
    if(order>=num0)
    {
        tree->child_num = order+1;
    
        int *info = (int *)(tree+1);
        mException(info[1]!=HASH_TreeNode,1,"invalid input child node");
        if(tree->child_num>info[0])
        {
                 if(info[0]==  0) info[0] =2;
            else if(info[0]<   4) info[0] =4;
            else if(info[0]<  16) info[0]+=4;
            else                  info[0]+=info[0]/3;
            info[0]=MAX(info[0],tree->child_num);
            MTreeNode **buff = mMalloc(info[0]*sizeof(MTreeNode *));
            if(num0>0)
            {
                memcpy(buff,tree->child,num0 *sizeof(MTreeNode *));
                mFree(tree->child);
            }
            memset(buff+num0,0,(info[0]-num0)*sizeof(MTreeNode *));
            tree->child = buff;
        }
    }
    
    tree->child[order] = child;
}

void TreeTraversal(MTreeNode *tree,void (*func)(MTreeNode *,void *),void *para,int mode)
{
    mException(INVALID_POINTER(tree),EXIT,"invalid tree");
    
    if(mode == MORN_TREE_PREORDER_TRAVERSAL)
    {
        func(tree,para);
        for(int i=0;i<tree->child_num;i++)
            TreeTraversal(tree->child[i],func,para,mode);
    }
    else if(mode == MORN_TREE_POSTORDER_TRAVERSAL)
    {
        for(int i=0;i<tree->child_num;i++)
            TreeTraversal(tree->child[i],func,para,mode);
        func(tree,para);
    }
    else if(mode == MORN_TREE_INORDER_TRAVERSAL)
    {
        mException((tree->child_num!=2),EXIT,"undefined operate");
        TreeTraversal(tree->child[0],func,para,mode);
        func(tree,para);
        TreeTraversal(tree->child[1],func,para,mode);
    }
    else
        mException(1,EXIT,"undefined operate");
}
void mTreeTraversal(MTree *tree,void *function,void *para,int mode)
{
    void (*func)(MTreeNode *,void *) =function;
    TreeTraversal(tree->treenode,func,para,mode);
}

MTreeNode *TreeDecide(MTreeNode *tree,int (*func)(MTreeNode *,void *),void *para)
{
    if(tree->child_num>0)
    {
        int order = func(tree,para);
        return TreeDecide(tree->child[order],func,para);
    }
    else return tree;
}
MTreeNode *mTreeDecide(MTree *tree,void *function,void *para)
{
    int (*func)(MTreeNode *,void *) = function;
    return TreeDecide(tree->treenode,func,para);
}

MTreeNode *TreeSearch(MTreeNode *node,int (*func)(MTreeNode *,void *),void *para)
{
    if(func(node,para)==1) return node;
    
    for(int i=0;i<node->child_num;i++)
    {
        MTreeNode *rst = TreeSearch(node->child[i],func,para);
        if(rst != NULL) return rst;
    }
    return NULL;
}

#define MORN_TREE_SEARCH_ALL DFLT
#define MORN_TREE_SEARCH_NEXT 0
MTreeNode *mTreeSearch(MTreeNode *node,void *function,void *para,int mode)
{
    int (*func)(MTreeNode *,void *) = function;
    MTreeNode *rst;
    for(int i=0;i<node->child_num;i++)
    {
        rst = TreeSearch(node->child[i],func,para);
        if(rst != NULL) return rst;
    }
    
    while(node->parent != NULL)
    {
        MTreeNode *parent = node->parent;
        int flag = (mode == MORN_TREE_SEARCH_NEXT)?0:1;
        if(flag==1)
            {if(func(parent,para)==1) return parent;}
        for(int i=0;i<parent->child_num;i++)
        {
            if(parent->child[i] == node) {flag = 1;continue;}
            if(flag == 0) continue;
            
            rst = TreeSearch(parent->child[i],func,para);
            if(rst != NULL) return rst;
        }
        node = parent;
    }
    return NULL;
}

