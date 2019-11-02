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
    FILE *f = fopen(filename,"rb");
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
    
    char key_buff[1024];
    char *key = key_buff;
    
    if(tree->object == NULL)
        tree->object = mTreeNode(tree,NULL,sizeof(JSONData));
    
    MTreeNode *node = tree->object;
    JSONData *data = (JSONData *)(node->data);
    char **json = &(data->name);
    *json = p;
    
    MTreeNode *child;
    
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

char *mJSONName(MTreeNode *node)  {return (((JSONData *)(node->data))->name );}
char *mJSONValue(MTreeNode *node) {return (((JSONData *)(node->data))->value);}

void JSONRead(MTreeNode *node,char **name,int n,MList *list)
{
    if(n==1)
    {
        for(int i=0;i<node->child_num;i++)
        {
            JSONData *data = node->child[i]->data;
            if(strcmp(data->name,name[0])==0)
                if(data->value!=NULL)
                    mListWrite(list,DFLT,data->value,DFLT);
        }
        return;
    }
    for(int i=0;i<node->child_num;i++)
    {
        JSONData *data = node->child[i]->data;
        if(strcmp(data->name,name[0])==0)
            JSONRead(node->child[i],name+1,n-1,list);
    }
}

struct HandleJSONRead
{
    MList *name;
    MTree *tree;
    MList *result;
}HandleJSONRead;
void endJSONRead(void *info) 
{
    struct HandleJSONRead *handle = info;
    if(handle->tree!= NULL) mTreeRelease(handle->tree);
    if(handle->name!= NULL) mListRelease(handle->name);
    if(handle->result!=NULL)mListRelease(handle->result);
}
#define HASH_JSONRead 0x40bc5267
MList *mJSONRead(MFile *json,char *name)
{
    mException((json==NULL)||(name==NULL),EXIT,"invalid input");
    MHandle *hdl; ObjectHandle(json,JSONRead,hdl);
    struct HandleJSONRead *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        if(handle->name  ==NULL) handle->name  =mListCreate(DFLT,NULL);
        if(handle->tree  ==NULL) handle->tree  =mTreeCreate();
        if(handle->result==NULL) handle->result=mListCreate(DFLT,NULL);

        mJSONLoad(json->filename,handle->tree);
        
        hdl->valid = 1;
    }
    mStringSplit(name,".",handle->name);
    mListClear(handle->result);
    JSONRead(handle->tree->treenode,(char **)(handle->name->data),handle->name->num,handle->result);
    return handle->result;
}
    
