## Morn：链表MChain

Morn里面的链表是双向循环链表。



####MChain定义

```
#define MChain MObject
```

可以看到，MChain就是MObject，chain->chainnode指向的是链表的入口（第一个node）。

关键是MChainNode的定义：

```
typedef struct MChainNode
{
    void *data;
    struct MChainNode *last;
    struct MChainNode *next;
}MChainNode;
```

这个也极简单，data指向的是节点里存放的数据，last是节点的上一个节点，next是节点的下一个节点。Morn提供的是循环链表，所以第一个节点的上一节点是最后一个节点，最后一个节点的下一节点是第一个节点。



#### MChain的基本操作



##### 创建一个链表

```c
MChain *mChainCreate();
```

因为创建的时候链表里还没有节点所以返回的chain的chainnode是指向NULL。



##### 释放一个链表

```c
void mChainRelease(MChain *chain);
```



##### 在链表里新建一个节点

```c
MChainNode *mChainNode(MChain *chain,void *data,int size);
```

data是节点的数据的首地址，数据可以是任何类型。size是数据的大小。data可以设为NULL，但(data==NULL)&&(size==DFLT)的话没有意义。如果数据是字符串的话，size可以设为DFLT。



##### 将节点插入到链表中

```c
void mChainNodeInsert(MChainNode *last,MChainNode *node,MChainNode *next);
```

node是待插入的节点，last是插入位置的上一个节点，next是插入位置的下一个节点，last和next必须至少有一个不为NULL，但如果两个都不为NULL的话，在插入之前必须(last->next==next)&&(next->lst==last)，也就是说它俩必须是相邻的两个节点。



##### 删除链表中的节点

```c
void mChainNodeDelete(MChainNode *node);
```

注意，这只是把节点从链表中取下来，但它仍然会存在于内存里，直到整个链表被Release（或者直到你手动的做了内存块回收，不建议）。



