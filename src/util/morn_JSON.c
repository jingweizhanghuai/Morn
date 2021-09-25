/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"

#define fread(Data,Size,Num,Fl) mException(((int)fread(Data,Size,Num,Fl)!=Num),EXIT,"read file error")

char *morn_json_type[15]={"UNKNOWN","KEY_UNKNOWN","BOOL","KEY_BOOL","INT","KEY_INT","DOUBLE","KEY_DOUBLE","STRING","KEY_STRING","LIST","KEY_LIST","ARRAY","KEY_ARRAY","UNKNOWN"};

union JsonData
{
    char   dataBool;
    int    dataS32;
    double dataD64;
    char   *string;
    struct {uint16_t num;uint16_t layer;int32_t idx;};
    MArray *array;
    void *handle0;
};
struct JsonNode
{
    union JsonData data;
    char *key;
    char type;
    uint8_t li1;
    uint16_t li2;
};
struct JsonLayer
{
    struct JsonNode *node;
    int cap;
    int num;
};

struct JsonNode *LayerNode(struct JsonLayer *layer)
{
    int num = layer->num;
    if(num>=layer->cap)
    {
        int n=MAX(256,num+num);
        struct JsonNode *buff=mMalloc(n*sizeof(struct JsonNode));
        memcpy(buff,layer->node,num*sizeof(struct JsonNode));if(num>1) mFree(layer->node);
        layer->node=buff;layer->cap=n;
    }
    layer->num=num+1;
    return (layer->node+num);
}

int LayerAppend(struct JsonLayer *layer,int idx,int n1,int n2,struct JsonNode *node)
{
    int num0=layer->num;
    if(idx+n1==num0) {idx=idx+n1;n2=n2-n1;n1=0;}
    if(layer->cap-num0<n2)
    {
        int n=num0+MAX(num0,n2);
        struct JsonNode *buff=mMalloc(n*sizeof(struct JsonNode));
        memcpy(buff,layer->node,num0*sizeof(struct JsonNode));if(num0>1) mFree(layer->node);
        layer->node=buff;layer->cap=n;
    }
    if(n1>0) memcpy(layer->node+num0,layer->node+idx,n1*sizeof(struct JsonNode));
    int i;for(i=num0+n1;i<num0+n2-1;i++)
    {
        layer->node[i].data.string=NULL;
        layer->node[i].type=JSON_STRING;
    }
    if(node!=NULL) layer->node[i].key=node->key;
    layer->num=num0+n2;
    return num0;
}

// int LayerCopy(struct JsonLayer *src,int idx,int num,struct JsonLayer *dst)
// {
//     if(dst->cap-dst->num<num)
//     {
//         int n=dst->num+MAX(dst->num,num);
//         struct JsonNode *buff=mMalloc(n*sizeof(struct JsonNode));
//         memcpy(buff,layer->node,dst->num*sizeof(struct JsonNode));if(dst->num>1) mFree(layer->node);
//         layer->node=buff;layer->cap=n;
//     }
//     memcpy(dst->node+dst->num,src->node+idx,num*sizeof(struct JsonNode)
// }

void LayerDelete(struct JsonLayer *layer,int idx,int num,int del)
{
    memmove(layer->node+idx+del,layer->node+idx+del+1,(num-del-1)*sizeof(struct JsonNode));
}

struct HandleJSON
{
    struct JsonLayer layer[64];
    struct JsonNode node0[2];
    
    char *file;
    MMemory *memory;
};
#define HASH_JSON 0x1eb02389
void endJSON(struct HandleJSON *handle)
{
    for(int i=0;i<64;i++) {if(handle->layer[i].num>1) {mFree(handle->layer[i].node);}}
    if(handle->file  !=NULL) mFree(handle->file);
    if(handle->memory!=NULL) mMemoryRelease(handle->memory);
}

char *StringNumber(char *p,union JsonData *data,char *type)
{
    int flag=0;int v=0;
         if(p[0]=='-') {flag=1;p++;}
    else if(p[0]=='+')         p++;
    for(;(p[0]>='0')&&(p[0]<='9');p++) v=v*10+(p[0]-'0');
    if(p[0]=='.')
    {
        double d=(double)v;p++;
        for(double f=0.1;(p[0]>='0')&&(p[0]<='9');p++,f*=0.1) d=d+(p[0]-'0')*f;
        data->dataD64=(flag)?(0-d):d;*type=JSON_DOUBLE;return p;
    }
    if((p[0]=='e')||(p[0]=='E'))
    {
        union JsonData e;char e_type;
        p=StringNumber(p+1,&e,&e_type);
        double d=v*pow(10.0,e.dataD64);
        data->dataD64=(flag)?(0-d):d;*type=JSON_DOUBLE;return p;
    }
    data->dataS32=(flag)?(0-v):v;*type=JSON_INT;return p;
}

char *JSONString(char *p)
{
    int i,j;
    for(i=0,j=0;p[i]!='"';i++,j++)
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
    struct JsonLayer *layer = handle->layer+l;
    char *p=*file;int flag=0;
    struct JsonNode *node=LayerNode(layer);
    
    while(1)
    {
        p++;
        if((p[0]==' ')||(p[0]=='\t')||(p[0]=='\n')||(p[0]=='\r')) continue;
        else if(p[0]=='"')
        {
            p=p+1;
            if(flag==0) {node->key=p;{for(;p[0]!='"';p++);}p[0]=0;node->key[-1]=p-node->key;}//printf("l=%d,key=%s\n",l,node->key);}
            else        {node->data.string=p;node->type=JSON_STRING;p=JSONString(p);}
        }
        else if(p[0]==':') {mException(flag==1,EXIT,"json file error");flag=1;}
        else if(p[0]==',') {node=LayerNode(layer);flag=0;node->data.string=NULL;}
        else if(p[0]=='}') {p[0]=0; *file=p;return;}
        else if(p[0]=='{') 
        {
            int n=(layer+1)->num;
            JSONListLoad( &p,handle,l+1);
            node->data.idx  =n;
            node->data.num  =((layer+1)->num-n);
            node->data.layer=l+1;
            node->type=JSON_LIST;
            int layer_num = layer->num-1;
            node->li1=layer_num>>16;node->li2=layer_num&0x0ffff; 
        }
        else if(p[0]=='[') 
        {
            int n=(layer+1)->num;
            JSONArrayLoad(&p,handle,l+1);
            node->data.idx  =n;
            node->data.num  =((layer+1)->num-n);
            node->data.layer=l+1;
            node->type=JSON_ARRAY;
            int layer_num = layer->num-1;
            node->li1=layer_num>>16;node->li2=layer_num&0x0ffff;
        }
        else if(p[0]=='t') {node->type=JSON_BOOL  ;node->data.dataBool = 1;p+=3;}
        else if(p[0]=='f') {node->type=JSON_BOOL  ;node->data.dataBool = 0;p+=4;}
        else if(p[0]=='n') {node->type=JSON_STRING;node->data.string=NULL;p+=3;}
        else                p=StringNumber(p,&(node->data),&(node->type))-1;
    }
}

void JSONArrayLoad(char **file,struct HandleJSON *handle,int l)
{
    struct JsonLayer *layer = handle->layer+l;
    char *p=*file;
    
    while(1)
    {
        p++;
        if((p[0]==' ')||(p[0]=='\t')||(p[0]=='\n')||(p[0]=='\r')||(p[0]==',')) continue;
        else if(p[0]==']') {*file=p;return;}

        struct JsonNode *node=LayerNode(layer);node->key=NULL;
        if(p[0]=='{')
        {
            int n=(layer+1)->num;
            JSONListLoad( &p,handle,l+1);
            node->data.idx  =n;
            node->data.num  =((layer+1)->num-n);
            node->data.layer=l+1;
            node->type=JSON_LIST;
            int layer_num = layer->num-1;
            node->li1=layer_num>>16;node->li2=layer_num&0x0ffff;
        }
        else if(p[0]=='[') 
        {
            int n=(layer+1)->num;
            JSONArrayLoad(&p,handle,l+1);
            node->data.idx  =n;
            node->data.num  =((layer+1)->num-n);
            node->data.layer=l+1;
            node->type=JSON_ARRAY;
            int layer_num = layer->num-1;
            node->li1=layer_num>>16;node->li2=layer_num&0x0ffff;
        }
        else if(p[0]=='"') {node->type=JSON_STRING; p=p+1;node->data.string=p;p=JSONString(p);}
        else if(p[0]=='t') {node->type=JSON_BOOL  ;node->data.dataBool = 1;p+=3;}
        else if(p[0]=='f') {node->type=JSON_BOOL  ;node->data.dataBool = 0;p+=4;}
        else if(p[0]=='n') {node->type=JSON_STRING;node->data.string=NULL;p+=3;}
        else                p=StringNumber(p,&(node->data),&(node->type))-1;
    }
}

struct JSONNode *mJSONParse(MObject *jsondata)
{
    if(jsondata->size<0) jsondata->size=strlen(jsondata->string);
    
    MHandle *hdl = mHandle(jsondata,JSON);
    struct HandleJSON *handle=hdl->handle;
    if(hdl->valid==0)
    {
        handle->node0[0].data.handle0 = handle;
        for(int i=0;i<64;i++) 
        {
            handle->layer[i].node = handle->node0;
            handle->layer[i].num=1;handle->layer[i].cap=1;
        }
        
        hdl->valid=1;
    }

    char *string = jsondata->string;
    mObjectRedefine(jsondata,NULL,jsondata->size);
    if(string!=jsondata->string) memcpy(jsondata->string,string,jsondata->size);

    for(char *p=jsondata->string;;p++)
    {
        if(p[0]=='{') {handle->node0[1].type=JSON_LIST ; JSONListLoad( &p,handle,1);break;}
        if(p[0]=='[') {handle->node0[1].type=JSON_ARRAY; JSONArrayLoad(&p,handle,1);break;}
    }
    int num=handle->layer[1].num;
    // json->num=num;json->element_size=sizeof(struct JsonNode);json->data=handle->layer[0].node;
    handle->node0[1].data.idx  =1;
    handle->node0[1].data.num  =num;
    handle->node0[1].data.layer=1;
    handle->node0[1].li1=0;handle->node0[1].li2=1;
    
    // *(struct JsonNode **)mReserve((MObject *)json,7)=&(handle->node0[1]);
    return (struct JSONNode *)(&(handle->node0[1]));
}

struct JSONNode *mJSONLoad(MFile *jsonfile)
{
    mException(INVALID_POINTER(jsonfile),EXIT,"invalid input");
    
    MHandle *hdl = mHandle(jsonfile,JSON);
    
    struct HandleJSON *handle=hdl->handle;
    if(hdl->valid==0)
    {
        handle->node0[0].data.handle0 = handle;
        for(int i=0;i<64;i++) 
        {
            handle->layer[i].node = handle->node0;
            handle->layer[i].num=1;handle->layer[i].cap=1;
        }
        
        hdl->valid=1;
    }

    FILE *f=fopen(jsonfile->filename,"rb");
    int size = fsize(f);
    if(handle->file!=NULL) mFree(handle->file);
    handle->file=(char *)mMalloc(size);
    fread(handle->file,size,1,f);
    fclose(f);

    for(char *p=handle->file;;p++)
    {
        if(p[0]=='{') {handle->node0[1].type=JSON_LIST ; JSONListLoad( &p,handle,1);break;}
        if(p[0]=='[') {handle->node0[1].type=JSON_ARRAY; JSONArrayLoad(&p,handle,1);break;}
    }
    int num=handle->layer[1].num;
    // json->num=num;json->element_size=sizeof(struct JsonNode);json->data=handle->layer[0].node;
    handle->node0[1].data.idx  =1;
    handle->node0[1].data.num  =num;
    handle->node0[1].data.layer=1;
    handle->node0[1].li1=0;handle->node0[1].li2=1;
    
    // *(struct JsonNode **)mReserve((MObject *)json,7)=&(handle->node0[1]);
    return (struct JSONNode *)(&(handle->node0[1]));
}

struct JsonNode *JSONArrayRead(struct JsonNode *node0,char *key);
struct JsonNode *JSONListRead( struct JsonNode *node0,char *key)
{
    // int lll=0-(((int)(node0->li1))<<16)-node0->li2;printf("lll=%d\n",lll);
    struct HandleJSON *handle=node0[0-(((int)(node0->li1))<<16)-node0->li2].data.handle0;
    int idx=node0->data.idx;int num=node0->data.num; int li=node0->data.layer;

    struct JsonNode *node = handle->layer[li].node+idx;
    int i;for(i=0;i<num;i++)
    {
        int s=node->key[-1];
        if(memcmp(key,node->key,s)==0) {if((key[s]==0)||(key[s]=='[')||(key[s]=='.')) {key+=s;break;}}
        node++;
    }
    if(i==num) return NULL;
    if(key[0]== 0 ) return node;
    if(key[0]=='.') {mException(node->type!=JSON_LIST ,EXIT,"invalid key");return JSONListRead( node,key+1);}
    if(key[0]=='[') {mException(node->type!=JSON_ARRAY,EXIT,"invalid key");return JSONArrayRead(node,key  );}
    mException(1,EXIT,"invalid key");return NULL;
}

struct JsonNode *JSONArrayRead(struct JsonNode *node0,char *key)
{
    struct HandleJSON *handle=node0[0-(((int)(node0->li1))<<16)-node0->li2].data.handle0;
    int idx=node0->data.idx;int num=node0->data.num; int li=node0->data.layer;
    
    union JsonData data;char data_type;key=StringNumber(key+1,&data,&data_type)+1;int n=data.dataS32;
    if(n>=num) return NULL;
    struct JsonNode *node = handle->layer[li].node+idx+n;
    
    if(key[0]== 0 ) return node;
    if(key[0]=='.') {mException(node->type!=JSON_LIST ,EXIT,"invalid key");return JSONListRead( node,key+1);}
    if(key[0]=='[') {mException(node->type!=JSON_ARRAY,EXIT,"invalid key");return JSONArrayRead(node,key  );}
    mException(1,EXIT,"invalid key");return NULL;
}

// struct JSONNode *m_JSONRead(struct JSONNode *node_0,char *key,struct JSONNode *data)
// {
//     struct JsonNode *node0=(struct JsonNode *)node_0;
//     mException(INVALID_POINTER(node0)||INVALID_POINTER(key),EXIT,"invalid input");
    
//     struct JsonNode *node=NULL;
//     if(key[0]=='[') {mException(node0->type!=JSON_ARRAY,EXIT,"invalid key");node=JSONArrayRead(node0,key);}
//     else            {mException(node0->type!=JSON_LIST ,EXIT,"invalid key");node= JSONListRead(node0,key);}
//     if(node==NULL) return NULL;
//     if(data!=NULL) *(struct JsonNode *)data=*node;
//     return (struct JSONNode *)node;
// }

struct JSONNode *m_JSONRead(struct JSONNode *n0,intptr_t v)
{
    if(n0->type<JSON_LIST) return NULL;
    struct JsonNode *node0=(struct JsonNode *)n0;
    struct HandleJSON *handle=node0[0-(((int)(node0->li1))<<16)-node0->li2].data.handle0;
    int idx   = node0->data.idx;
    int number= node0->data.num;
    int li    = node0->data.layer;
    
    if(v<number) return (struct JSONNode *)(handle->layer[li].node+idx+v);

    char *key = (char *)v;
    struct JsonNode *node=NULL;
    
    if(key[0]=='[') {mException(node0->type!=JSON_ARRAY,EXIT,"invalid key");node=JSONArrayRead(node0,key);}
    else            {mException(node0->type!=JSON_LIST ,EXIT,"invalid key");node= JSONListRead(node0,key);}
    if(node==NULL) return NULL;
    // if(data!=NULL) *(struct JsonNode *)data=*node;
    return (struct JSONNode *)node;
}

void mJSONArray(MArray *array,struct JSONNode *node)
{
    struct JsonNode *node0=(struct JsonNode *)node;
    struct HandleJSON *handle=node0[0-(((int)(node0->li1))<<16)-node0->li2].data.handle0;
    int idx=node0->data.idx;int num=node0->data.num; int li=node0->data.layer;
    array->num=num;array->element_size=sizeof(struct JsonNode);array->data=handle->layer[li].node+idx;
    *(struct JsonNode **)mReserve((MObject *)array,7)=node0;
}

struct JsonNode *JSONArrayWrite(struct JsonNode *node0,char *key);
struct JsonNode *JSONListWrite( struct JsonNode *node0,char *key)
{
    struct HandleJSON *handle=node0[0-(((int)(node0->li1))<<16)-node0->li2].data.handle0;
    int idx=node0->data.idx;int num=node0->data.num; int li=node0->data.layer;
    
    struct JsonNode *node = handle->layer[li].node+idx;
    int n;for(n=0;n<num;n++)
    {
        int s=node->key[-1];
        if(memcmp(key,node->key,s)==0) {if((key[s]==0)||(key[s]=='[')||(key[s]=='.')) {key+=s;break;}}
        node++;
    }
    if(n==num)
    {
        if(handle->memory==NULL) handle->memory=mMemoryCreate(DFLT,DFLT,MORN_HOST);
        
        struct JsonNode nd;
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
        struct JsonNode *pnode= JSONListWrite( node,key+1);
        node->type=JSON_LIST;node->data.num=MAX(num,n+1);node->data.layer=li+1;node->data.idx=pnode-handle->layer[li].node;
        return pnode;
    }
    if(key[0]=='[') 
    {
        mException(node->type!=JSON_ARRAY,EXIT,"invalid key");
        struct JsonNode *pnode= JSONArrayWrite(node,key  );
        node->type=JSON_ARRAY;node->data.num=MAX(num,n+1);node->data.layer=li+1;node->data.idx=pnode-handle->layer[li].node;
        return pnode;
    }
    mException(1,EXIT,"invalid key");return NULL;
}

struct JsonNode *JSONArrayWrite(struct JsonNode *node0,char *key)
{
    struct HandleJSON *handle=node0[0-(((int)(node0->li1))<<16)-node0->li2].data.handle0;
    int idx=node0->data.idx;int num=node0->data.num; int li=node0->data.layer;

    union JsonData data;char data_type;key=StringNumber(key+1,&data,&data_type)+1;int n=data.dataS32;
    if(n>=num)
    {
        node0->data.idx  =LayerAppend(handle->layer+li,idx,num,n,NULL);
        node0->data.num  =n;
        node0->data.layer=li;
    }
    struct JsonNode *node = handle->layer[li].node+node0->data.idx+n;
    
    if(key[0]== 0 ) return node;
    if(key[0]=='.') 
    {
        mException(node->type!=JSON_LIST ,EXIT,"invalid key");
        struct JsonNode *pnode= JSONListWrite( node,key+1);
        node->type=JSON_LIST;node->data.num=MAX(num,n+1);node->data.layer=li+1;node->data.idx=pnode-handle->layer[li].node;
        return pnode;
    }
    else if(key[0]=='[') 
    {
        mException(node->type!=JSON_ARRAY,EXIT,"invalid key");
        struct JsonNode *pnode= JSONArrayWrite(node,key  );
        node->type=JSON_ARRAY;node->data.num=MAX(num,n+1);node->data.layer=li+1;node->data.idx=pnode-handle->layer[li].node;
        return pnode;
    }
    else mException(1,EXIT,"invalid key");
    return NULL;
}
/*
void JSONCopy(struct JsonNode *src_node,struct JsonNode *dst_node);
{
    struct HandleJSON *handle;
    handle=src_node[0-(((int)(src_node->li1))<<16)-src_node->li2].data.handle0;
    int src_idx=src_node->data.idx;int src_num=(src_node->data.flag)>>8; int src_li=(src_node->data.flag)&0x0ff;
    struct JsonLayer *src_layer = handle->layer[src_li];



    handle=dst_node[0-(((int)(dst_node->li1))<<16)-dst_node->li2].data.handle0;
    int dst_idx=dst_node->data.idx;int dst_num=(dst_node->data.flag)>>8; int dst_li=(dst_node->data.flag)&0x0ff;
    dst
    

    if(dst->cap-dst->num<src_num)
    
    // if(dst->cap-dst->num<num)
//     {
//         int n=dst->num+MAX(dst->num,num);
//         struct JsonNode *buff=mMalloc(n*sizeof(struct JsonNode));
//         memcpy(buff,layer->node,dst->num*sizeof(struct JsonNode));if(dst->num>1) mFree(layer->node);
//         layer->node=buff;layer->cap=n;
//     }
    
}
*/
struct JSONNode *m_JSONWrite(struct JSONNode *node_0,char *key,struct JSONNode *data)
{
    struct JsonNode *node0=(struct JsonNode *)node_0;
    mException(INVALID_POINTER(node0)||INVALID_POINTER(key),EXIT,"invalid input");
    
    struct JsonNode *node=NULL;
    if(key[0]=='[') {mException(node0->type!=JSON_ARRAY,EXIT,"invalid key");node=JSONArrayRead(node0,key);}
    else            {mException(node0->type!=JSON_LIST ,EXIT,"invalid key");node= JSONListRead(node0,key);}
    
    if(node==NULL) return NULL;
    if(data!=NULL) *node=*(struct JsonNode *)data;
    return (struct JSONNode *)node;
}


/*
void m_JSONArray0(MArray *in,MArray *out,int n)
{
    mException(INVALID_POINTER(in),EXIT,"invalid input");
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)in,6);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    *(struct HandleJSON **)mReserve((MObject *)out,6) = handle;
    
    struct JsonNode *node = (struct JsonNode *)(in->dataS8+sizeof(struct JsonNode)*n);
    mException((node->type<JSON_LIST),EXIT,"invalid input");
    *(struct JsonNode **)mReserve((MObject *)out,7) = node;
    int idx=node->data.idx;int num=(node->data.flag)>>8; int layer=(node->data.flag)&0x0ff;
    out->num=num;out->element_size=sizeof(struct JsonNode);out->data=handle->layer[layer].node+idx;
}

void m_JSONArray1(MArray *in,MArray *out,char *key)
{
    mException(INVALID_POINTER(in),EXIT,"invalid input");
    if(key==NULL) {out->num=in->num;out->element_size=in->element_size;out->data=in->data;return;}
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)in,6);
    struct JsonNode *node0=*(struct JsonNode **)mReserve((MObject *)in,7);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    *(struct HandleJSON **)mReserve((MObject *)out,6) = handle;

    struct JsonNode *node =NULL;
    if(key[0]=='[') {mException(node0->type!=JSON_ARRAY,EXIT,"invalid key");node=JSONArrayRead(node0,handle,key);}
    else            {mException(node0->type!=JSON_LIST ,EXIT,"invalid key");node= JSONListRead(node0,handle,key);}
    mException(node==NULL,EXIT,"invalid key");
    *(struct JsonNode **)mReserve((MObject *)out,7) = node;
    int idx=node->data.idx;int num=(node->data.flag)>>8; int layer=(node->data.flag)&0x0ff;
    out->num=num;out->element_size=sizeof(struct JsonNode);out->data=handle->layer[layer].node+idx;
}

struct JsonNode *JSONArrayWrite(struct JsonNode *node0,struct HandleJSON *handle,char *key);
struct JsonNode *JSONListWrite( struct JsonNode *node0,struct HandleJSON *handle,char *key)
{
    int idx=node0->data.idx;int num=(node0->data.flag)>>8; int ll=(node0->data.flag)&0x0ff;
    
    struct JsonNode *node = handle->layer[ll].node+idx;
    int n;for(n=0;n<num;n++)
    {
        int s=node->ksize;
        if(memcmp(key,node->key,s)==0) {if((key[s]==0)||(key[s]=='[')||(key[s]=='.')) {key+=s;break;}}
        node++;
    }
    if(n==num)
    {
        if(handle->memory==NULL) handle->memory=mMemoryCreate(DFLT,DFLT,MORN_HOST);
        
        struct JsonNode nd;
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
        struct JsonNode *pnode= JSONListWrite( node,handle,key+1);
        node->type=JSON_LIST;node->data.flag=(MAX(num,n+1)<<8)+ll+1;node->data.idx=pnode-handle->layer[ll].node;
        return pnode;
    }
    if(key[0]=='[') 
    {
        mException(node->type!=JSON_ARRAY,EXIT,"invalid key");
        struct JsonNode *pnode= JSONArrayWrite(node,handle,key  );
        node->type=JSON_ARRAY;node->data.flag=(MAX(num,n+1)<<8)+ll+1;node->data.idx=pnode-handle->layer[ll].node;
        return pnode;
    }
    mException(1,EXIT,"invalid key");return NULL;
}

struct JsonNode *JSONArrayWrite(struct JsonNode *node0,struct HandleJSON *handle,char *key)
{
    int idx=node0->data.idx;int num=(node0->data.flag)>>8; int ll=(node0->data.flag)&0x0ff;

    union JsonData data;char data_type;key=StringNumber(key+1,&data,&data_type)+1;int n=data.dataS32;
    if(n>=num)
    {
        node0->data.idx =LayerAppend(handle->layer+ll,idx,num,n,NULL);
        node0->data.flag=((n+1)<<8)+ll;
    }
    struct JsonNode *node = handle->layer[ll].node+node0->data.idx+n;
    
    if(key[0]== 0 ) return node;
    if(key[0]=='.') 
    {
        mException(node->type!=JSON_LIST ,EXIT,"invalid key");
        struct JsonNode *pnode= JSONListWrite( node,handle,key+1);
        node->type=JSON_LIST;node->data.flag=(MAX(num,n+1)<<8)+ll+1;node->data.idx=pnode-handle->layer[ll].node;
        return pnode;
    }
    else if(key[0]=='[') 
    {
        mException(node->type!=JSON_ARRAY,EXIT,"invalid key");
        struct JsonNode *pnode= JSONArrayWrite(node,handle,key  );
        node->type=JSON_ARRAY;node->data.flag=(MAX(num,n+1)<<8)+ll+1;node->data.idx=pnode-handle->layer[ll].node;
        return pnode;
    }
    else mException(1,EXIT,"invalid key");
    return NULL;
}

struct JsonNode *m_JSONWrite0(MArray *json,int n,struct JsonNode *data)
{
    // if(data->type<0) data->type=JSON_STRING;
    mException(INVALID_POINTER(json),EXIT,"invalid input");
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)json,6);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    
    struct JsonNode *node=NULL;
    if(n<0) n=json->num;
    if(n>=json->num)
    {
        struct JsonNode *node0=*(struct JsonNode **)mReserve((MObject *)json,7);
        mException((node0->type!=JSON_ARRAY),EXIT,"invalid json");
        int idx=node0->data.idx;int ll=(node0->data.flag)&0x0ff;
        node0->data.idx=LayerAppend(handle->layer+ll,idx,json->num,n);
        node0->data.flag =((n+1)<<8)+ll;
    }
    else node=(struct JsonNode *)(json->dataS8+json->element_size*n);
    
    node->type=type;
    if(type==JSON_BOOL  ) {node->data.dataBool=*(char   *)data;return &(node->data.dataBool);}
    if(type==JSON_INT   ) {node->data.dataS32 =*(int    *)data;return &(node->data.dataS32);}
    if(type==JSON_DOUBLE) {node->data.dataD64 =*(double *)data;return &(node->data.dataD64);}
    if(type==JSON_STRING) {if(handle->memory==NULL){handle->memory=mMemoryCreate(DFLT,DFLT,MORN_HOST);} node->data.string =mMemoryWrite(handle->memory,data,DFLT);return node->data.string;}
    MArray *a=(MArray *)data;
    struct JsonNode *pnode=*(struct JsonNode **)mReserve((MObject *)a,7);
    mException((pnode->type==JSON_ARRAY)!=(a->element_size==sizeof(struct JsonNode)),EXIT,"invalid json");
    node->data=pnode->data;
    return data;
}

void *m_JSONWrite1(MArray *json,char *key,void *data,int type)
{
    if(type<0) type=JSON_STRING;
    mException(INVALID_POINTER(json)||INVALID_POINTER(key),EXIT,"invalid input");
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)json,6);
    struct JsonNode *node0=*(struct JsonNode **)mReserve((MObject *)json,7);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    // mException((node0->type==JSON_ARRAY)!=(json->element_size==sizeof(struct JsonNode)),EXIT,"invalid json");

    struct JsonNode *node=NULL;
    if(key[0]=='[') {mException(node0->type!=JSON_ARRAY,EXIT,"invalid key");node=JSONArrayWrite(node0,handle,key);}
    else            {mException(node0->type!=JSON_LIST ,EXIT,"invalid key");node= JSONListWrite(node0,handle,key);}
    
    node->type=type;
    if(type==JSON_BOOL  ) {node->data.dataBool=*(char   *)data;return &(node->data.dataBool);}
    if(type==JSON_INT   ) {node->data.dataS32=*(int    *)data;return &(node->data.dataS32);}
    if(type==JSON_DOUBLE) {node->data.dataD64=*(double *)data;return &(node->data.dataD64);}
    if(type==JSON_STRING) {if(handle->memory==NULL){handle->memory=mMemoryCreate(DFLT,DFLT,MORN_HOST);} node->data.string =mMemoryWrite(handle->memory,data,DFLT);return node->data.string;}
    MArray *a=(MArray *)data;
    struct JsonNode *pnode=*(struct JsonNode **)mReserve((MObject *)a,7);
    mException((pnode->type==JSON_ARRAY)!=(a->element_size==sizeof(struct JsonNode)),EXIT,"invalid json");
    node->data=pnode->data;
    return data;
}

void JSONArrayDelete(void *pnode,struct HandleJSON *handle,char *key);
void JSONListDelete(void *pnode,struct HandleJSON *handle,char *key)
{
    struct JsonNode *node0 = pnode;
    int idx=node0->data.idx;int num=(node0->data.flag)>>8; int ll=(node0->data.flag)&0x0ff;
    
    struct JsonNode *node = handle->layer[ll].node+idx;
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
    struct JsonNode *node0 = pnode;
    int idx=node0->data.idx;int num=(node0->data.flag)>>8; int ll=(node0->data.flag)&0x0ff;
    
    union JsonData data;char data_type;key=StringNumber(key+1,&data,&data_type)+1;int n=data.dataS32;
    if(n>=num) return;
    struct JsonNode *node = handle->layer[ll].node+idx+n;
    
    if(key[0]== 0 ) {LayerDelete(handle->layer+ll,idx,num,n);return;}
    if(key[0]=='.') {mException(node->type!=JSON_LIST ,EXIT,"invalid key");JSONListDelete( node,handle,key+1);return;}
    if(key[0]=='[') {mException(node->type!=JSON_ARRAY,EXIT,"invalid key");JSONArrayDelete(node,handle,key  );return;}
    mException(1,EXIT,"invalid key");
}

void m_JSONDelete0(MArray *json,int n)
{
    mException(INVALID_POINTER(json)||(n<0)||(n>=json->num),EXIT,"invalid input");
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)json,6);
    struct JsonNode *node0=*(struct JsonNode **)mReserve((MObject *)json,7);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    // mException((node0->type==JSON_ARRAY)!=(json->element_size==sizeof(struct JsonNode)),EXIT,"invalid json");

    int idx=node0->data.idx;int num=(node0->data.flag)>>8; int ll=(node0->data.flag)&0x0ff;
    LayerDelete(handle->layer+ll,idx,num,n);
}

void m_JSONDelete1(MArray *json,char *key)
{
    mException(INVALID_POINTER(json)||INVALID_POINTER(key),EXIT,"invalid input");
    
    struct HandleJSON *handle=*(struct HandleJSON **)mReserve((MObject *)json,6);
    struct JsonNode *node0=*(struct JsonNode **)mReserve((MObject *)json,7);
    mException(handle->json_flag!=HASH_JSON,EXIT,"invalid json");
    mException((node0->type==JSON_ARRAY)!=(json->element_size==sizeof(struct JsonNode)),EXIT,"invalid json");

    if(key[0]=='[') {mException(node0->type!=JSON_ARRAY,EXIT,"invalid key");JSONArrayDelete(node0,handle,key);}
    else            {mException(node0->type!=JSON_LIST ,EXIT,"invalid key");JSONListDelete(node0,handle,key);}
}
*/




