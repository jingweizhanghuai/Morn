## Permutation & Combination 

Here, the problem we're trying to be settled  is such as getting the ball out of the pocket, lining up with boys and girls, or round robin for all tennis players, etc.

simply speaking, permutation is select `m` from `n` elements with order, and combination is also select `m` from `n` elements, but without order. `m≥n` of course.

In Morn, permutation and combination use structural `MList` .



### API

#### Combination

```c
MList *mCombination(MList *list,int m);
```

This function is used to select `m` from all the list elements **without order**. It returns a new list of selected elements.

Looping execute of this function will obtain all possible combinations, and if all combinations have already  been listed, it returns `NULL`.



#### Permutation

```c
MList *mPermutation(MList *list);
MList *mPermutation(MList *list,int n);
```

This function is used to select `m` from all the list elements **with order**. It also returns a new list of selected elements.

If `m` is not given, it is all permutations for all list elements.

Looping execute of this function will obtain all possible permutations, and if all permutations have already  been listed, it returns `NULL`.



### Example

#### Combination

Here is a simple example: Trying to pick any 4 numbers from 1 to 10, and make the sum of these 4 numbers is 20.

The programs can be following:

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

 The execution results: 

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

Of course, it is not the easiest algorithm, It is just a`mCombination` demo.



#### Permutation

Think about this question: There are five random points on the plane. How can I arrange the four points so that their lines form a convex quadrilateral (a quadrilateral whose diagonals intersect)?

The programs can be following:

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

 The execution results: 

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



