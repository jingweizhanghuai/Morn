/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Util.h"
#define fread(Data,Size,Num,Fl) mException(((int)fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error")

MTreeNode *mTreeNode(MTree *tree,void *data,int size);
void mTreeSet(MTreeNode *tree,MTreeNode *child,int order);
void TreeNodeRelease(MTreeNode *node);

typedef struct JSONData
{
    char *name;
    char *value;
    //int count;
}JSONData;

void PrintNode(MTreeNode *node)
{
    JSONData *data = (JSONData *)(node->data);
    printf("name is %s, value is %s, child_num is %d\n",data->name,data->value,node->child_num);
}
 
struct HandleJSONLoad {
    char header[16];
    char *file;
};
void endJSONLoad(void *info)
{
    struct HandleJSONLoad *handle = (struct HandleJSONLoad *)info;
    if(handle->file != NULL)
        mFree(handle->file);
}
#define HASH_JSONLoad 0xa59d25b3
void JSONLoad(MTree *tree,char *filename)
{
    FILE *f = fopen(filename,"rb");
    mException((f==NULL),EXIT,"cannot open file %s",filename);

    fseek(f,0,SEEK_END);
    int filesize = ftell(f);
    fseek(f,0,SEEK_SET);
    
    MHandle *hdl=mHandle(tree,JSONLoad);
    struct HandleJSONLoad *handle = (struct HandleJSONLoad *)(hdl->handle);
    if(hdl->valid==0)
    {
        if(handle->file!=NULL) mFree(handle->file);
        handle->file = (char *)mMalloc(filesize); 

        strcpy(handle->header,"mornjson\0");
        hdl->valid=1;
    }
    
    fread(handle->file,filesize,1,f);
    fclose(f);
    char *p=handle->file;
    
    char key_buff[1024];memset(key_buff,0,1024);
    char *key = key_buff;

    if(tree->object != NULL) TreeNodeRelease((MTreeNode *)(tree->object));
    tree->object = mTreeNode(tree,NULL,sizeof(JSONData));
    
    MTreeNode *node = mTreeNode(tree,NULL,sizeof(JSONData));
    mTreeNodeSet(tree->treenode,node,DFLT);
    JSONData *data = (JSONData *)(node->data);
    data->name = handle->header;
    data->value= p;
    
    char **json = &(data->name);
    
    #define DeSPACE {while((p[1]==' ')||(p[1]=='\t')||(p[1]=='\0')||(p[1]=='\n')||(p[1]=='\r')||(p[1]=='\b')) {p++;p[0]=0;}}
    while(p<handle->file+filesize)
    {
        if(key[0]=='"') 
        {
            //printf("key[0] is %c\n",key[0]);
            mException((p[0]=='\t')||(p[0]=='\n')||(p[0]=='\r')||(p[0]=='\b'),EXIT,"file error");
            if(p[0] == '"') {p[0]=0;key=key-1;}
            p++; continue;
        }

        if((p[0]==' ')||(p[0]=='\t')||(p[0]=='\0')||(p[0]=='\n')||(p[0]=='\r')||(p[0]=='\b')) p[0]=0;
        else if(p[0]=='"') {p[0]=0; *json=p+1; key=key+1; key[0]='"';}
        else if(p[0]==':') {p[0]=0; json=&(data->value); DeSPACE; *json=p+1;}
        else if(p[0]=='[') {p[0]=0; key=key+1;key[0]='[';json=&(data->value);DeSPACE; *json=p+1;}
        else if(p[0]==']') {p[0]=0; mException((key[0]!='['),EXIT,"file error");key=key-1;}
        else if(p[0]==',') 
        {
            p[0]=0;
            
            MTreeNode *child=mTreeNode(tree,NULL,sizeof(JSONData));
            mTreeNodeSet(node->parent,child,DFLT);
            data = (JSONData *)(child->data);
            
            if(key[0]=='[') {data->name = ((JSONData *)(node->data))->name;json=&(data->value);}
            else json=&(data->name);
            DeSPACE; *json=p+1;
                
            node=child;
        }
        else if(p[0]=='{')
        {
            p[0]=0;key=key+1;key[0]='{';
            
            MTreeNode *child=mTreeNode(tree,NULL,sizeof(JSONData));
            mTreeNodeSet(node,child,DFLT);
            data = (JSONData *)(child->data);
     
            json=&(data->name);
            DeSPACE; *json=p+1;
            
            node=child;
        }
        else if(p[0]=='}')
        {
            p[0]=0;
            mException((key[0]!='{'),EXIT,"file error");key=key-1;
            node=node->parent;
        }
        
        p=p+1;
    }
}

/*
void mJSONLoad1(char *filename,MTree *tree)
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
    //MTreeNode *parent;
    
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
*/

char *mJSONName(MTreeNode *node)  {return (((JSONData *)(node->data))->name );}
char *mJSONValue(MTreeNode *node) {return (((JSONData *)(node->data))->value);}

/*
int JSONNodeCheck(MTreeNode *node,void *para)
{
    MList *regular = para;
    char buff[256];
    for(int i=regular->num-1;i>=0;i--)
    {
        JSONData *data = node->data;
        if(strcmp(regular->data[i],data->name)==0) {node=node->parent;continue;}
        if(data->value==NULL) return 0;
        sprintf(buff,"%s=%s",data->name,data->value);
        if(strcmp(regular->data[i],buff)==0) {node=node->parent;continue;}
        return 0;
    }
    return 1;
}
MTreeNode *mJSONNode(MTree *tree,char **regular,int regular_num)
{
    MList *list = mListCreate(regular_num,(void **)regular);
    return mTreeSearch(tree,

    
    mListClear(handle->regular);
    char *p = handle->name;
    for(j=0;j<regular_num;j++)
    {
        JSONData data;
        int l=strlen(regular[j])+1;
        memcpy(p,regular[j],l);
        data.name = p;data.value=NULL;
        int n=0; 
        for(i=0;i<l;i++) if(p[i]=='=') {p[i]=0;n=i+1;break;}
        if(n!=0) 
        {
            if(p[n+1]=='=') p[n+1]=' ';
            for(i=n;i>0;i--) {if(p[i]==' ')p[i]=0;else break;}
            for(i=n;i<l;i++) {if(p[i]==' ')p[i]=0;else break;}
            data.value=p+i;
        }
        mListWrite(handle->regular,j,&data,sizeof(JSONData));
    }
    
}
*/

void JSONSearch(MTreeNode *node,char **name,int n,MList *list)
{
    if(n==1)
    {
        for(int i=0;i<node->child_num;i++)
        {
            JSONData *data = (JSONData *)(node->child[i]->data);
            if(strcmp(data->name,name[0])==0)
                if(data->value!=NULL)
                    mListWrite(list,DFLT,data->value,DFLT);
        }
        return;
    }
    for(int i=0;i<node->child_num;i++)
    {
        JSONData *data = (JSONData *)(node->child[i]->data);
        if(strcmp(data->name,name[0])==0)
            JSONSearch(node->child[i],name+1,n-1,list);
    }
}

struct HandleJSONSearch
{
    MList *name;
};
void endJSONSearch(void *info) 
{
    struct HandleJSONSearch *handle = (struct HandleJSONSearch *)info;
    if(handle->name!= NULL) mListRelease(handle->name);
}
#define HASH_JSONSearch 0x40bc5267
void mJSONSearch(MTree *tree,MList *result,char *name)
{
    mException((tree==NULL)||(name==NULL),EXIT,"invalid input");
    MHandle *hdl=mHandle(tree,JSONSearch);
    struct HandleJSONSearch *handle = (struct HandleJSONSearch *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->name  ==NULL) handle->name  =mListCreate(DFLT,NULL);
        hdl->valid = 1;
    }

    mStringSplit(name,".",handle->name);
    
    mListClear(result);
    int flag=0;
    for(int i=0;i<tree->treenode->child_num;i++)
    {
        if(strcmp(mJSONName(tree->treenode->child[i]),"mornjson")==0)
            JSONSearch(tree->treenode->child[i],(char **)(handle->name->data),handle->name->num,result);
        else flag=1;
    }
    if(flag)JSONSearch(tree->treenode          ,(char **)(handle->name->data),handle->name->num,result);
}

// int _JSONNode(MTreeNode *ptr,void *para) {return (strcmp(mJSONName(ptr),para)==0);}
// MTreeNode *mJSONNode(MTree *tree,char *name)
// {
//     node = json->treenode;
//     return mTreeSearch(node,_JSONNode,name,0);
// }

    


    

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
    