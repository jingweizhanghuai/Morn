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

MTreeNode *mTreeNode(MTree *tree,void *data,int size);
void mTreeSet(MTreeNode *tree,MTreeNode *child,int order);

typedef struct JSONData
{
    char *name;
    char *value;
    int count;
}JSONData;

void PrintNode(MTreeNode *node)
{
    JSONData *data = (JSONData *)(node->data);
    printf("name is %s, value is %s, child_num is %d\n",data->name,data->value,node->child_num);
}
 
struct HandleJSONLoad {
    char *file;
};
void endJSONLoad(void *info)
{
    struct HandleJSONLoad *handle = info;
    if(handle->file != NULL)
        mFree(handle->file);
}
#define HASH_JSONLoad 0xa59d25b3
void mJSONLoad(char *filename,MTree *tree)
{
    FILE *f = fopen(filename,"r");
    mException((f==NULL),EXIT,"cannot open file");
    
    fseek(f,0,SEEK_END);
    int filesize = ftell(f);
    fseek(f,0,SEEK_SET);
    
    MHandle *hdl; ObjectHandle(tree,JSONLoad,hdl);
    struct HandleJSONLoad *handle = hdl->handle;
    if(hdl->valid==1) mFree(handle->file);
    handle->file = (char *)mMalloc(filesize); hdl->valid=1;
    
    fread(handle->file,filesize,1,f);
    fclose(f);
    
    char *p=handle->file;
    
    // file[filesize-1]=0;
    // printf("%s\n",file);
    
    char key_buff[1024];
    char *key = key_buff;
    
    if(tree->object == NULL)
        tree->object = mTreeNode(tree,NULL,sizeof(JSONData));
    
    MTreeNode *node = tree->object;
    JSONData *data = (JSONData *)(node->data);
    char **json = &(data->name);
    *json = p;
    
    MTreeNode *child;
    MTreeNode *parent;
    
    while(p<handle->file+filesize)
    {
        if(key[0]=='"')
        {
            if(p[0] == '"')
            {
                p[0]=0;
                key=key-1;
            }
        }
        else if(p[0]=='"')
        {
            p[0]=0;
            *json=p+1;
            key=key+1;key[0]='"';
        }
        else if(p[0]==':')
        {
            json=&(data->value);
            *json = p+1;
        }
        else if((p[0]==' ')||(p[0]=='\t'));
        else if((p[0]=='\0')||(p[0]=='\n')||(p[0]=='\r')||(p[0]=='\b')||(p[0]=='\0'))
        {
            p[0] = 0;
            mException((key[0]=='"'),EXIT,"file error");
        }    
        else if(p[0]==',') 
        {
            p[0]=0;
            child=mTreeNode(tree,NULL,sizeof(JSONData));
            mTreeNodeSet(node->parent,child,DFLT);
            data = child->data;
            if(key[0]=='[')
            {
                data->name = ((JSONData *)(node->data))->name;
                json=&(data->value);
            }
            else
                json=&(data->name);
            node=child;
        }
        else if(p[0]=='{')
        {
            p[0]=0;
            key=key+1;key[0]='{';
            child=mTreeNode(tree,NULL,sizeof(JSONData));
            mTreeNodeSet(node,child,DFLT);
            node=child;
            data=node->data;
            json=&(data->name);
        }
        else if(p[0]=='}')
        {
            p[0]=0;
            mException((key[0]!='{'),EXIT,"file error");key=key-1;
            node=node->parent;
        }
        else if(p[0]=='[') 
        {
            p[0]=0;
            key=key+1;key[0]='[';
        }
        else if(p[0]==']') 
        {
            mException((key[0]!='['),EXIT,"file error");key=key-1;
        }
        
        p=p+1;
    }
}

/*
struct HandleJSONRead
{
    MTree *tree;
    MTreeNode *node;
}HandleJSONRead;
void endJSONRead(void *info) 
{
    struct HandleJSONRead *handle = info;
    if(handle->tree!= NULL)
        mTreeRelease(handle->tree);
}
#define HASH_JSONRead 0x40bc5267
char *mJSONRead(MProc *file,char *name)
{
    MTreeNode *node;
    JSONData *data;
    
    int handle_index;
    mHandle(file,JSONRead,handle_index);
    struct HandleJSONRead *handle = file->handle->handle[handle_index];
    if(file->handle->valid[handle_index] == 0)
    {
        handle->tree = mTreeCreate(NULL);
        mJSONLoad(file->object,handle->tree);
        
        node = (MTreeNode *)(handle->tree->object);
        data = node->data;
        data->count = 0;
        handle->node = node;
        
        file->handle->valid[handle_index]=1;
    }
    
    node = handle->node;
    if(node == handle->tree->object)
    {
        data = node->data;
        data->count += 1;
    }
    
    for(int j=0;;j++)
    {
        // printf("\n%d:\n",j);
        JSONData *data0 = node->data;
        
        int child_num = node->child_num;
        
        // printf("node name is %s,child_num is %d,count is %d\n",data0->name,node->child_num,data0->count);
        int i;
        for(i=0;i<child_num;i++)
        {
            data = node->child[i]->data;
            // printf("child name is %s,count is %d\n",data->name,data->count);
            if(data->count < data0->count)
            {
                // printf("i is %d,data->count is %d\n",i,data->count);
                // if(node->child[i] == handle->node)
                    // return NULL;
                
                data->count += 1;
                if(strcmp(name,data->name)==0)
                {
                    handle->node = node->child[i];
                    return data->value;
                }
                node = node->child[i];
                
                break;
            }
        }
       
        if(i==child_num)
        {
            // if((j>0)&&(node == handle->node))
                // return NULL;
            
            if(node->parent != NULL)
                node = node->parent;
            else
            {
                // if(node == handle->node)
                handle->node = node;
                return NULL;
                
                // data = node->data;
                // data->count += 1;
            }
        }
    }
}





char *mJSONNodeName(MTreeNode *node)  {return ((JSONData *)(node->data))->name ;}
char *mJSONNodeValue(MTreeNode *node) {return ((JSONData *)(node->data))->value;}
    

#define NODE_CMP(Data,Name,Value,Flag) {\
    int flag1 = (Name !=NULL)&&(Data->name !=NULL);\
    int flag2 = (value!=NULL)&&(Data->value!=NULL);\
    flag =!(flag1||flag2);\
    if((flag==0)&&flag1) flag = strcmp(Data->name ,Name );\
    if((flag==0)&&flag2) flag = strcmp(Data->value,Value);\
}

void NodeSearch(MTreeNode *node,void *para)
{
    void **p = (void **)para;
    char *name =(char *)(p[0]);
    char *value=(char *)(p[1]);
    MList *list =(MList*)(p[2]);
    
    JSONData *data = node->data;
    
    int flag;
    NODE_CMP(data,name,value,flag);
    if(flag == 0)
    {
        int n = list->num;
        mListAppend(list,DFLT);
        list->data[n] = node;
        list->num = n+1;
    }
}
void TreeTraversal(MTreeNode *tree,void (*func)(MTreeNode *,void *),void *para,int mode);
void mJSONSearch(MTreeNode *node,char *name,char *value,MList *list)
{
    mException(INVALID_POINTER(node),"invalid input",EXIT);
    mException((INVALID_POINTER(name)&&INVALID_POINTER(value)),"invalid input",EXIT);
    
    mException(INVALID_POINTER(list),"invalid input",EXIT);
    mListRedefine(list,0);
   
    void *para[3] = {name,value,list};
    TreeTraversal(node,NodeSearch,para,MORN_TREE_PREORDER_TRAVERSAL);
}
*/
    
    
   
    
    
    
    
    
    

    

    

