#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

void mJSONLoad(char *filename,MTree *tree);
char *mJSONName(MTreeNode *node);
char *mJSONValue(MTreeNode *node);
MList *mJSONGet(MTree *tree,char *name);
MTreeNode *mTreeSearch(MTreeNode *node,int (*func)(MTreeNode *,void *),void *para,int mode);

void main()
{
    MTree *json=mTreeCreate();
    mJSONLoad("./test_JSON_file.json",json);

    printf("%s是%s\n\n",mJSONName(json->treenode->child[0]),mJSONValue(json->treenode->child[0]));

    MList *list;
    list = mJSONGet(json,"班主任");
    for(int i=0;i<list->num;i++) printf("班主任%d是%s\n",i,list->data[i]);
    printf("\n");

    list = mJSONGet(json,"任课老师");
    for(int i=0;i<list->num;i++) printf("任课老师%d是%s\n",i,list->data[i]);
    printf("\n");

    list = mJSONGet(json,"学生.成绩.语文");
    for(int i=0;i<list->num;i++) printf("成绩%d是%d\n",i,atoi(list->data[i]));
    printf("\n");

    int func(MTreeNode *ptr,void *para)
    {return ((strcmp(mJSONName(ptr),"数学")==0)&&(atoi(mJSONValue(ptr))>=90));}
    MTreeNode *node = json->treenode;
    while(1)
    {
        node = mTreeSearch(node,func,NULL,0);
        if(node == NULL) break;
        printf("姓名%s\n",mJSONValue(node->parent->parent->child[0]));
    }

    mTreeRelease(json);
}
    