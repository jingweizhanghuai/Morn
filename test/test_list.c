/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译：gcc -O2 -fopenmp test_List.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_List.exe

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "morn_util.h"

int test0()
{
    MList *list = mListCreate(DFLT,NULL);
    printf("\n\nbefore\n");
    for(int i=0;i<8;i++)
    {
        int a=mRand(0,10);
        printf("%d,",a);
        mListWrite(list,DFLT,&a,sizeof(int));
    }
    void Threshold(void *data,void *para)
    {
        *((int *)data)=(*((int *)data)>5)?10:0;
    }
    mListElementOperate(list,Threshold,NULL);
    printf("\nafter\n");
    for(int i=0;i<list->num;i++)
        printf("%d,",*((int *)list->data[i]));
    mListRelease(list);
}

int test1()
{
    MList *list = mListCreate(DFLT,NULL);
    printf("\n\nbefore\n");
    for(int i=0;i<8;i++)
    {
        int a=mRand(0,10);
        printf("%d,",a);
        mListWrite(list,DFLT,&a,sizeof(int));
    }
    int EvenScreen(void *data,void *para)
    {
        return (*((int *)data)%2==0);
    }
    mListElementScreen(list,EvenScreen,NULL);
    printf("\nafter\n");
    for(int i=0;i<list->num;i++)
        printf("%d,",*((int *)list->data[i]));
    mListRelease(list);
}

int test2()
{
    MList *list = mListCreate(DFLT,NULL);
    printf("\n\nbefore\n");
    for(int i=0;i<8;i++)
    {
        int a=mRand(0,10);
        printf("%d,",a);
        mListWrite(list,DFLT,&a,sizeof(int));
    }
    void DuplicateDelete(void *data1,void *data2,int *flag1,int *flag2,void *para)
    {
        *flag1 = 1;
        *flag2 = (*((int *)data2)!=*((int *)data1));
    }
    mListElementSelect(list,DuplicateDelete,NULL);
    printf("\nafter\n");
    for(int i=0;i<list->num;i++)
        printf("%d,",*((int *)list->data[i]));
    mListRelease(list);
}

int test3()
{
    MList *list = mListCreate(DFLT,NULL);
    printf("\n\nbefore\n");
    for(int i=0;i<8;i++)
    {
        int a=mRand(0,10);
        printf("%d,",a);
        mListWrite(list,DFLT,&a,sizeof(int));
    }
    int Compare(void *data1,void *data2,void *para)
    {
        return (*((int *)data1)-*((int *)data2));
    }
    mListSort(list,Compare,NULL);
    printf("\nafter sort\n");
    for(int i=0;i<list->num;i++)
        printf("%d,",*((int *)list->data[i]));
    mListReorder(list);
    printf("\nafter reorder\n");
    for(int i=0;i<list->num;i++)
        printf("%d,",*((int *)list->data[i]));
    mListRelease(list);
}

int test4()
{
    MList *list = mListCreate(DFLT,NULL);
    printf("\n\nbefore\n");
    for(int i=0;i<8;i++)
    {
        int a=mRand(0,10);
        printf("%d,",a);
        mListWrite(list,DFLT,&a,sizeof(int));
    }
    int Neighbor(void *data1,void *data2,void *para)
    {
        return (ABS(*((int *)data1)-*((int *)data2))<=1);
    }
    int group[list->num];
    int group_num = mListCluster(list,group,Neighbor,NULL);
    printf("\nafter\n");
    printf("group num is %d\n",group_num);
    for(int i=0;i<list->num;i++)
        printf("%d(group%d),",*((int *)list->data[i]),group[i]);
    mListRelease(list);
}

int main()
{
    test0();
    test1();
    test2();
    test3();
    test4();
    return 0;
}
