/*
Copyright (C) 2019-2024 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_math.h"

struct GraphNode
{
    void *data;
    struct GraphNode **node;
    float *length;
    int node_num;
    
    int node_cap;
    int ID;
    MGraph *graph;
};

struct HandleGraphCreate
{
    struct GraphNode **node;
    int node_num;
    int node_cap;
    MMemory *memory;
    int edition;

    float (*linkloss)(struct GraphNode *,struct GraphNode *,struct GraphNode *,void *);
    void *linkloss_para;
};
void endGraphCreate(struct HandleGraphCreate *handle)
{
    for(int i=0;i<handle->node_num;i++)
        mFree(handle->node[i]->node);
    if(handle->memory != NULL) mMemoryRelease(handle->memory);
    if(handle->node   != NULL) mFree(handle->node);
}
#define HASH_GraphCreate 0xde7d24f1
MGraph *mGraphCreate()
{
    MGraph *graph = mObjectCreate();
    mObjectType(graph)=HASH_GraphCreate;
    MHandle *hdl = mHandle(graph,GraphCreate);hdl->valid = 1;
    struct HandleGraphCreate *handle = hdl->handle;

    mPropertyVariate(graph,"linkloss"     ,&(handle->linkloss)     ,sizeof(void *));
    mPropertyVariate(graph,"linkloss_para",&(handle->linkloss_para),sizeof(void *));
    
    return graph;
}

void mGraphRelease(MGraph *graph)
{
    mObjectRelease(graph);
}

MGraphNode *m_GraphNode(MGraph *graph,void *data,int size)
{
    MHandle *hdl = ObjHandle(graph,1);
    mException((hdl->flag != HASH_GraphCreate),EXIT,"invalid input graph");
    struct HandleGraphCreate *handle =(struct HandleGraphCreate *)(hdl->handle);
    
    if(handle->memory == NULL) handle->memory = mMemoryCreate(DFLT,DFLT,MORN_HOST);
    if(handle->node_cap == handle->node_num)
    {
        handle->node_cap = handle->node_cap+MAX(handle->node_cap/2,8);
        struct GraphNode **buff = (struct GraphNode **)mMalloc(handle->node_cap*sizeof(struct GraphNode *));
        if(handle->node_num>0) 
        {
            memcpy(buff,handle->node,handle->node_num*sizeof(struct GraphNode *));
            mFree(handle->node);
        }
        handle->node = buff;
    }

    if(size<=0) size =(data!=NULL)?(strlen(data)+1):0;
    if(data!=NULL)
    {
        for(int i=0;i<handle->node_num;i++)
        {
            struct GraphNode *p=handle->node[i];
            if(memcmp(p->data,data,size)==0) return (MGraphNode *)p;
        }
    }
    
    struct GraphNode *node = mMemoryWrite(handle->memory,NULL,sizeof(struct GraphNode)+size);
    handle->node[handle->node_num] = node;
    memset(node,0,sizeof(struct GraphNode));
    node->data = (void *)(node+1); 
    node->ID = handle->node_num;
    node->graph = graph;
    handle->node_num+=1;
    if(graph->graphnode==NULL) graph->graphnode = (MGraphNode *)node;

    if(data!=NULL) memcpy(node->data,data,size);
    handle->edition++;
    return (MGraphNode *)node;
}

void mGraphLink(MGraphNode *node0,MGraphNode *node,float length)
{
    struct GraphNode *graph = (struct GraphNode *)node0;
    mException(graph->graph!=((struct GraphNode *)node)->graph,EXIT,"invalid input graph node");
    
    MHandle *hdl = ObjHandle(graph->graph,1);
    struct HandleGraphCreate *handle =(struct HandleGraphCreate *)(hdl->handle);
    handle->edition++;
    
    for(int i=0;i<graph->node_num;i++)
    {
        if(graph->node[i]==(struct GraphNode *)node){graph->length[i] = length;return;}
    }
    
    if(graph->node_cap == graph->node_num)
    {
        graph->node_cap = graph->node_cap + MAX(graph->node_cap/2,2);
        struct GraphNode **nbuff = (struct GraphNode **)mMalloc(graph->node_cap*(sizeof(struct GraphNode *)+sizeof(float)));
        float *wbuff = (float *)(nbuff+graph->node_cap);
        if(graph->node_num > 0)
        {
            memcpy(nbuff,graph->node  ,graph->node_num*sizeof(struct GraphNode *));
            memcpy(wbuff,graph->length,graph->node_num*sizeof(float));
            mFree(graph->node);
        }
        graph->node  =nbuff;
        graph->length=wbuff;
    }
    graph->  node[graph->node_num] = (struct GraphNode *)node;
    graph->length[graph->node_num] = length;
    graph->node_num+=1;
}

void mGraphDelink(MGraphNode *node0,MGraphNode *node)
{
    struct GraphNode *graph = (struct GraphNode *)node0;
    mException(graph->graph!=((struct GraphNode *)node)->graph,EXIT,"invalid input graph node");
    
    int num=graph->node_num;
    for(int i=0;i<num;i++)
    {
        if(graph->node[i]==(struct GraphNode *)node)
        {
            memmove(graph->node  +i,graph->node  +i+1,(num-i-1)*sizeof(struct GraphNode **));
            memmove(graph->length+i,graph->length+i+1,(num-i-1)*sizeof(float              ));
            graph->node_num=num-1;
            break;
        }
    }
    
    if(num!=graph->node_num)
    {
        MHandle *hdl = ObjHandle(graph->graph,1);
        struct HandleGraphCreate *handle =(struct HandleGraphCreate *)(hdl->handle);
        handle->edition++;
    }
}

struct HandleGraphTraversal
{
    MArray *array;
    int *flag;
    struct GraphNode **node;
    int node_num;
};
#define HASH_GraphTraversal 0xcb46155
void endGraphTraversal(struct HandleGraphTraversal *handle)
{
    if(handle->array!=NULL) mArrayRelease(handle->array);
    if(handle->flag !=NULL)         mFree(handle->flag);
}
void *GraphDFTraversal(struct HandleGraphTraversal *handle,struct GraphNode *node,int (*func)(struct GraphNode *,void *),void *para)
{
    handle->flag[node->ID]=1;
    if(func(node,para)) return node;
    for(int i=0;i<node->node_num;i++)
    {
        struct GraphNode *p=node->node[i];
        if(handle->flag[p->ID]==1) continue;
        void *rst=GraphDFTraversal(handle,p,func,para);
        if(rst) return rst;
    }
    return NULL;
}

void *GraphBFTraversal(struct HandleGraphTraversal *handle,struct GraphNode *node,int (*func)(struct GraphNode *,void *),void *para)
{
    if(handle->array==NULL) handle->array=mArrayCreate(sizeof(int));
    mArrayClear(handle->array);
    mArrayWrite(handle->array,&(node->ID));
    for(int n=0;n<handle->array->num;n++)
    {
        node=handle->node[handle->array->dataS32[n]];
        handle->flag[node->ID]=1;
        int rst=func(node,para);
        if(rst) return node;
        for(int i=0;i<node->node_num;i++)
        {
            struct GraphNode *p=node->node[i];
            if(handle->flag[p->ID]==1) continue;
            mArrayWrite(handle->array,&(p->ID));
        }
    }
    return NULL;
}

MGraph *m_GraphTraversal(MGraph *graph,int mode,void *function,void *para)
{
    int (*func)(struct GraphNode *,void *) = function;
    
    MHandle *hdl = ObjHandle(graph,1);
    mException((hdl->flag != HASH_GraphCreate),EXIT,"invalid input graph");
    struct HandleGraphCreate *handle0 =(struct HandleGraphCreate *)(hdl->handle);
    if(mode<0)
    {
        for(int i=0;i<handle0->node_num;i++) {if(func(handle0->node[i],para)) return (MGraph *)(handle0->node[i]);}
        return NULL;
    }
    
    hdl=mHandle(graph,GraphTraversal);
    struct HandleGraphTraversal *handle=hdl->handle;
    if(handle->node_num==handle0->node_num)
    {
        handle->node    =handle0->node;
        handle->node_num=handle0->node_num;
        if(handle->flag!=NULL) mFree(handle->flag);
        handle->flag=mMalloc(handle->node_num*sizeof(int));
        hdl->valid=1;
    }
    memset(handle->flag,0,handle->node_num*sizeof(int));
    
    struct GraphNode *node=(struct GraphNode *)(graph->graphnode);
    if(mode ==MORN_BREADTH_FIRST) return GraphBFTraversal(handle,node,func,para);
    else/*mode==MORN_DEPTH_FIRST*/return GraphDFTraversal(handle,node,func,para);
}

struct HandleGraphWay
{
    MMatrix *mat;
    MTable *tab;
    int graph_edition;
    void *linkloss;
};
void endGraphWay(struct HandleGraphWay *handle)
{
    if(handle->mat!=NULL) mMatrixRelease(handle->mat);
    if(handle->tab!=NULL) mTableRelease( handle->tab);
}
#define HASH_GraphWay 0xe1f142fc

void GraphWay(MGraph *graph,void *link_loss,void *para)
{
    float (*linkloss)(struct GraphNode *,struct GraphNode *,struct GraphNode *,void *) = link_loss;
    
    MHandle *hdl = ObjHandle(graph,1);
    mException((hdl->flag != HASH_GraphCreate),EXIT,"invalid input graph");
    struct HandleGraphCreate *handle0 =(struct HandleGraphCreate *)(hdl->handle);
    int node_num = handle0->node_num;
    struct GraphNode **node = handle0->node;
//     printf("node_num=%d\n",node_num);

    if(linkloss == NULL) linkloss = handle0->linkloss;
    if(para     == NULL) para     = handle0->linkloss_para;
    
    hdl = mHandle(graph,GraphWay);
    struct HandleGraphWay *handle = (struct HandleGraphWay *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->mat==NULL) handle->mat = mMatrixCreate(node_num,node_num);
        else                  mMatrixRedefine(handle->mat,node_num,node_num);
        if(handle->tab==NULL) handle->tab = mTableCreate(node_num,node_num,sizeof(int));
        else                  mTableRedefine(handle->tab,node_num,node_num,sizeof(int));
        for(int j=0;j<node_num;j++)for(int i=0;i<node_num;i++)
        {
            handle->mat->data[j][i]=(i==j)?0:mSup();
            handle->tab->dataS32[j][i]=DFLT;
        }
        hdl->valid = 1;
    }
    float **mat = handle->mat->data;
    int   **tab = handle->tab->dataS32;

    for(int j=0;j<node_num;j++)
    {
//         printf("j=%d,node[j].data=%s,node[j].node_num=%d\n",j,node[j]->data,node[j]->node_num);
        for(int i=0;i<node[j]->node_num;i++)
        {
            struct GraphNode *p = node[j]->node[i];
            mat[j][p->ID]=node[j]->length[i];
        }
    }

    char *valid = mMalloc(node_num*4*sizeof(char));
    char *col_valid = valid;                memset(col_valid ,0,node_num*sizeof(char));
    char *col_valid0= col_valid +node_num;  memset(col_valid0,1,node_num*sizeof(char));
    char *row_valid = col_valid0+node_num;  memset(row_valid ,0,node_num*sizeof(char));
    char *row_valid0= row_valid +node_num;  memset(row_valid0,1,node_num*sizeof(char));

    int flag=0; do
    {
        flag=0;
        for(int j=0;j<node_num;j++)for(int i=0;i<node_num;i++)
        {
            if(i==j) continue;
            if(row_valid0[j]+col_valid0[i]==0) continue;
            for(int k=0;k<node_num;k++)
            {
                if((k==i)||(k==j)) continue;
                float d = mat[j][k]+mat[k][i];
                if(d>=mat[j][i]) continue;
                if(linkloss!=NULL)
                {
                    int jj=j;while(tab[jj][k]>=0) jj=tab[jj][k];
                    int ii=i;while(tab[k][ii]>=0) ii=tab[k][ii];
                    d+=linkloss(node[jj],node[k],node[ii],para);
                    if(d>=mat[j][i]) continue;
                }
                mat[j][i]=d;tab[j][i]=k;
                row_valid[j]=1;
                col_valid[i]=1;
                flag=1;
            }
        }
        char *buff;
        buff=col_valid;col_valid=col_valid0;col_valid0=buff;memset(col_valid,0,node_num*sizeof(char));
        buff=row_valid;row_valid=row_valid0;row_valid0=buff;memset(row_valid,0,node_num*sizeof(char));
    }while(flag==1);

    mFree(valid);
}

void WayList(struct GraphNode **node,int ID0,int ID1,int **tab,MList *list)
{
    int ID = tab[ID0][ID1];
    
    if(ID>=0)
    {
        WayList(node,ID0,ID,tab,list);
        mListAppend(list);list->data[list->num-1]=node[ID];
        WayList(node,ID,ID1,tab,list);
    }
}

float m_GraphWay(MList *list,MGraphNode *node0,MGraphNode *node1,void *linkloss,void *para)
{
    struct GraphNode *p0 = (struct GraphNode *)node0;
    struct GraphNode *p1 = (struct GraphNode *)node1;
    MGraph *graph = p0->graph;
    mException(graph!=p1->graph,EXIT,"invalid graph node");
    
    MHandle *hdl = ObjHandle(graph,1);
    mException((hdl->flag != HASH_GraphCreate),EXIT,"invalid input graph");
    struct HandleGraphCreate *handle0 =(struct HandleGraphCreate *)(hdl->handle);
    struct GraphNode **node = handle0->node;
    
    hdl = mHandle(graph,GraphWay);
    struct HandleGraphWay *handle = (struct HandleGraphWay *)(hdl->handle);
    if(handle->graph_edition!=handle0->edition) hdl->valid=0;
    if(handle->linkloss!=linkloss) hdl->valid=0;
    if(hdl->valid == 0)
    {
        handle->graph_edition=handle0->edition;
        handle->linkloss=linkloss;
        GraphWay(graph,linkloss,para);
        hdl->valid = 1;
    }

    int ID0=p0->ID;int ID1=p1->ID;
    float d = handle->mat->data[ID0][ID1];
    if(list!=NULL) mListClear(list);
    if(mIsSup(d)) return DFLT;
    
    if(list!=NULL)
    {
        mListClear(list);
        int **tab = handle->tab->dataS32;
        
        mListAppend(list);list->data[0]=node[ID0];
        WayList(node,ID0,ID1,tab,list);
        mListAppend(list);list->data[list->num-1]=node[ID1];
    }
    return d;
}

struct HandleGraphPath
{
    int *index;
    float *distance;
    float *length;
    MArray *array;
    
    int node_num;
    int edition;
    void *linkloss;
};
#define HASH_GraphPath 0xe1f142fc
void endGraphPath(struct HandleGraphPath *handle)
{
    if(handle->index   !=NULL) mFree(handle->index   );
    if(handle->length  !=NULL) mFree(handle->length  );
    if(handle->distance!=NULL) mFree(handle->distance);
    if(handle->array   !=NULL) mArrayRelease(handle->array);
}
float m_GraphPath(MList *list,MGraphNode *src,MGraphNode *dst,void *linkloss,void *para)
{
    if(src==dst) return 0.0f;
    struct GraphNode *p0 = (struct GraphNode *)src;
    struct GraphNode *p1 = (struct GraphNode *)dst;

    MGraph *graph = p0->graph;
    mException(graph!=p1->graph,EXIT,"invalid graph node");
    
    MHandle *hdl = ObjHandle(graph,1);
    mException((hdl->flag != HASH_GraphCreate),EXIT,"invalid input graph");
    struct HandleGraphCreate *handle0 =(struct HandleGraphCreate *)(hdl->handle);
    int node_num = handle0->node_num;
    
    hdl=mHandle(graph,GraphPath);
    struct HandleGraphPath *handle=hdl->handle;
    if((handle->edition!=handle0->edition)||(handle->linkloss!=linkloss))
    {
        handle->edition=handle0->edition;
        handle->linkloss=linkloss;
        
        if(handle->array==NULL) handle->array=mArrayCreate(sizeof(int));
        
        if(handle->node_num!=node_num)
        {
            if(handle->index   !=NULL){mFree(handle->index   );} handle->index   =mMalloc(node_num*sizeof(int  ));
            if(handle->distance!=NULL){mFree(handle->distance);} handle->distance=mMalloc(node_num*sizeof(float));
            if(handle->length  !=NULL){mFree(handle->length  );} handle->length  =mMalloc(node_num*sizeof(float));
        }
        hdl->valid=1;
    }
    
//     float (*link_loss)(struct GraphNode *,struct GraphNode *,struct GraphNode *,void *) = linkloss;
    
    mArrayClear(handle->array);mArrayWrite(handle->array,&(p0->ID));
    memset(handle->index   ,0xff,node_num*sizeof(int));handle->index[   p0->ID]=p0->ID;
    memset(handle->distance,0x7f,node_num*sizeof(int));handle->distance[p0->ID]=0;
                                                       handle->length[  p0->ID]=0;
    float target_distance=mSup();
    for(int n=0;n<handle->array->num;n++)
    {
        struct GraphNode *node=handle0->node[handle->array->dataS32[n]];
        int ID0=node->ID;
        printf("\nID0=%d,handle->index[ID0]=%d,handle->distance[ID0]=%f\n",ID0,handle->index[ID0],handle->distance[ID0]);
        
        float d0=0;int idx=ID0;do
        {
            if(d0<1000000) printf("idx======%d,length=%f\n",idx,handle->length[idx]);
            else exit(0);
            d0+=handle->length[idx];
            idx=handle->index[ idx];
        }while(idx!=p0->ID);
        
//         float l=d0;idx=ID0;
//         while(l!=handle->distance[idx])
//         {
//             handle->distance[idx]=l;
//             l -=handle->length[idx];
//             idx=handle->index[ idx];
//         }
        
        printf("d0=%f\n",d0);
        handle->distance[ID0]=d0;
        if(d0>=target_distance) continue;
        
        for(int i=0;i<node->node_num;i++)
        {
            struct GraphNode *p=node->node[i];
            int ID=p->ID;
            
            float node_length=node->length[i];
            float l=d0+node_length;
            
//             if(ID0==150)
//                 printf("ID=%d,node_length=%f,l=%f,handle->distance[ID]=%f\n",ID,node_length,l,handle->distance[ID]);
            
            if(l>=target_distance     ) continue;
            if(l>=handle->distance[ID]) continue;
            
//             if(link_loss!=NULL)
//             {
//                 struct GraphNode *pp=handle0->node[handle->index[ID0]];
//                 if(pp!=node)
//                 {
//                     node_length+=link_loss(handle0->node[handle->index[ID0]],node,p,para);
//                     l=d0+node_length;
//                     if(l>= target_distance    ) continue;
//                     if(l>=handle->distance[ID]) continue;
//                 }
//             }
            printf("ID0=%d,ID=%d,length=%f,l=%f\n",ID0,ID,node_length,l);

            handle->index[ID]=ID0;handle->distance[ID]=l;handle->length[ID]=node_length;
            if(ID==p1->ID) {target_distance=l;/*printf("\n");*/}
            else mArrayWrite(handle->array,&ID);
        }
        
//         printf("handle->index[535]======%d,handle->distance[535]=====%f\n\n",handle->index[535],handle->distance[535]);
        
    }    
    
    if(list==NULL) return target_distance;
    
    int *array=handle->array->dataS32;int idx=p1->ID;
    int n;for(n=0;idx!=p0->ID;n++) {array[n]=idx;idx=handle->index[idx];}
    mListAppend(list,n+1);list->data[0]=src;for(int i=1;i<=n;i++) list->data[i]=handle0->node[array[n-i]];
    return target_distance;
}


struct HandleGraphRoute
{
    int num;
    MChain *map;
    MChainNode **list;
    int list_num;
    
    int *index;
    float *distance;
    
    int node_num;
    int edition;
    void *linkloss;
};

#define HASH_GraphRoute 0x538d4d0
void endGraphRoute(struct HandleGraphRoute *handle)
{
    if(handle->map     !=NULL) mChainRelease(handle->map);
    if(handle->list    !=NULL) mFree(handle->list);
    if(handle->index   !=NULL) mFree(handle->index);
    if(handle->distance!=NULL) mFree(handle->distance);
}

void _GraphListAppend(struct HandleGraphRoute *handle)
{
    MChainNode *node0 = handle->list[0];
    
    int list_num =handle->list_num;
    if(list_num/2>=handle->num)
    {
        handle->list_num = MAX(list_num/4,2);
        list_num=handle->list_num;
    }
    else if(list_num*2<=handle->num) 
    {
        handle->list_num = list_num*2;
        list_num=handle->list_num;
        if(list_num>128)
        {
            m_Free(handle->list);
            handle->list = (MChainNode **)mMalloc((list_num+1)*sizeof(MChainNode *));
        }
    }
    MChainNode **list = handle->list;
    
    float k=(float)(list_num)/(float)(handle->num);
    
    MChainNode *node = node0->prev;
    for(int i=handle->num-1;i>=0;i--)
    {
        int idx=(int)(k*i);list[idx]=node;
        node = node->prev;
    }
    list[       0]=node0;
    list[list_num]=node0;
}

MChainNode *_GraphNodeSeek(struct HandleGraphRoute *handle,const float distance,int index)
{
    MChainNode **list=handle->list;
    if(handle->num==1) {return list[0];}
    
    int step = (handle->list_num+1)/4;
    int n=MAX(handle->list_num/2,1);

    MChainNode *node = list[n];
    float *data = (float *)(node->data);
    int f;float d=data[0]-distance;if(d==0.0f) f=*((int *)(data+1))-index;else f=(d>0)?1:-1;
    while(step!=0)
    {
        if(f<0) n=n+step;else n=n-step;
        node = list[n];data = (float *)(node->data);
        d=data[0]-distance;if(d==0.0f) f=*((int *)(data+1))-index;else f=(d>0)?1:-1;
        step=step>>1;
    }
    
    MChainNode *node0=node,*node1=node;
    if(f>0) {do{n=n-1;node0=list[n];}while(node0==node);node1=node;      }
    else    {do{n=n+1;node1=list[n];}while(node1==node);node0=node;n=n-1;}
    node=node0->next;
    
    int count=0;
    while(node!=node1)
    {
        data = (float *)(node->data);
        d=data[0]-distance;if(d==0.0f) f=*((int *)(data+1))-index;else f=(d>0)?1:-1;
        if(f >0) break;
        count++;if(count>16)break;
        node=node->next;
    }
    
    if(count>16)
    {
        _GraphListAppend(handle);
        return _GraphNodeSeek(handle,distance,index);
    }
    if(count>4)
    {
        if(n==0) list[1]=node1->prev->prev;
        else     list[n]=node0->next->next;
    }
    return node;
}

void _GraphMapWrite(struct HandleGraphRoute *handle,MChainNode *chain_node,float distance,int index)
{
    MChainNode *node = mChainNode(handle->map,NULL,sizeof(float)+sizeof(int));
    float *data = (float *)(node->data);data[0] = distance;*((int *)(data+1))=index;
    
    MChainNode *n=chain_node->next;
    if((distance<=*((float *)(n->data)))||(n==handle->map->chainnode))
        mChainNodeInsert(chain_node,node,NULL);
    else
    {
        MChainNode *p = _GraphNodeSeek(handle,distance,index);
        mChainNodeInsert(NULL,node,p);
    }
    handle->num++;if(handle->num>=handle->list_num*2)_GraphListAppend(handle);
}

float m_GraphRoute(MList *list,MGraphNode *src,MGraphNode *dst,void *linkloss,void *para)
{
    if(src==dst) return 0.0f;
    struct GraphNode *p0 = (struct GraphNode *)src;
    struct GraphNode *p1 = (struct GraphNode *)dst;

    MGraph *graph = p0->graph;
    mException(graph!=p1->graph,EXIT,"invalid graph node");
    
    MHandle *hdl = ObjHandle(graph,1);
    mException((hdl->flag != HASH_GraphCreate),EXIT,"invalid input graph");
    struct HandleGraphCreate *handle0 =(struct HandleGraphCreate *)(hdl->handle);
    int node_num = handle0->node_num;
    
    hdl=mHandle(graph,GraphRoute);
    struct HandleGraphRoute *handle=hdl->handle;
    if((handle->edition!=handle0->edition)||(handle->linkloss!=linkloss))
    {
        handle->edition=handle0->edition;
        handle->linkloss=linkloss;
        
        if(handle->map ==NULL) handle->map =mChainCreate();
        if(handle->list==NULL) handle->list=(MChainNode **)m_Malloc(129*sizeof(MChainNode *));
        
        if(handle->node_num!=node_num)
        {
            if(handle->index   !=NULL) {mFree(handle->index   );}handle->index   =mMalloc(node_num*sizeof(int  ));
            if(handle->distance!=NULL) {mFree(handle->distance);}handle->distance=mMalloc(node_num*sizeof(float));
        }
        hdl->valid=1;
    }

    float (*link_loss)(struct GraphNode *,struct GraphNode *,struct GraphNode *,void *) = linkloss;
    
    MChain*map=handle->map;
    mChainClear(map);
    MChainNode *node = mChainNode(map,NULL,sizeof(float)+sizeof(int));
    float *data = (float *)(node->data);data[0]=0;data[1]=0;
    map->chainnode =node;
    handle->list[0]=node;
    handle->list[1]=node;
    handle->list_num=1;
    handle->num=1;
    memset(handle->index   ,0xff,node_num*sizeof(int));handle->index[   p0->ID]=p0->ID;
    memset(handle->distance,0x7f,node_num*sizeof(int));handle->distance[p0->ID]=0;
    
    int idx=p0->ID;float d=0;
    handle->distance[idx]=0;
    
    _GraphMapWrite(handle,node,d,(float)idx);
    
    MChainNode *chain_node=map->chainnode->next;
    while(idx!=p1->ID)
    {
        struct GraphNode *node=handle0->node[idx];
        float d0=handle->distance[idx];
//         printf("\nidx=%d,node->data=%s,d0=%f\n",idx,node->data,d0);
        
        handle->index[idx]=0-handle->index[idx];
        
        for(int i=0;i<node->node_num;i++)
        {
            int  id=node->node[i]->ID;if(id<0) continue;
            float d=d0+node->length[i];
//             printf("id===%d,data=%s,d=%f\n",id,node->node[i]->data,d);
            
            if(d>=handle->distance[id]) continue;
            if(linkloss!=NULL)
            {
                d+=link_loss(handle0->node[0-handle->index[idx]],node,node->node[i],para);
                if(d>=handle->distance[id]) continue;
            }
          
//             printf("write %d\n",id);
            _GraphMapWrite(handle,chain_node,d,(float)id);
            handle->distance[id]=d;
            handle->index[   id]=idx;
        }
    
        while(chain_node!=map->chainnode)
        {
            chain_node=chain_node->next;
            int *data=chain_node->data;
            idx=data[1];
//             printf("idx=%d\n",idx);
            if(handle->index[idx]>=0) break;
        }
    }
    
    float distance=handle->distance[idx];
//     printf("\ndistance======%f\n",distance);
    
    if(list==NULL) return distance;
    
    int *array=(int *)handle->distance;
    array[0]=idx;idx=handle->index[idx];
//     printf("idx=%d\n",idx);
    int n;for(n=1;idx!=p0->ID;n++) {array[n]=idx;idx=0-handle->index[idx];/*printf("idx=%d\n",idx);*/}
    mListAppend(list,n+1);list->data[0]=src;for(int i=1;i<=n;i++) list->data[i]=handle0->node[array[n-i]];
    
    return distance;
}

/*
struct HandleGraphRoute
{
    MChain *chain;
    
    int *index;
    float *distance;
    
    int node_num;
    int edition;
    void *linkloss;
};

#define HASH_GraphRoute 0x538d4d0
void endGraphRoute(struct HandleGraphRoute *handle)
{
    if(handle->chain   !=NULL) mChainRelease(handle->chain);
    if(handle->index   !=NULL) mFree(handle->index);
    if(handle->distance!=NULL) mFree(handle->distance);
}

float m_GraphRoute(MList *list,MGraphNode *src,MGraphNode *dst,void *linkloss,void *para)
{
    if(src==dst) return 0.0f;
    struct GraphNode *p0 = (struct GraphNode *)src;
    struct GraphNode *p1 = (struct GraphNode *)dst;

    MGraph *graph = p0->graph;
    mException(graph!=p1->graph,EXIT,"invalid graph node");
    
    MHandle *hdl = ObjHandle(graph,1);
    mException((hdl->flag != HASH_GraphCreate),EXIT,"invalid input graph");
    struct HandleGraphCreate *handle0 =(struct HandleGraphCreate *)(hdl->handle);
    int node_num = handle0->node_num;
    
    hdl=mHandle(graph,GraphRoute);
    struct HandleGraphRoute *handle=hdl->handle;
    if((handle->edition!=handle0->edition)||(handle->linkloss!=linkloss))
    {
        handle->edition=handle0->edition;
        handle->linkloss=linkloss;
        
        if(handle->chain==NULL) handle->chain=mChainCreate();
        
        if(handle->node_num!=node_num)
        {
            if(handle->index   !=NULL) mFree(handle->index   );handle->index   =mMalloc(node_num*sizeof(int  ));
            if(handle->distance!=NULL) mFree(handle->distance);handle->distance=mMalloc(node_num*sizeof(float));
        }
        hdl->valid=1;
    }

    float (*link_loss)(struct GraphNode *,struct GraphNode *,struct GraphNode *,void *) = linkloss;
    
    int idx=p0->ID;
    MChain*chain=handle->chain;
    mChainClear(chain);
    MChainNode *chain_node = mChainNode(chain,NULL,sizeof(float)+sizeof(int));
    float *data = (float *)(chain_node->data);data[0]=0;*((int *)(data+1))=idx;
    chain->chainnode =chain_node;
    
    memset(handle->index   ,0xff,node_num*sizeof(int));handle->index[   p0->ID]=p0->ID;
    memset(handle->distance,0x7f,node_num*sizeof(int));handle->distance[p0->ID]=0;
    
    handle->distance[idx]=  0;
    handle->   index[idx]=idx;
    
    MChainNode *search=chain_node;
    while(idx!=p1->ID)
    {
        struct GraphNode *node=handle0->node[idx];
        float d0=handle->distance[idx];
//         printf("\nidx=%d,node->data=%s,d0=%f\n",idx,node->data,d0);
//         if(d0>100000) exit(0);
        
        handle->index[idx]=0-handle->index[idx];
        
        for(int i=0;i<node->node_num;i++)
        {
            int  id=node->node[i]->ID;if(id<0) continue;
            float d=d0+node->length[i];
//             printf("id===%d,d=%f\n",id,d);            
            if(d>=handle->distance[id]) continue;
            if(linkloss!=NULL)
            {
                d+=link_loss(handle0->node[0-handle->index[idx]],node,node->node[i],para);
                if(d>=handle->distance[id]) continue;
            }
          
//             printf("write %d\n",id);
            MChainNode *cn = mChainNode(chain,NULL,sizeof(float)+sizeof(int));
            float *data = (float *)(cn->data);data[0]=d;*((int *)(data+1))=id;
            MChainNode *cn0=(d>=*((float *)(search->data)))?search:chain_node;
//             printf("serach =%f\n",*((float *)(search->data)));
            while(d>=*((float *)(cn0->data)))
            {
//                 printf("chain ddddddd=%f\n",*((float *)(cn0->data)));
                cn0=cn0->next;
                if(cn0==chain->chainnode) break;
            }
            mChainNodeInsert(cn0->prev,cn,cn0);
            search=cn;
            handle->distance[id]=d;
            handle->index[   id]=idx;
        }
    
        chain_node=chain_node->next;
        if(chain_node==chain->chainnode) return DFLT;
        while(chain_node!=chain->chainnode)
        {
            int *data=chain_node->data;
            idx=data[1];
//             printf("idx=%d\n",idx);
            
            if(handle->index[idx]>=0) break;
            chain_node=chain_node->next;
        }
    }
    
    float distance=handle->distance[idx];
//     printf("\ndistance======%f\n",distance);
    
    if(list==NULL) return distance;
    
    int *array=(int *)handle->distance;
    array[0]=idx;idx=handle->index[idx];
//     printf("idx=%d\n",idx);
    int n;for(n=1;idx!=p0->ID;n++) {array[n]=idx;idx=0-handle->index[idx];}
    mListAppend(list,n+1);list->data[0]=src;for(int i=1;i<=n;i++) list->data[i]=handle0->node[array[n-i]];
    
    return distance;
}
*/

