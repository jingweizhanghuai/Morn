/*
Copyright (C) 2019-2023 JingWeiZhangHuai <jingweizhanghuai@163.com>
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
            if(handle->index   !=NULL) mFree(handle->index   );handle->index   =mMalloc(node_num*sizeof(int  ));
            if(handle->distance!=NULL) mFree(handle->distance);handle->distance=mMalloc(node_num*sizeof(float));
            if(handle->length  !=NULL) mFree(handle->length  );handle->length  =mMalloc(node_num*sizeof(float));
        }
        hdl->valid=1;
    }
    
    float (*link_loss)(struct GraphNode *,struct GraphNode *,struct GraphNode *,void *) = linkloss;
    
    mArrayClear(handle->array);mArrayWrite(handle->array,&(p0->ID));
    memset(handle->index   ,0xff,node_num*sizeof(int));handle->index[   p0->ID]=p0->ID;
    memset(handle->distance,0x7f,node_num*sizeof(int));handle->distance[p0->ID]=0;
                                                       handle->length[  p0->ID]=0;
    float target_distance=mSup();
    for(int n=0;n<handle->array->num;n++)
    {
        struct GraphNode *node=handle0->node[handle->array->dataS32[n]];
        int ID0=node->ID;
        
        float d0=0;int idx=ID0;do
        {
//             if(d0>1000000) 
//             {
//                 printf("ID0=%d,idx=%d,d0=%f,length=%f\n",ID0,idx,d0,handle->length[idx]);
//                 if(d0>5000000)
//                 {
//                     int iidd=handle->index[ID0 ];printf("iidd=%d\n",iidd);
//                         iidd=handle->index[iidd];printf("iidd=%d\n",iidd);
//                         iidd=handle->index[iidd];printf("iidd=%d\n",iidd);
//                         iidd=handle->index[iidd];printf("iidd=%d\n",iidd);
//                         iidd=handle->index[iidd];printf("iidd=%d\n",iidd);
//                         iidd=handle->index[iidd];printf("iidd=%d\n",iidd);
//                     exit(0);
//                 }
//             }
            d0+=handle->length[idx];
            idx=handle->index[ idx];
        }while(idx!=p0->ID);
        
        handle->distance[ID0]=d0;
        if(d0>=target_distance) continue;
        
        for(int i=0;i<node->node_num;i++)
        {
            struct GraphNode *p=node->node[i];
            int ID=p->ID;
            if(ID==handle->index[ID0]) continue;
            
            float node_length=node->length[i];
            float l=d0+node_length;
            if(l>=target_distance     ) continue;
            if(l>=handle->distance[ID]) continue;
            
            if(link_loss!=NULL)
            {
                struct GraphNode *pp=handle0->node[handle->index[ID0]];
                if(pp!=node)
                {
                    node_length+=link_loss(handle0->node[handle->index[ID0]],node,p,para);
                    l=d0+node_length;
                    if(l>=target_distance     ) continue;
                    if(l>=handle->distance[ID]) continue;
                }
            }
//             printf("ID0=%d,ID=%d,length=%f,l=%f\n",ID0,ID,node_length,l);

            handle->index[ID]=ID0;handle->distance[ID]=l;handle->length[ID]=node_length;
            if(ID==p1->ID) {target_distance=l;/*printf("\n");*/}
            else mArrayWrite(handle->array,&ID);
        }
    }    
    
    if(list==NULL) return target_distance;
    
    int *array=handle->array->dataS32;int idx=p1->ID;
    int n;for(n=0;idx!=p0->ID;n++) {array[n]=idx;idx=handle->index[idx];}
    mListAppend(list,n+1);for(int i=0;i<=n;i++) list->data[i]=handle0->node[array[n-i]];
    return target_distance;
}
