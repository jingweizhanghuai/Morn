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

struct HandleTreeCreate
{
    MMemory *memory;
};
void endTreeCreate(void *info)
{
    struct HandleTreeCreate *handle = (struct HandleTreeCreate *)info;
    if(handle->memory != NULL)
        mMemoryRelease(handle->memory);
}
#define HASH_TreeCreate 0x14c697eb
MTree *mTreeCreate()
{
    MTree *tree = mObjectCreate(NULL);
    
    MHandle *hdl; ObjectHandle(tree,TreeCreate,hdl);
    
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

MTreeNode *mTreeNode(MTree *tree,void *data,int size)
{
    MHandle *hdl = tree->handle->data[1];
    mException((hdl->flag != HASH_TreeCreate),EXIT,"invalid input tree");
    struct HandleTreeCreate *handle =hdl->handle;
    if(handle->memory == NULL) handle->memory = mMemoryCreate(DFLT,DFLT);
    
    MTreeNode *node = mMemoryWrite(handle->memory,NULL,sizeof(MTreeNode));
    memset(node,0,sizeof(MBtreeNode));
    node->data = mMemoryWrite(handle->memory,data,size);
    
    return node;
}

void mTreeNodeSet(MTreeNode *tree,MTreeNode *child,int order)
{
    child->parent = tree;
    
    if(order <0)
    {
        int i;
        for(i=0;i<tree->child_num;i++)
            if(tree->child[i] == NULL)
                break;
        order = i;
    }
    
    if(order >= tree->child_num)
    {
        int child_num = order+1;
        if(tree->child == NULL)
        {
            tree->child = (MTreeNode **)mMalloc(MAX(2,child_num)*sizeof(MTreeNode *));
            memset(tree->child,0,MAX(2,child_num)*sizeof(MTreeNode *));
        }
        else if((tree->child_num < child_num)&&(child_num >2))
        {
            MTreeNode **buff = (MTreeNode **)mMalloc(MAX(2,child_num)*sizeof(MTreeNode *));
            memcpy(buff,tree->child,tree->child_num*sizeof(MTreeNode *));
            memset(buff+tree->child_num,0,(child_num-tree->child_num)*sizeof(MTreeNode *));
            mFree(tree->child);
            tree->child = buff;
        }
        
        tree->child_num = child_num;
    }
    
    if(tree->child[order] == child)
        return;
    
    mException((tree->child[order]!=NULL),EXIT,"invalid operate");
    
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
void mTreeTraversal(MTree *tree,void (*func)(MTreeNode *,void *),void *para,int mode)
{
    TreeTraversal(tree->object,func,para,mode);
}

MTreeNode *TreeDecide(MTreeNode *tree,int (*func)(MTreeNode *,void *),void *para)
{
    MTreeNode *node;
    if(tree->child_num>0)
    {
        int order = func(tree,para);
        node = TreeDecide(tree->child[order],func,para);
    }
    else
        node = tree;
    
    return node;
}
MTreeNode *mTreeDecide(MTree *tree,int (*func)(MTreeNode *,void *),void *para)
{
    return TreeDecide(tree->object,func,para);
}



MTreeNode *TreeSearch(MTreeNode *node,int (*func)(MTreeNode *,void *),void *para)
{
    if(func(node,para)==1)
        return node;
    
    MTreeNode *rst;
    for(int i=0;i<node->child_num;i++)
    {
        rst = TreeSearch(node->child[i],func,para);
        if(rst != NULL) return rst;
    }
    return NULL;
}

#define MORN_TREE_SEARCH_ALL DFLT
#define MORN_TREE_SEARCH_NEXT 0
MTreeNode *mTreeSearch(MTreeNode *node,int (*func)(MTreeNode *,void *),void *para,int mode)
{
    int i;
    MTreeNode *rst;
    
    for(i=0;i<node->child_num;i++)
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
        for(i=0;i<parent->child_num;i++)
        {
            if(parent->child[i] == node) {flag = 1;continue;}
            if(flag == 0) continue;
            
            rst = TreeSearch(parent->child[i],func,func);
            if(rst != NULL) return rst;
        }
        node = parent;
    }
    return NULL;
}

/*
struct Shadow
{
    int ID;
    MTree *tree;
};
void TreeShadow(MTree *node,MTree *sd_node,int *ID)
{
    struct Shadow sd;
    sd.ID = *ID;
    sd.tree = node;
    memcpy(sd_node->data,&sd,sizeof(struct Shadow));
    
    *ID = *ID +1;
    
    int n = node->child_num;
    sd_node->child_num = n;
    if(n>0)
    {
        sd_node->child = (MTree **)mMalloc(n*sizeof(MTree *));
        for(int i=0;i<n;i++)
        {
            sd_node->child[i] = mTreeCreate(sizeof(struct Shadow),NULL);
            TreeShadow(node->child[i],sd_node->child[i],ID);
        }
    }
}

void TreeListWrite(MTree *sd_node,void *para)
{
    MList *list = (MList *)para;
    
    int child_num = sd_node->child_num;
    
    int size = sizeof(int)
             + sizeof(int)
             + sizeof(int)
             + child_num*sizeof(int)
             + sizeof(int);
             
    struct Shadow *sd = sd_node->data;
    MTree *node = sd->tree;
    
    mListWrite(list,DFLT,NULL,size+node->size);
    
    int *data = (int *)(list->data[list->num-1]);
    
    data[0] = sd->ID;
    
    struct Shadow *parent_sd = sd_node->parent->data;
    data[1] = parent_sd->ID;
    
    data[2] = child_num;
    
    for(int i=0;i<child_num;i++)
    {
        struct Shadow *child_sd = sd_node->child[i]->data;
        data[3+i] = child_sd->ID;
    }
    
    data[1+1+1+child_num] = node->size;
    
    memcpy(data+1+1+1+child_num+1,node->data,node->size);
}

void TreeToList(MTree *tree,MList *list)
{
    MTree *shadow = mTreeCreate(sizeof(struct Shadow),NULL);
    int ID = 0;
    
    TreeShadow(tree,shadow,&ID);
    
    list->num = 0;
    mTreeTraversal(shadow,TreeListWrite,(void *)list,MORN_TREE_PREORDER_TRAVERSAL);
    
    mTreeRelease(shadow);
}
*/
    
    
    
    
    
    
    
    