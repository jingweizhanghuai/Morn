## Morn：二叉树MBtree

二叉树和树的区别是，树可以有任意多个子节点，而二叉树只有两个子节点。但是二叉树比树更常用。



#### MBtree定义

```
#define MBtree MObject
```

可以看到，MBtree就是MObject，Btree->btreenode指向的是二叉树的入口（根节点）。

关键是MBtreeNode的定义：

```
typedef struct MBtreeNode
{
    void *data;
    struct MBtreeNode *left;
    struct MBtreeNode *right;
    struct MBtreeNode *parent;
}MBtreeNode;
```

这个也极简单，data指向的是节点里存放的数据，left是节点的左子节点，right是节点的右子节点，parent就是节点的父节点。如果一个节点没有左子节点的话则left为NULL，没有右子节点的话则right为NULL，两者都冇的话则为叶节点。



#### MBtree的基本操作



##### 创建一个二叉树

```c
MBtree *mBtreeCreate();
```

因为创建的时候二叉树里还没有节点所以返回的btree的btreenode是指向NULL。



##### 释放一个二叉树

```c
void mBtreeRelease(MBtree *Btree);
```



##### 在二叉树里新建一个节点

```c
MBtreeNode *mBtreeNode(MBtree *Btree,void *data,int size);
```

data是节点的数据的首地址，数据可以是任何类型。size是数据的大小。data可以设为NULL，但(data==NULL)&&(size==DFLT)的话没有意义。如果数据是字符串的话，size可以设为DFLT。



##### 设置节点在二叉树中的位置

```c
void mBtreeNodeSet(MBtreeNode *node,MBtreeNode *parent,int order)；
```

node是待设置的节点，parent是父节点，order表示是设置为父节点的左子节点（MORN_LEFT）还是右子节点（MORN_RIGHT）。

如果节点被设置的位置上原已有节点，那么原节点将会被覆盖。



##### 遍历二叉树

```c
void mBtreeOperate(MBtree *tree,void (*func)(MBtreeNode *,void *),void *para,int mode)
```

func是遍历节点时对每个节点的操作，para是func的参数。mode是遍历顺序，可以为：

MORN_TREE_PREORDER_TRAVERSAL：前序遍历

MORN_TREE_POSTORDER_TRAVERSAL：后续遍历

MORN_TREE_INORDER_TRAVERSAL：中续遍历



##### 二叉树决策

```c
MTreeNode *mBTreeDecide(MBTree *btree,int (*func)(MBTreeNode *,void *),void *para);
```

这个函数的前提是btree是一棵决策树。func是决策函数，para是func的参数。返回值是决策找到的节点。

