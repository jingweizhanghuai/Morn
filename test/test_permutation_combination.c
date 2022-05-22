/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

// 编译：gcc -O2 -fopenmp test_permutation_combination.c -o test_permutation_combination.exe -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -lm -static
#include "morn_image.h"

void test_combination()
{
    printf("combination 3 element from 5\n");
    MList *list=mListCreate();
    for(int i=0;i<5;i++) mListWrite(list,DFLT,&i,sizeof(int));
    while(1)
    {
        MList *out=mCombination(list,3);
        if(out==NULL) break;
        int **idx=(int **)(out->data);
        printf("idx=%d,%d,%d\n",*(idx[0]),*(idx[1]),*(idx[2]));
    }
    mListRelease(list);
}

void test_permutation1()
{
    printf("permutation 3 elements from 5\n");
    MList *list=mListCreate();
    for(int i=0;i<5;i++) mListWrite(list,DFLT,&i,sizeof(int));
    while(1)
    {
        MList *out=mPermutation(list,3);
        if(out==NULL) break;
        int **idx=(int **)(out->data);
        printf("idx=%d,%d,%d\n",*(idx[0]),*(idx[1]),*(idx[2]));
    }
    mListRelease(list);
}

void test_permutation2()
{
    printf("permutation 5 elements\n");
    MList *list=mListCreate();
    for(int i=0;i<5;i++) mListWrite(list,DFLT,&i,sizeof(int));
    while(1)
    {
        MList *out=mPermutation(list);
        if(out==NULL) break;
        int **idx=(int **)(out->data);
        printf("idx=%d,%d,%d,%d,%d\n",*(idx[0]),*(idx[1]),*(idx[2]),*(idx[3]),*(idx[4]));
    }
    mListRelease(list);
}

void test1()
{
    int data[10] = {1,2,3,4,5,6,7,8,9,10};
    MList *list = mListCreate();
    mListPlace(list,data,10,sizeof(int));
    MList *out;
    while(out=mCombination(list,4))
    {
        int **p=(int **)(out->data);
        if(*p[0]+*p[1]+*p[2]+*p[3]==20) printf("%2d+%2d+%2d+%2d==20\n",*p[0],*p[1],*p[2],*p[3]);
    }
    mListRelease(list);
}

void test2()
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
}



int main()
{
    // test_combination();
    // test_permutation1();
    // test_permutation2();
    // test1();
    test2();
    return 0;
}