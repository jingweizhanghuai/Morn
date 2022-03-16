#include "morn_util.h"

// float myGraphPath(MList *list,MGraphNode *src,MGraphNode *dst,void *linkloss,void *para);

int main()
{
    MGraph *graph = mGraphCreate();
    MGraphNode *node_a = mGraphNode(graph,"a");
    MGraphNode *node_b = mGraphNode(graph,"b");
    MGraphNode *node_c = mGraphNode(graph,"c");
    MGraphNode *node_d = mGraphNode(graph,"d");
    MGraphNode *node_e = mGraphNode(graph,"e");
    MGraphNode *node_f = mGraphNode(graph,"f");

    mGraphNodeLink(node_a,node_b,1);
    mGraphNodeLink(node_a,node_c,12);
    
    mGraphNodeLink(node_b,node_c,9);
    mGraphNodeLink(node_b,node_d,3);
    
    mGraphNodeLink(node_c,node_e,5);
    
    mGraphNodeLink(node_d,node_c,4);
    mGraphNodeLink(node_d,node_e,13);
    mGraphNodeLink(node_d,node_f,15);

    mGraphNodeLink(node_e,node_f,4);

    MList *list = mListCreate();
    mGraphPath(list,node_a,node_f,NULL,NULL);

    // 
    // float d;
    // d=mGraphPath(node_a,node_f,list);
    // if(d>0)
    // {
    //     for(int i=0;i<list->num;i++)
    //     {
    //         MGraphNode *node = list->data[i];
    //         printf("%s>>",node->data);
    //     }
    //     printf("\b\b:");
    // }
    // printf("d=%f\n",d);

    // d=mGraphPath(node_b,node_c,list);
    // if(d>0)
    // {
    //     for(int i=0;i<list->num;i++)
    //     {
    //         MGraphNode *node = list->data[i];
    //         printf("%s>>",node->data);
    //     }
    //     printf("\b\b:");
    // }
    // printf("d=%f\n",d);

    // d=mGraphPath(node_d,node_a,list);
    // if(d>0)
    // {
    //     for(int i=0;i<list->num;i++)
    //     {
    //         MGraphNode *node = list->data[i];
    //         printf("%s>>",node->data);
    //     }
    //     printf("\b\b:");
    // }
    // printf("d=%f\n",d);
    
    mListRelease(list);
    mGraphRelease(graph);
}