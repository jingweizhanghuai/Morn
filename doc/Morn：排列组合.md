## Morn：排列组合

这里的排列组合就是你们所学过的：从口袋里拿球的问题，男生女生排排队问题，乒乓球捉对比赛问题，等等。

简单的讲，排列就是从m个数中有序的取出n个数，组合就是从m个数中无序的取出n个数，当然，m≥n。

Morn提供了排列组合的索引生产函数。

### 接口

#### 组合

```c
MList *mCombination(MList *list,int n);
```

此函数用于从列表list中无序的取出n个元素。返回值是抽取所得元素组成的新列表。

循环的执行此函数，将获得所有可能的组合。若所有的组合都已经被列出时，此函数将返回`NULL`。

一个例子：现有1到10共10个数字，现要求从中取出4个数字，使得4数相加等于20。这时，可以使用如下程序：

```c
int main()
{
    int data[10] = {1,2,3,4,5,6,7,8,9,10};
    MList *list = mListCreate();
    mListPlace(list,data,10,sizeof(int));
    MList *out;
    while(out=mCombination(list,4))
    {
        int **p=(int **)(out->data);
        if(*p[0]+*p[1]+*p[2]+*p[3]==20)
            printf("%2d+%2d+%2d+%2d==20\n",*p[0],*p[1],*p[2],*p[3]);
    }
    mListRelease(list);
    return 0;
}
```

其执行结果为：

```
 1+ 2+ 7+10==20
 1+ 2+ 8+ 9==20
 1+ 3+ 6+10==20
 1+ 3+ 7+ 9==20
 1+ 4+ 5+10==20
 1+ 4+ 6+ 9==20
 1+ 4+ 7+ 8==20
 1+ 5+ 6+ 8==20
 2+ 3+ 5+10==20
 2+ 3+ 6+ 9==20
 2+ 3+ 7+ 8==20
 2+ 4+ 5+ 9==20
 2+ 4+ 6+ 8==20
 2+ 5+ 6+ 7==20
 3+ 4+ 5+ 8==20
 3+ 4+ 6+ 7==20
```

当然，这不是最简单的算法，这里只是演示`mCombination`函数怎么用。



#### 排列

```c
MList *mPermutation(MList *list);
MList *mPermutation(MList *list,int n);
```

此函数用于从列表list中有序的取出n个元素，若不指定n，则为列表中所有元素的全排列。

循环的执行此函数，将获得所有可能的排列。若所有的排列都已经被列出时，此函数将返回`NULL`。

一个例子：平面上有随机的5个点，从其中取出4个点，如何排列，其连线才能组成一个凸四边形（对角线相交的四边形）？这时，可以使用如下程序：

```c
int main()
{
    MList *list = mListCreate();
    for(int i=0;i<5;i++)
    {
        MImagePoint pt;pt.x=mRand(0,100);pt.y=mRand(0,100);
        mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
    }
    MList *polygon;
    while(polygon=mPermutation(list,4))
    {
        MImagePoint **p=(MImagePoint **)(polygon->data);
        if(mLineCross(p[0],p[2],p[1],p[3],NULL))
            printf("Quadrangle:(%.0f,%.0f),(%.0f,%.0f),(%.0f,%.0f),(%.0f,%.0f)\n",p[0]->x,p[0]->y,p[1]->x,p[1]->y,p[2]->x,p[2]->y,p[3]->x,p[3]->y);
    }
    return 0;
}
```

其执行结果为：

```
Quadrangle:(35,20),(2,79),(51,85),(54,29)
Quadrangle:(35,20),(54,29),(51,85),(2,79)
Quadrangle:(2,79),(35,20),(54,29),(51,85)
Quadrangle:(2,79),(51,85),(54,29),(35,20)
Quadrangle:(54,29),(35,20),(2,79),(51,85)
Quadrangle:(54,29),(51,85),(2,79),(35,20)
Quadrangle:(51,85),(2,79),(35,20),(54,29)
Quadrangle:(51,85),(54,29),(35,20),(2,79)
Quadrangle:(35,20),(2,79),(51,85),(75,34)
Quadrangle:(35,20),(75,34),(51,85),(2,79)
Quadrangle:(2,79),(35,20),(75,34),(51,85)
Quadrangle:(2,79),(51,85),(75,34),(35,20)
Quadrangle:(51,85),(2,79),(35,20),(75,34)
Quadrangle:(51,85),(75,34),(35,20),(2,79)
Quadrangle:(75,34),(35,20),(2,79),(51,85)
Quadrangle:(75,34),(51,85),(2,79),(35,20)
Quadrangle:(2,79),(54,29),(75,34),(51,85)
Quadrangle:(2,79),(51,85),(75,34),(54,29)
Quadrangle:(54,29),(2,79),(51,85),(75,34)
Quadrangle:(54,29),(75,34),(51,85),(2,79)
Quadrangle:(51,85),(2,79),(54,29),(75,34)
Quadrangle:(51,85),(75,34),(54,29),(2,79)
Quadrangle:(75,34),(54,29),(2,79),(51,85)
Quadrangle:(75,34),(51,85),(2,79),(54,29)
```



