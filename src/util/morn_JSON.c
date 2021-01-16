/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"
#define fread(Data,Size,Num,Fl) mException(((int)fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error")

MTreeNode *mTreeNode(MTree *tree,void *data,int size);
void mTreeSet(MTreeNode *tree,MTreeNode *child,int order);
void TreeNodeRelease(MTreeNode *node);

typedef struct JSONData
{
    char *name;
    char *value;
}JSONData;

void PrintNode(MTreeNode *node)
{
    char *name =NULL;
    char *value=NULL;
    JSONData *data = (JSONData *)(node->data);
    printf("data=%p,data->name=%p,data->value=%p\n",data,data->name,data->value);
    if(data!=NULL) {name = data->name;value= data->value;}
    if(name ==NULL) name = "noname";
    if(value==NULL) value="novalue";
    printf("name is %s, value is %s, child_num is %d\n",name,value,node->child_num);
}

// char *morn_json_header = "mornjson";
struct HandleJSONLoad {
    char *file;
};
void endJSONLoad(struct HandleJSONLoad *handle)
{
    if(handle->file != NULL) free(handle->file);
}
#define HASH_JSONLoad 0xa59d25b3
void JSONLoad(MTree *tree,char *filedata)
{
    char *p=filedata;
    
    char key_buff[1024];memset(key_buff,0,1024);char *key = key_buff;

    if(tree->object != NULL) TreeNodeRelease((MTreeNode *)(tree->object));
    tree->object = mTreeNode(tree,NULL,sizeof(JSONData));
    MTreeNode *node = tree->object;
    JSONData *data = (JSONData *)(node->data);
    printf("data=%p,data->name=%p,data->value=%p\n",data,data->name,data->value);
    
    char **json = &(data->name);
    
    #define DeSPACE {while((p[1]==' ')||(p[1]=='\t')||(p[1]=='\0')||(p[1]=='\n')||(p[1]=='\r')||(p[1]=='\b')) {p++;p[0]=0;}}
    while(*p)
    {
        printf("%c",*p);
        if(key[0]=='"') 
        {
            mException((p[0]=='\t')||(p[0]=='\n')||(p[0]=='\r')||(p[0]=='\b'),EXIT,"file error");
            if(p[0] == '"') {p[0]=0;key=key-1;}
            p++; continue;
        }

        if((p[0]==' ')||(p[0]=='\t')||(p[0]=='\0')||(p[0]=='\n')||(p[0]=='\r')||(p[0]=='\b')) p[0]=0;
        
        else if(p[0]=='"') {p[0]=0; *json=p+1;key=key+1; key[0]='"';}
        else if(p[0]==':') {p[0]=0; json=&(data->value); DeSPACE; *json=p+1;}
        else if(p[0]=='[')
        {
            p[0]=0;key=key+1;key[0]='[';
            MTreeNode *child=mTreeNode(tree,NULL,sizeof(JSONData));
            mTreeNodeSet(node,child,DFLT);
            node=child;data = (JSONData *)(node->data);json=&(data->value);
            DeSPACE; *json=p+1;
        }
        else if(p[0]==']') 
        {
            p[0]=0; mException((key[0]!='['),EXIT,"file error");key=key-1;
            node=node->parent;
            printf("node->child_num=%d\n",node->child_num);
        }
        else if(p[0]==',') 
        {
            p[0]=0;
            MTreeNode *child=mTreeNode(tree,NULL,sizeof(JSONData));
            mTreeNodeSet(node->parent,child,DFLT);node=child;data = (JSONData *)(node->data);json=(key[0]=='[')?(&(data->value)):(&(data->name));
            DeSPACE; *json=p+1;
        }
        else if(p[0]=='{')
        {
            p[0]=0;key=key+1;key[0]='{';
            MTreeNode *child=mTreeNode(tree,NULL,sizeof(JSONData));
            mTreeNodeSet(node,child,DFLT);
            node=child;data = (JSONData *)(node->data);json=&(data->name);
            DeSPACE; *json=p+1;
        }
        else if(p[0]=='}')
        {
            p[0]=0; mException((key[0]!='{'),EXIT,"file error");key=key-1;
            node=node->parent;
            printf("node->child_num=%d\n",node->child_num);
        }
        
        p=p+1;
    }
}

void mJSONLoad(MTree *tree,char *filename,...)
{
    va_list namepara;
    va_start (namepara,filename);
    vsnprintf(morn_filename,256,filename,namepara);
    va_end(namepara);
    FILE *f = fopen(morn_filename,"rb");
    mException((f==NULL),EXIT,"cannot open file %s",morn_filename);
    int filesize = fsize(f);
    
    MHandle *hdl=mHandle(tree,JSONLoad);
    struct HandleJSONLoad *handle = (struct HandleJSONLoad *)(hdl->handle);
    if(hdl->valid==0)
    {
        if(handle->file!=NULL) free(handle->file);
        handle->file = (char *)malloc(filesize+1); 
    }
    
    fread(handle->file,filesize,1,f);
    fclose(f);
    handle->file[filesize]=0;
    JSONLoad(tree,handle->file);
}

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

void mJSONSearch(MTree *tree,MList *result,char *name)
{
    mException((tree==NULL)||(name==NULL),EXIT,"invalid input");

    MList *name_list = mStringSplit(name,".");
    
    mListClear(result);
    int flag=0;
    for(int i=0;i<tree->treenode->child_num;i++)
    {
        if(strcmp(mJSONName(tree->treenode->child[i]),"mornjson")==0)
            JSONSearch(tree->treenode->child[i],(char **)(name_list->data),name_list->num,result);
        else flag=1;
    }
    if(flag)JSONSearch(tree->treenode          ,(char **)(name_list->data),name_list->num,result);
}

int _NodeCompare(MTreeNode *ptr,void *para) {return (strcmp(mJSONName(ptr),para)==0);}
MTreeNode *m_JSONNode(MTreeNode *treenode,const char *name,const char *format,...)
{
    MTreeNode *node = mTreeSearch(treenode,_NodeCompare,(void *)name,0);
    if((node!=NULL)&&(!INVALID_POINTER(format)))
    {
        va_list jsonpara;
        va_start(jsonpara,format);
        vsscanf(mJSONValue(node),format,jsonpara);
        va_end(jsonpara);
    }
    return node;
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
    