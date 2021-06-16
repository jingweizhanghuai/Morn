
// gcc -O2 -fopenmp test_JSON_file2.c -o test_JSON_file2.exe -I ..\include\ -L ..\lib\x64_mingw -lmorn
#include "morn_util.h"

void PrintNode(MTreeNode *node);

int main()
{
    MTree *json=mTreeCreate();
    mJSONLoad(json,"./china.json");
    printf("aaaaaaaaa\n");

    MTreeNode *node = json->treenode;
    node = node->child[1];
    PrintNode(node);

    // mTreeRelease(json);
}
