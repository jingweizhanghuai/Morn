/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"

#define fread(Data,Size,Num,Fl) mException(((int)fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error")

char *morn_json_type[15]={"UNKNOWN","KEY_UNKNOWN","BOOL","KEY_BOOL","INT","KEY_INT","DOUBLE","KEY_DOUBLE","STRING","KEY_STRING","LIST","KEY_LIST","ARRAY","KEY_ARRAY","UNKNOWN"};

union JsonData
{
    bool   dataBool;
    int    dataS32;
    double dataD64;
    char   *string;
    struct {uint16_t num;uint16_t layer;int32_t idx;};
    void *handle0;
};

struct ListNode
{
    union JsonData data;
    char type;
    uint32_t li:24;
    char *key;
};
struct ListLayer
{
    struct ListNode *node;
    int cap;
    int num;
};

struct ArrayNode
{
    union JsonData data;
    char type;
    uint32_t li:24;
};
struct ArrayLayer
{
    struct ArrayNode *node;
    int cap;
    int num;
};

struct ListNode *ListLayerNode(struct ListLayer *layer)
{
    int num = layer->num;
    if(num>=layer->cap)
    {
        int n=MAX(256,num+num);//printf("n=%d,num=%d\n",n,num);
        struct ListNode *buff=mMalloc(n*sizeof(struct ListNode));
        memcpy(buff,layer->node,num*sizeof(struct ListNode));if(num>1) mFree(layer->node);
        layer->node=buff;layer->cap=n;
    }
    layer->num=num+1;
    return (layer->node+num);
}
struct ArrayNode *ArrayLayerNode(struct ArrayLayer *layer)
{
    int num = layer->num;
    if(num>=layer->cap)
    {
        int n=MAX(256,num+num);
        struct ArrayNode *buff=mMalloc(n*sizeof(struct ArrayNode));
        memcpy(buff,layer->node,num*sizeof(struct ArrayNode));if(num>1) mFree(layer->node);
        layer->node=buff;layer->cap=n;
    }
    layer->num=num+1;
    return (layer->node+num);
}

int ListLayerAppend(struct ListLayer *layer,int idx,int n1,int n2,struct ListNode *node)
{
    int num0=layer->num;
    if(idx+n1==num0) {idx=idx+n1;n2=n2-n1;n1=0;}
    if(layer->cap-num0<n2)
    {
        int n=num0+MAX(num0,n2);
        struct ListNode *buff=mMalloc(n*sizeof(struct ListNode));
        memcpy(buff,layer->node,num0*sizeof(struct ListNode));if(num0>1) mFree(layer->node);
        layer->node=buff;layer->cap=n;
    }
    if(n1>0) memcpy(layer->node+num0,layer->node+idx,n1*sizeof(struct ListNode));
    int i;for(i=num0+n1;i<num0+n2-1;i++)
    {
        layer->node[i].data.string=NULL;
        layer->node[i].type=JSON_STRING;
    }
    if(node!=NULL) layer->node[i].key=node->key;
    layer->num=num0+n2;
    return num0;
}

int ArrayLayerAppend(struct ArrayLayer *layer,int idx,int n1,int n2,struct ArrayNode *node)
{
    int num0=layer->num;
    if(idx+n1==num0) {idx=idx+n1;n2=n2-n1;n1=0;}
    if(layer->cap-num0<n2)
    {
        int n=num0+MAX(num0,n2);
        struct ArrayNode *buff=mMalloc(n*sizeof(struct ArrayNode));
        memcpy(buff,layer->node,num0*sizeof(struct ArrayNode));if(num0>1) mFree(layer->node);
        layer->node=buff;layer->cap=n;
    }
    if(n1>0) memcpy(layer->node+num0,layer->node+idx,n1*sizeof(struct ArrayNode));
    int i;for(i=num0+n1;i<num0+n2-1;i++)
    {
        layer->node[i].data.string=NULL;
        layer->node[i].type=JSON_STRING;
    }
    // if(node!=NULL) layer->node[i].key=node->key;
    layer->num=num0+n2;
    return num0;
}

// int LayerCopy(struct ListLayer *src,int idx,int num,struct ListLayer *dst)
// {
//     if(dst->cap-dst->num<num)
//     {
//         int n=dst->num+MAX(dst->num,num);
//         struct ListNode *buff=mMalloc(n*sizeof(struct ListNode));
//         memcpy(buff,layer->node,dst->num*sizeof(struct ListNode));if(dst->num>1) mFree(layer->node);
//         layer->node=buff;layer->cap=n;
//     }
//     memcpy(dst->node+dst->num,src->node+idx,num*sizeof(struct ListNode)
// }

void ListLayerDelete(struct ListLayer *layer,int idx,int num,int del)
{
    memmove(layer->node+idx+del,layer->node+idx+del+1,(num-del-1)*sizeof(struct ListNode));
}

void ArrayLayerDelete(struct ArrayLayer *layer,int idx,int num,int del)
{
    memmove(layer->node+idx+del,layer->node+idx+del+1,(num-del-1)*sizeof(struct ArrayNode));
}

struct HandleJSON
{
    struct ListLayer   list_layer[64];
    struct ArrayLayer array_layer[64];
    struct ListNode    list_node0[2];
    struct ArrayNode  array_node0[2];
    
    char *file;
    int file_size;
    MMemory *memory;
};
#define HASH_JSON 0x1eb02389
void endJSON(struct HandleJSON *handle)
{
    for(int i=0;i<64;i++) 
    {
        if(handle-> list_layer[i].num>1) {mFree(handle-> list_layer[i].node);}
        if(handle->array_layer[i].num>1) {mFree(handle->array_layer[i].node);}
    }
    if(handle->file  !=NULL) mFree(handle->file);
    if(handle->memory!=NULL) mMemoryRelease(handle->memory);
}

static double morn_char_number[20][10]={
    {0.0,0.1000000000000000000,0.2000000000000000000,0.3000000000000000000,0.4000000000000000000,0.5000000000000000000,0.6000000000000000000,0.7000000000000000000,0.8000000000000000000,0.9000000000000000000},
    {0.0,0.0100000000000000000,0.0200000000000000000,0.0300000000000000000,0.0400000000000000000,0.0500000000000000000,0.0600000000000000000,0.0700000000000000000,0.0800000000000000000,0.0900000000000000000},
    {0.0,0.0010000000000000000,0.0020000000000000000,0.0030000000000000000,0.0040000000000000000,0.0050000000000000000,0.0060000000000000000,0.0070000000000000000,0.0080000000000000000,0.0090000000000000000},
    {0.0,0.0001000000000000000,0.0002000000000000000,0.0003000000000000000,0.0004000000000000000,0.0005000000000000000,0.0006000000000000000,0.0007000000000000000,0.0008000000000000000,0.0009000000000000000},
    {0.0,0.0000100000000000000,0.0000200000000000000,0.0000300000000000000,0.0000400000000000000,0.0000500000000000000,0.0000600000000000000,0.0000700000000000000,0.0000800000000000000,0.0000900000000000000},
    {0.0,0.0000010000000000000,0.0000020000000000000,0.0000030000000000000,0.0000040000000000000,0.0000050000000000000,0.0000060000000000000,0.0000070000000000000,0.0000080000000000000,0.0000090000000000000},
    {0.0,0.0000001000000000000,0.0000002000000000000,0.0000003000000000000,0.0000004000000000000,0.0000005000000000000,0.0000006000000000000,0.0000007000000000000,0.0000008000000000000,0.0000009000000000000},
    {0.0,0.0000000100000000000,0.0000000200000000000,0.0000000300000000000,0.0000000400000000000,0.0000000500000000000,0.0000000600000000000,0.0000000700000000000,0.0000000800000000000,0.0000000900000000000},
    {0.0,0.0000000010000000000,0.0000000020000000000,0.0000000030000000000,0.0000000040000000000,0.0000000050000000000,0.0000000060000000000,0.0000000070000000000,0.0000000080000000000,0.0000000090000000000},
    {0.0,0.0000000001000000000,0.0000000002000000000,0.0000000003000000000,0.0000000004000000000,0.0000000005000000000,0.0000000006000000000,0.0000000007000000000,0.0000000008000000000,0.0000000009000000000},
    {0.0,0.0000000000100000000,0.0000000000200000000,0.0000000000300000000,0.0000000000400000000,0.0000000000500000000,0.0000000000600000000,0.0000000000700000000,0.0000000000800000000,0.0000000000900000000},
    {0.0,0.0000000000010000000,0.0000000000020000000,0.0000000000030000000,0.0000000000040000000,0.0000000000050000000,0.0000000000060000000,0.0000000000070000000,0.0000000000080000000,0.0000000000090000000},
    {0.0,0.0000000000001000000,0.0000000000002000000,0.0000000000003000000,0.0000000000004000000,0.0000000000005000000,0.0000000000006000000,0.0000000000007000000,0.0000000000008000000,0.0000000000009000000},
    {0.0,0.0000000000000100000,0.0000000000000200000,0.0000000000000300000,0.0000000000000400000,0.0000000000000500000,0.0000000000000600000,0.0000000000000700000,0.0000000000000800000,0.0000000000000900000},
    {0.0,0.0000000000000010000,0.0000000000000020000,0.0000000000000030000,0.0000000000000040000,0.0000000000000050000,0.0000000000000060000,0.0000000000000070000,0.0000000000000080000,0.0000000000000090000},
    {0.0,0.0000000000000001000,0.0000000000000002000,0.0000000000000003000,0.0000000000000004000,0.0000000000000005000,0.0000000000000006000,0.0000000000000007000,0.0000000000000008000,0.0000000000000009000},
    {0.0,0.0000000000000000100,0.0000000000000000200,0.0000000000000000300,0.0000000000000000400,0.0000000000000000500,0.0000000000000000600,0.0000000000000000700,0.0000000000000000800,0.0000000000000000900},
    {0.0,0.0000000000000000010,0.0000000000000000020,0.0000000000000000030,0.0000000000000000040,0.0000000000000000050,0.0000000000000000060,0.0000000000000000070,0.0000000000000000080,0.0000000000000000090},
    {0.0,0.0000000000000000001,0.0000000000000000002,0.0000000000000000003,0.0000000000000000004,0.0000000000000000005,0.0000000000000000006,0.0000000000000000007,0.0000000000000000008,0.0000000000000000009},
    {0.0,0.0000000000000000000,0.0000000000000000000,0.0000000000000000000,0.0000000000000000000,0.0000000000000000000,0.0000000000000000000,0.0000000000000000000,0.0000000000000000000,0.0000000000000000000},
};

char *StringNumber(char *p,union JsonData *data,char *type)
{
    *type=JSON_INT;
    int flag=0;int v=0;double d=0;
         if(*p=='-') {flag=1;p++;}
    else if(*p=='+')         p++;
    for(;(*p>='0')&&(*p<='9');p++) v=v*10+(*p-'0');
    if(*p=='.')
    {
        d=(double)v;p++;
        int i;for(i=0;(p[i]>='0')&&(p[i]<='9');i++) d=d+morn_char_number[MIN(i,19)][p[i]-'0'];
        *type=JSON_DOUBLE;p=p+i;
    }
    if((*p=='e')||(*p=='E'))
    {
        if(*type==JSON_INT) {d=(double)v;*type=JSON_DOUBLE;}
        p++;
        int f=0;if(*p=='-') {f=1;p++;} else if(*p=='+') {p++;}
        int e=0;for(;(*p>='0')&&(*p<='9');p++) e=e*10+(*p-'0');
        d=d*pow(10.0,((f==1)?(0-e):e));
    }
    if(*type==JSON_DOUBLE) {data->dataD64=(flag)?(0-d):d;return p;}
    data->dataS32=(flag)?(0-v):v;return p;
}

char *JSONString(char *p)
{
    int i,j;
    for(i=0;(p[i]!='"')&&(p[i]!='\\');i++);
    j=i;
    for(;p[i]!='"';i++,j++)
    {
        if(p[i]=='\\')
        {
            i++; if(p[i]=='n') p[j]='\n';
            else if(p[i]=='t') p[j]='\t';
            else if(p[i]=='r') p[j]='\r';
            else if(p[i]=='b') p[j]='\b';
            else if(p[i]=='f') p[j]='\f';
            else if(p[i]=='v') p[j]='\v';
            else               p[j]=p[i];
        }
        else p[j]=p[i];
    }
    p[j]=0;
    return (p+i);
}

void JSONArrayLoad(char **file,struct HandleJSON *handle,int l);
void JSONListLoad(char **file,struct HandleJSON *handle,int l)
{
    struct ListLayer *layer = handle->list_layer+l;
    char *p=*file;int flag=0;
    struct ListNode *node=ListLayerNode(layer);
    
    while(1)
    {
        p++;
        if(p[0]<=' ') continue;
        else if(p[0]=='"')
        {
            p=p+1;
            if(flag==0) {node->key=p;{for(;p[0]!='"';p++);}p[0]=0;node->key[-1]=p-node->key;}
            else        {node->data.string=p;node->type=JSON_KEY_STRING;p=JSONString(p);}
        }
        else if(p[0]==':') {mException(flag==1,EXIT,"json file error");flag=1;}
        else if(p[0]==',') {node=ListLayerNode(layer);flag=0;node->data.string=NULL;}
        else if(p[0]=='}') {*file=p;return;}
        else if(p[0]=='{') 
        {
            int n=handle->list_layer[l+1].num;
            JSONListLoad( &p,handle,l+1);
            node->data.idx  =n;
            node->data.num  =(handle->list_layer[l+1].num-n);
            node->data.layer=l+1;
            node->type=JSON_KEY_LIST;
            node->li = layer->num-1;
        }
        else if(p[0]=='[') 
        {
            int n=handle->array_layer[l+1].num;
            JSONArrayLoad(&p,handle,l+1);
            node->data.idx  =n;
            node->data.num  =(handle->array_layer[l+1].num-n);
            node->data.layer=l+1;
            node->type=JSON_KEY_ARRAY;
            node->li = layer->num-1;
        }
        else if(p[0]=='t') {node->type=JSON_KEY_BOOL  ;node->data.dataBool = 1;p+=3;}
        else if(p[0]=='f') {node->type=JSON_KEY_BOOL  ;node->data.dataBool = 0;p+=4;}
        else if(p[0]=='n') {node->type=JSON_KEY_STRING;node->data.string =NULL;p+=3;}
        else               {p=StringNumber(p,&(node->data),&(node->type))-1;node->type++;}
    }
}

void JSONArrayLoad(char **file,struct HandleJSON *handle,int l)
{
    struct ArrayLayer *layer = handle->array_layer+l;
    char *p=*file;
    
    while(1)
    {
        p++;
        if((p[0]<=' ')||(p[0]==',')) continue;
        else if(p[0]==']') {*file=p;return;}

        struct ArrayNode *node=ArrayLayerNode(layer);
        if(p[0]=='{')
        {
            int n=handle->list_layer[l+1].num;
            JSONListLoad( &p,handle,l+1);
            node->data.idx  =n;
            node->data.num  =(handle->list_layer[l+1].num-n);
            node->data.layer=l+1;
            node->type=JSON_LIST;
            node->li=layer->num-1;
        }
        else if(p[0]=='[') 
        {
            int n=handle->array_layer[l+1].num;
            JSONArrayLoad(&p,handle,l+1);
            node->data.idx  =n;
            node->data.num  =(handle->array_layer[l+1].num-n);
            node->data.layer=l+1;
            node->type=JSON_ARRAY;
            node->li=layer->num-1;
        }
        else if(p[0]=='"') {node->type=JSON_STRING; p=p+1;node->data.string=p;p=JSONString(p);}
        else if(p[0]=='t') {node->type=JSON_BOOL  ;node->data.dataBool = 1;p+=3;}
        else if(p[0]=='f') {node->type=JSON_BOOL  ;node->data.dataBool = 0;p+=4;}
        else if(p[0]=='n') {node->type=JSON_STRING;node->data.string=NULL;p+=3;}
        else                p=StringNumber(p,&(node->data),&(node->type))-1;
    }
}

struct JSONNode *mJSONLoad(MFile *jsonfile)
{
    mException(INVALID_POINTER(jsonfile),EXIT,"invalid input");
    
    MHandle *hdl = mHandle(jsonfile,JSON);
    struct HandleJSON *handle=hdl->handle;
    if(hdl->valid==0)
    {
        handle-> list_node0[0].data.handle0 = handle;
        handle->array_node0[0].data.handle0 = handle;
        for(int i=0;i<64;i++) 
        {
            handle-> list_layer[i].node = handle-> list_node0;handle-> list_layer[i].num=1;handle-> list_layer[i].cap=1;
            handle->array_layer[i].node = handle->array_node0;handle->array_layer[i].num=1;handle->array_layer[i].cap=1;
        }
        
        hdl->valid=1;
    }

    char *string;

    if(jsonfile->size<0) jsonfile->size=strlen(jsonfile->string);
    char *p1;for(p1=jsonfile->string;                 (*p1<=' ')||(*p1==0);p1++);
    char *p2;for(p2=jsonfile->string+jsonfile->size-1;(*p2<=' ')||(*p2==0);p2--);

    if(((*p1=='{')&&(*p2=='}'))||((*p1=='[')&&(*p2==']')))
    {
        if(handle->file_size<jsonfile->size)
        {
            if(handle->file!=NULL) mFree(handle->file);
            handle->file=(char *)mMalloc(jsonfile->size);
            handle->file_size = jsonfile->size;
        }
        memcpy(handle->file,jsonfile->string,jsonfile->size);
        string = handle->file;
        // string = jsonfile->string;
        // mObjectRedefine(jsonfile,NULL,jsonfile->size);
        // if(string!=jsonfile->string) {memcpy(jsonfile->string,string,jsonfile->size);string = jsonfile->string;}
    }
    else
    {
        FILE *f=fopen(jsonfile->filename,"rb");
        mException(f==NULL,EXIT,"invalid file");
        int size = fsize(f);
        if(handle->file_size<size)
        {
            if(handle->file!=NULL) mFree(handle->file);
            handle->file=(char *)mMalloc(size);
            handle->file_size = size;
        }
        fread(handle->file,size,1,f);
        fclose(f);
        string=handle->file;
    }

    struct ArrayNode *node0 = &(handle->array_node0[1]);
    node0->data.idx =1;node0->data.layer=1;node0->li =1;
    for(char *p=string;;p++)
    {
        if(p[0]=='{') 
        {
            JSONListLoad( &p,handle,1);
            node0->type=JSON_LIST ;
            node0->data.num =handle->list_layer[1].num-1;
            return (struct JSONNode *)node0;
        }
        if(p[0]=='[') 
        {
            JSONArrayLoad(&p,handle,1);
            node0->type=JSON_ARRAY;
            node0->data.num =handle->array_layer[1].num-1;
            return (struct JSONNode *)node0;
        }
    }
}

inline struct JSONNode *JSONSubNode(struct JSONNode *node0)
{
    struct HandleJSON *handle;int idx,li;
    if(node0->type&0x01) 
    {
        struct  ListNode * list_node=(struct ListNode *)node0;
        handle= list_node[0- list_node->li].data.handle0;
        idx=list_node->data.idx; li=list_node->data.layer;
    }
    else
    {
        struct ArrayNode *array_node=(struct ArrayNode*)node0;
        handle=array_node[0-array_node->li].data.handle0;
        idx=array_node->data.idx; li=array_node->data.layer;
    }
    if(node0->type<JSON_ARRAY) {return (struct JSONNode *)(handle-> list_layer[li].node+idx);}
    else                       {return (struct JSONNode *)(handle->array_layer[li].node+idx);}
}

struct JSONNode *JSONArrayRead(struct JSONNode *node0,char *key);
struct JSONNode *JSONListRead( struct JSONNode *node0,char *key)
{
    mException((node0->type!=JSON_LIST)&&(node0->type!=JSON_KEY_LIST),EXIT,"invalid key");
    struct JSONNode *node = JSONSubNode(node0);
    int i;for(i=0;i<node0->num;i++)
    {
        int s=node->key[-1];
        if(memcmp(key,node->key,s)==0) {if((key[s]==0)||(key[s]=='[')||(key[s]=='.')) {key+=s;break;}}
        node++;
    }
    if(i==node0->num) return NULL;
    
    if(key[0]== 0 ) return node;
    if(key[0]=='.') return JSONListRead( node,key+1);
    if(key[0]=='[') return JSONArrayRead(node,key  );
    mException(1,EXIT,"invalid key");return NULL;
}

struct JSONNode *JSONArrayRead(struct JSONNode *node0,char *key)
{
    mException((node0->type!=JSON_ARRAY)&&(node0->type!=JSON_KEY_ARRAY),EXIT,"invalid key");
    union JsonData data;char data_type;key=StringNumber(key+1,&data,&data_type)+1;int n=data.dataS32;
    if(n>=node0->num) return NULL;
    struct JSONNode *node = (struct JSONNode *)(((struct ArrayNode *)JSONSubNode(node0))+n);
    
    if(key[0]== 0 ) return node;
    if(key[0]=='.') {return JSONListRead( node,key+1);}
    if(key[0]=='[') {return JSONArrayRead(node,key  );}
    mException(1,EXIT,"invalid key");return NULL;
}

struct JSONNode *m_JSONRead(struct JSONNode *node0,intptr_t v,struct JSONNode *data)
{
    if(node0->type<JSON_LIST) return NULL;
    if(v==node0->num) return NULL;
    struct JSONNode *node;
    if(v<node0->num)
    {
        if(node0->type<JSON_ARRAY) node= (struct JSONNode *)(((struct ListNode  *)JSONSubNode(node0))+v);
        else                       node= (struct JSONNode *)(((struct ArrayNode *)JSONSubNode(node0))+v);
    }
    else
    {
        char *key=(char *)v;
        if(key[0]=='[') {node= JSONArrayRead(node0,key);}
        else            {node=  JSONListRead(node0,key);}
    }
    if(data!=NULL) *data=*node;
    return node;
}

/*
void mJSONArray(MArray *array,struct ListNode *node)
{
    struct ListNode *node0=(struct ListNode *)node;
    struct HandleJSON *handle=node0[0-node0->li].data.handle0;
    // struct HandleJSON *handle=node0[0-(((int)(node0->li1))<<16)-node0->li2].data.handle0;
    int idx=node0->data.idx;int num=node0->data.num; int li=node0->data.layer;
    array->num=num;array->element_size=sizeof(struct ListNode);array->data=handle->layer[li].node+idx;
    *(struct ListNode **)mReserve((MObject *)array,7)=node0;
}

struct ListNode *JSONArrayWrite(struct ListNode *node0,char *key);
struct ListNode *JSONListWrite( struct ListNode *node0,char *key)
{
    struct HandleJSON *handle=node0[0-node0->li].data.handle0;
    // struct HandleJSON *handle=node0[0-(((int)(node0->li1))<<16)-node0->li2].data.handle0;
    int idx=node0->data.idx;int num=node0->data.num; int li=node0->data.layer;
    
    struct ListNode *node = handle->layer[li].node+idx;
    int n;for(n=0;n<num;n++)
    {
        int s=node->key[-1];
        if(memcmp(key,node->key,s)==0) {if((key[s]==0)||(key[s]=='[')||(key[s]=='.')) {key+=s;break;}}
        node++;
    }
    if(n==num)
    {
        if(handle->memory==NULL) handle->memory=mMemoryCreate(DFLT,DFLT,MORN_HOST);
        
        struct ListNode nd;
        char *p=key; for(;(p[0]!=0)&&(p[0]!='[')&&(p[0]!='.');p++);
        nd.key=mMemoryWrite(handle->memory,key-1,DFLT);
        nd.key[-1]=p-key;
        
        node0->data.idx  =LayerAppend(handle->layer+li,idx,num,num+1,&nd);
        node0->data.num  =n;
        node0->data.layer=li;
        
        key=p;
        node =handle->layer[li].node+node0->data.idx+n;
    }
    
    if(key[0]== 0 ) return node;
    if(key[0]=='.') 
    {
        struct ListNode *pnode= JSONListWrite( node,key+1);
        node->type=JSON_LIST;node->data.num=MAX(num,n+1);node->data.layer=li+1;node->data.idx=pnode-handle->layer[li].node;
        return pnode;
    }
    if(key[0]=='[') 
    {
        mException(node->type!=JSON_ARRAY,EXIT,"invalid key");
        struct ListNode *pnode= JSONArrayWrite(node,key  );
        node->type=JSON_ARRAY;node->data.num=MAX(num,n+1);node->data.layer=li+1;node->data.idx=pnode-handle->layer[li].node;
        return pnode;
    }
    mException(1,EXIT,"invalid key");return NULL;
}

struct ListNode *JSONArrayWrite(struct ListNode *node0,char *key)
{
    // struct HandleJSON *handle=node0[0-(((int)(node0->li1))<<16)-node0->li2].data.handle0;
    struct HandleJSON *handle=node0[0-node0->li].data.handle0;
    int idx=node0->data.idx;int num=node0->data.num; int li=node0->data.layer;

    union JsonData data;char data_type;key=StringNumber(key+1,&data,&data_type)+1;int n=data.dataS32;
    if(n>=num)
    {
        node0->data.idx  =LayerAppend(handle->layer+li,idx,num,n,NULL);
        node0->data.num  =n;
        node0->data.layer=li;
    }
    struct ListNode *node = handle->layer[li].node+node0->data.idx+n;
    
    if(key[0]== 0 ) return node;
    if(key[0]=='.') 
    {
        mException(node->type!=JSON_LIST ,EXIT,"invalid key");
        struct ListNode *pnode= JSONListWrite( node,key+1);
        node->type=JSON_LIST;node->data.num=MAX(num,n+1);node->data.layer=li+1;node->data.idx=pnode-handle->layer[li].node;
        return pnode;
    }
    else if(key[0]=='[') 
    {
        mException(node->type!=JSON_ARRAY,EXIT,"invalid key");
        struct ListNode *pnode= JSONArrayWrite(node,key  );
        node->type=JSON_ARRAY;node->data.num=MAX(num,n+1);node->data.layer=li+1;node->data.idx=pnode-handle->layer[li].node;
        return pnode;
    }
    else mException(1,EXIT,"invalid key");
    return NULL;
}
*/

/*
void JSONCopy(struct ListNode *src_node,struct ListNode *dst_node);
{
    struct HandleJSON *handle;
    handle=src_node[0-(((int)(src_node->li1))<<16)-src_node->li2].data.handle0;
    int src_idx=src_node->data.idx;int src_num=(src_node->data.flag)>>8; int src_li=(src_node->data.flag)&0x0ff;
    struct ListLayer *src_layer = handle->layer[src_li];



    handle=dst_node[0-(((int)(dst_node->li1))<<16)-dst_node->li2].data.handle0;
    int dst_idx=dst_node->data.idx;int dst_num=(dst_node->data.flag)>>8; int dst_li=(dst_node->data.flag)&0x0ff;
    dst
    

    if(dst->cap-dst->num<src_num)
    
    // if(dst->cap-dst->num<num)
//     {
//         int n=dst->num+MAX(dst->num,num);
//         struct ListNode *buff=mMalloc(n*sizeof(struct ListNode));
//         memcpy(buff,layer->node,dst->num*sizeof(struct ListNode));if(dst->num>1) mFree(layer->node);
//         layer->node=buff;layer->cap=n;
//     }
    
}
*/
/*
struct ListNode *m_JSONWrite(struct ListNode *node_0,char *key,struct ListNode *data)
{
    struct ListNode *node0=(struct ListNode *)node_0;
    mException(INVALID_POINTER(node0)||INVALID_POINTER(key),EXIT,"invalid input");
    
    struct ListNode *node=NULL;
    if(key[0]=='[') {mException(node0->type!=JSON_ARRAY,EXIT,"invalid key");node=JSONArrayRead(node0,key);}
    else            {mException(node0->type!=JSON_LIST ,EXIT,"invalid key");node= JSONListRead(node0,key);}
    
    if(node==NULL) return NULL;
    if(data!=NULL) *node=*(struct ListNode *)data;
    return (struct ListNode *)node;
}
*/

/*
void m_JSONArray0(MArray *in,MArray *out,int n)
{
    mException(INVALID_POINTER(in),EXIT,"invalid input");
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)in,6);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    *(struct HandleJSON **)mReserve((MObject *)out,6) = handle;
    
    struct ListNode *node = (struct ListNode *)(in->dataS8+sizeof(struct ListNode)*n);
    mException((node->type<JSON_LIST),EXIT,"invalid input");
    *(struct ListNode **)mReserve((MObject *)out,7) = node;
    int idx=node->data.idx;int num=(node->data.flag)>>8; int layer=(node->data.flag)&0x0ff;
    out->num=num;out->element_size=sizeof(struct ListNode);out->data=handle->layer[layer].node+idx;
}

void m_JSONArray1(MArray *in,MArray *out,char *key)
{
    mException(INVALID_POINTER(in),EXIT,"invalid input");
    if(key==NULL) {out->num=in->num;out->element_size=in->element_size;out->data=in->data;return;}
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)in,6);
    struct ListNode *node0=*(struct ListNode **)mReserve((MObject *)in,7);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    *(struct HandleJSON **)mReserve((MObject *)out,6) = handle;

    struct ListNode *node =NULL;
    if(key[0]=='[') {mException(node0->type!=JSON_ARRAY,EXIT,"invalid key");node=JSONArrayRead(node0,handle,key);}
    else            {mException(node0->type!=JSON_LIST ,EXIT,"invalid key");node= JSONListRead(node0,handle,key);}
    mException(node==NULL,EXIT,"invalid key");
    *(struct ListNode **)mReserve((MObject *)out,7) = node;
    int idx=node->data.idx;int num=(node->data.flag)>>8; int layer=(node->data.flag)&0x0ff;
    out->num=num;out->element_size=sizeof(struct ListNode);out->data=handle->layer[layer].node+idx;
}

struct ListNode *JSONArrayWrite(struct ListNode *node0,struct HandleJSON *handle,char *key);
struct ListNode *JSONListWrite( struct ListNode *node0,struct HandleJSON *handle,char *key)
{
    int idx=node0->data.idx;int num=(node0->data.flag)>>8; int ll=(node0->data.flag)&0x0ff;
    
    struct ListNode *node = handle->layer[ll].node+idx;
    int n;for(n=0;n<num;n++)
    {
        int s=node->ksize;
        if(memcmp(key,node->key,s)==0) {if((key[s]==0)||(key[s]=='[')||(key[s]=='.')) {key+=s;break;}}
        node++;
    }
    if(n==num)
    {
        if(handle->memory==NULL) handle->memory=mMemoryCreate(DFLT,DFLT,MORN_HOST);
        
        struct ListNode nd;
        char *p=key; for(;(p[0]!=0)&&(p[0]!='[')&&(p[0]!='.');p++);
        nd.ksize=p-key;
        nd.key=mMemoryWrite(handle->memory,key,DFLT);
        
        node0->data.idx=LayerAppend(handle->layer+ll,idx,num,num+1,&nd);
        node0->data.flag =((n+1)<<8)+ll;
        
        key=p;
        node =handle->layer[ll].node+node0->data.idx+n;
    }
    
    if(key[0]== 0 ) return node;
    if(key[0]=='.') 
    {
        mException(node->type!=JSON_LIST ,EXIT,"invalid key");
        struct ListNode *pnode= JSONListWrite( node,handle,key+1);
        node->type=JSON_LIST;node->data.flag=(MAX(num,n+1)<<8)+ll+1;node->data.idx=pnode-handle->layer[ll].node;
        return pnode;
    }
    if(key[0]=='[') 
    {
        mException(node->type!=JSON_ARRAY,EXIT,"invalid key");
        struct ListNode *pnode= JSONArrayWrite(node,handle,key  );
        node->type=JSON_ARRAY;node->data.flag=(MAX(num,n+1)<<8)+ll+1;node->data.idx=pnode-handle->layer[ll].node;
        return pnode;
    }
    mException(1,EXIT,"invalid key");return NULL;
}

struct ListNode *JSONArrayWrite(struct ListNode *node0,struct HandleJSON *handle,char *key)
{
    int idx=node0->data.idx;int num=(node0->data.flag)>>8; int ll=(node0->data.flag)&0x0ff;

    union JsonData data;char data_type;key=StringNumber(key+1,&data,&data_type)+1;int n=data.dataS32;
    if(n>=num)
    {
        node0->data.idx =LayerAppend(handle->layer+ll,idx,num,n,NULL);
        node0->data.flag=((n+1)<<8)+ll;
    }
    struct ListNode *node = handle->layer[ll].node+node0->data.idx+n;
    
    if(key[0]== 0 ) return node;
    if(key[0]=='.') 
    {
        mException(node->type!=JSON_LIST ,EXIT,"invalid key");
        struct ListNode *pnode= JSONListWrite( node,handle,key+1);
        node->type=JSON_LIST;node->data.flag=(MAX(num,n+1)<<8)+ll+1;node->data.idx=pnode-handle->layer[ll].node;
        return pnode;
    }
    else if(key[0]=='[') 
    {
        mException(node->type!=JSON_ARRAY,EXIT,"invalid key");
        struct ListNode *pnode= JSONArrayWrite(node,handle,key  );
        node->type=JSON_ARRAY;node->data.flag=(MAX(num,n+1)<<8)+ll+1;node->data.idx=pnode-handle->layer[ll].node;
        return pnode;
    }
    else mException(1,EXIT,"invalid key");
    return NULL;
}

struct ListNode *m_JSONWrite0(MArray *json,int n,struct ListNode *data)
{
    // if(data->type<0) data->type=JSON_STRING;
    mException(INVALID_POINTER(json),EXIT,"invalid input");
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)json,6);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    
    struct ListNode *node=NULL;
    if(n<0) n=json->num;
    if(n>=json->num)
    {
        struct ListNode *node0=*(struct ListNode **)mReserve((MObject *)json,7);
        mException((node0->type!=JSON_ARRAY),EXIT,"invalid json");
        int idx=node0->data.idx;int ll=(node0->data.flag)&0x0ff;
        node0->data.idx=LayerAppend(handle->layer+ll,idx,json->num,n);
        node0->data.flag =((n+1)<<8)+ll;
    }
    else node=(struct ListNode *)(json->dataS8+json->element_size*n);
    
    node->type=type;
    if(type==JSON_BOOL  ) {node->data.dataBool=*(char   *)data;return &(node->data.dataBool);}
    if(type==JSON_INT   ) {node->data.dataS32 =*(int    *)data;return &(node->data.dataS32);}
    if(type==JSON_DOUBLE) {node->data.dataD64 =*(double *)data;return &(node->data.dataD64);}
    if(type==JSON_STRING) {if(handle->memory==NULL){handle->memory=mMemoryCreate(DFLT,DFLT,MORN_HOST);} node->data.string =mMemoryWrite(handle->memory,data,DFLT);return node->data.string;}
    MArray *a=(MArray *)data;
    struct ListNode *pnode=*(struct ListNode **)mReserve((MObject *)a,7);
    mException((pnode->type==JSON_ARRAY)!=(a->element_size==sizeof(struct ListNode)),EXIT,"invalid json");
    node->data=pnode->data;
    return data;
}

void *m_JSONWrite1(MArray *json,char *key,void *data,int type)
{
    if(type<0) type=JSON_STRING;
    mException(INVALID_POINTER(json)||INVALID_POINTER(key),EXIT,"invalid input");
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)json,6);
    struct ListNode *node0=*(struct ListNode **)mReserve((MObject *)json,7);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    // mException((node0->type==JSON_ARRAY)!=(json->element_size==sizeof(struct ListNode)),EXIT,"invalid json");

    struct ListNode *node=NULL;
    if(key[0]=='[') {mException(node0->type!=JSON_ARRAY,EXIT,"invalid key");node=JSONArrayWrite(node0,handle,key);}
    else            {mException(node0->type!=JSON_LIST ,EXIT,"invalid key");node= JSONListWrite(node0,handle,key);}
    
    node->type=type;
    if(type==JSON_BOOL  ) {node->data.dataBool=*(char   *)data;return &(node->data.dataBool);}
    if(type==JSON_INT   ) {node->data.dataS32=*(int    *)data;return &(node->data.dataS32);}
    if(type==JSON_DOUBLE) {node->data.dataD64=*(double *)data;return &(node->data.dataD64);}
    if(type==JSON_STRING) {if(handle->memory==NULL){handle->memory=mMemoryCreate(DFLT,DFLT,MORN_HOST);} node->data.string =mMemoryWrite(handle->memory,data,DFLT);return node->data.string;}
    MArray *a=(MArray *)data;
    struct ListNode *pnode=*(struct ListNode **)mReserve((MObject *)a,7);
    mException((pnode->type==JSON_ARRAY)!=(a->element_size==sizeof(struct ListNode)),EXIT,"invalid json");
    node->data=pnode->data;
    return data;
}

void JSONArrayDelete(void *pnode,struct HandleJSON *handle,char *key);
void JSONListDelete(void *pnode,struct HandleJSON *handle,char *key)
{
    struct ListNode *node0 = pnode;
    int idx=node0->data.idx;int num=(node0->data.flag)>>8; int ll=(node0->data.flag)&0x0ff;
    
    struct ListNode *node = handle->layer[ll].node+idx;
    int n;for(n=0;n<num;n++)
    {
        int s=node->ksize;
        if(memcmp(key,node->key,s)==0) {if((key[s]==0)||(key[s]=='[')||(key[s]=='.')) {key+=s;break;}}
        node++;
    }
    if(n==num) return;
    
    if(key[0]== 0 ) {LayerDelete(handle->layer+ll,idx,num,n);return;}
    if(key[0]=='.') {mException(node->type!=JSON_LIST ,EXIT,"invalid key");JSONListDelete( node,handle,key+1);return;}
    if(key[0]=='[') {mException(node->type!=JSON_ARRAY,EXIT,"invalid key");JSONArrayDelete(node,handle,key  );return;}
    mException(1,EXIT,"invalid key");
}

void JSONArrayDelete(void *pnode,struct HandleJSON *handle,char *key)
{
    struct ListNode *node0 = pnode;
    int idx=node0->data.idx;int num=(node0->data.flag)>>8; int ll=(node0->data.flag)&0x0ff;
    
    union JsonData data;char data_type;key=StringNumber(key+1,&data,&data_type)+1;int n=data.dataS32;
    if(n>=num) return;
    struct ListNode *node = handle->layer[ll].node+idx+n;
    
    if(key[0]== 0 ) {LayerDelete(handle->layer+ll,idx,num,n);return;}
    if(key[0]=='.') {mException(node->type!=JSON_LIST ,EXIT,"invalid key");JSONListDelete( node,handle,key+1);return;}
    if(key[0]=='[') {mException(node->type!=JSON_ARRAY,EXIT,"invalid key");JSONArrayDelete(node,handle,key  );return;}
    mException(1,EXIT,"invalid key");
}

void m_JSONDelete0(MArray *json,int n)
{
    mException(INVALID_POINTER(json)||(n<0)||(n>=json->num),EXIT,"invalid input");
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)json,6);
    struct ListNode *node0=*(struct ListNode **)mReserve((MObject *)json,7);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    // mException((node0->type==JSON_ARRAY)!=(json->element_size==sizeof(struct ListNode)),EXIT,"invalid json");

    int idx=node0->data.idx;int num=(node0->data.flag)>>8; int ll=(node0->data.flag)&0x0ff;
    LayerDelete(handle->layer+ll,idx,num,n);
}

void m_JSONDelete1(MArray *json,char *key)
{
    mException(INVALID_POINTER(json)||INVALID_POINTER(key),EXIT,"invalid input");
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)json,6);
    struct ListNode *node0=*(struct ListNode **)mReserve((MObject *)json,7);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    mException((node0->type==JSON_ARRAY)!=(json->element_size==sizeof(struct ListNode)),EXIT,"invalid json");

    if(key[0]=='[') {mException(node0->type!=JSON_ARRAY,EXIT,"invalid key");JSONArrayDelete(node0,handle,key);}
    else            {mException(node0->type!=JSON_LIST ,EXIT,"invalid key");JSONListDelete(node0,handle,key);}
}
*/




