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
    MHandle *hdl = mHandle(graph,GraphCreate);hdl->valid = 1;
    struct HandleGraphCreate *handle = hdl->handle;

    mPropertyVariate(graph,"linkloss"     ,&(handle->linkloss));
    mPropertyVariate(graph,"linkloss_para",&(handle->linkloss_para));
    
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
    
    struct GraphNode *node = mMemoryWrite(handle->memory,NULL,sizeof(struct GraphNode)+size);
    handle->node[handle->node_num] = node;
    memset(node,0,sizeof(struct GraphNode));
    node->data = (void *)(node+1); 
    node->ID = handle->node_num;
    node->graph = graph;
    handle->node_num+=1;
    if(graph->graphnode==NULL) graph->graphnode = (MGraphNode *)node;

    if(data!=NULL) memcpy(node->data,data,size);
    
    return (MGraphNode *)node;
}

void mGraphNodeLink(MGraphNode *node0,MGraphNode *node,float length)
{
    struct GraphNode *graph = (struct GraphNode *)node0;
    mException(graph->graph!=((struct GraphNode *)node)->graph,EXIT,"invalid input graph node");
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

struct GraphNode *depth_first(struct GraphNode *node,int (*func)(void *,void *),void *para)
{
    int flag = func(node->data,para);
    if(flag == 1) return node;
    node->ID=0-node->ID;
    for(int i=0;i<node->node_num;i++)
    {
        if(node->node[i]->ID<0) continue;
        depth_first(node,func,para);
    }
    return NULL;
}
struct GraphNode *breadth_first(struct GraphNode *node,int (*func)(void *,void *),void *para)
{
    node->ID=0-node->ID;
    for(int i=0;i<node->node_num;i++)
    {
        if(node->node[i]->ID<0) continue;
        int flag=func(node->node[i]->data,para);
        if(flag == 1) return node->node[i];
    }
    for(int i=0;i<node->node_num;i++)
    {
        if(node->node[i]->ID<0) continue;
        breadth_first(node,func,para);
    }
    return NULL;
}

void mGraphTraversal(MGraph *graph,void (*func)(void *,void *),void *para)
{
    MHandle *hdl = ObjHandle(graph,1);
    mException((hdl->flag != HASH_GraphCreate),EXIT,"invalid input graph");
    struct HandleGraphCreate *handle =(struct HandleGraphCreate *)(hdl->handle);
    for(int i=0;i<handle->node_num;i++)
        func(handle->node[i],para);
}

struct HandleGraphWay
{
    MMatrix *mat;
    MTable *tab;
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
        // printf("j=%d,node[j].data=%s,node[j].node_num=%d\n",j,node[j]->data,node[j]->node_num);
        for(int i=0;i<node[j]->node_num;i++)
        {
            struct GraphNode *p = node[j]->node[i];
            mat[j][p->ID] = node[j]->length[i];
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
        for(int j=0;j<node_num;j++)
        {
            // printf("\n");
            for(int i=0;i<node_num;i++)
            {
                // printf("%8.2f,",mat[j][i]);
                if(i==j) continue;
                if(row_valid0[j]+col_valid0[i]==0) continue;
                for(int k=0;k<node_num;k++)
                {
                    if((k==i)||(k==j)) continue;
                    float d = mat[j][k]+mat[k][i];
                    if(d>=mat[j][i]) continue;
                    if(linkloss!=NULL) d+=linkloss(node[j],node[k],node[i],para);
                    if(d<mat[j][i])
                    {
                        mat[j][i]=d;tab[j][i]=k;
                        row_valid[j]=1;
                        col_valid[i]=1;
                        flag=1;
                    }
                }
            }
        }
        // printf("\nflag=%d\n",flag);
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
    
    if(hdl->valid == 0)
    {
        GraphWay(graph,linkloss,para);
        hdl->valid = 1;
    }
    
    struct HandleGraphWay *handle = (struct HandleGraphWay *)(hdl->handle);

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


// float _GraphWay(MGraphNode *node0,MGraphNode *node1,MMatrix *mat)
// {
//     float dmin=d;
//     for(int i=0;i<node0->node_num;i++)
//     {
//         d = _GraphWay(node0->node[i],node1,linkloss,para);
//         if(d<dmin) dmin=d;
//     }
//     return dmin;
// }



void GraphPath(struct GraphNode **node,struct GraphNode *src,int dst,float pre,int order,float *dis,int *path,int *rst,void *link_loss,void *para)
{
    float (*linkloss)(struct GraphNode *,struct GraphNode *,struct GraphNode *,void *) = link_loss;
    for(int i=0;i<src->node_num;i++)
    {
        int ID = src->node[i]->ID;
        float d = pre+src->length[i];
        if(d>=dis[ID]) continue;
        // printf("src=%s,ID=%d,order=%d,path[0]=%d\n",src->data,ID,order,path[0]);
        if((order>1)&&(linkloss!=NULL))
        {
            d=d+linkloss(node[path[order-2]],src,src->node[i],para);
            if(d>=dis[ID]) continue;
        }
        
        dis[ID]=d;
        path[order] = ID;
        if(ID==dst) memcpy(rst,path,(order+1)*sizeof(int));
        else GraphPath(node,src->node[i],dst,d,order+1,dis,path,rst,link_loss,para);
    }
}
float m_GraphPath(MList *list,MGraphNode *src,MGraphNode *dst,void *linkloss,void *para)
{
    // printf("src=%s,dst=%s\n",src->data,dst->data);
    struct GraphNode *p0 = (struct GraphNode *)src;
    struct GraphNode *p1 = (struct GraphNode *)dst;

    MGraph *graph = p0->graph;
    mException(graph!=p1->graph,EXIT,"invalid graph node");
    
    MHandle *hdl = ObjHandle(graph,1);
    mException((hdl->flag != HASH_GraphCreate),EXIT,"invalid input graph");
    struct HandleGraphCreate *handle0 =(struct HandleGraphCreate *)(hdl->handle);
    int node_num = handle0->node_num;
    struct GraphNode **node = handle0->node;

    if(linkloss == NULL) linkloss = handle0->linkloss;
    if(para     == NULL) para     = handle0->linkloss_para;

    int *path=mMalloc(node_num*sizeof(int));
    int *rst =mMalloc(node_num*sizeof(int));
    float *distance = mMalloc(node_num*sizeof(float));
    for(int i=0;i<node_num;i++) distance[i]=mSup();
    distance[p0->ID] = 0;
    path[0]=p0->ID;

    GraphPath(node,p0,p1->ID,0,1,distance,path,rst,linkloss,para);
    float d = distance[p1->ID];
    // printf("distance[p1->ID]=%f\n",d);

    if(list!=NULL)
    {
        mListClear(list);
        // mListAppend(list);list->data[0]=p0;
        for(int i=0;i<node_num;i++)
        {
            mListAppend(list);list->data[i]=node[rst[i]];
            // printf("%s>>",node[rst[i]]->data);
            if(rst[i]==p1->ID) break;
        }
    }
    return d;
}

