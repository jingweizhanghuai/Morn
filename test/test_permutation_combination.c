/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

// 编译：gcc -O2 -fopenmp test_permutation_combination.c -o test_permutation_combination.exe -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -lm -static
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "morn_image.h"

void test1()
{
    int idx[10];
    while(1)
    {
        int i=mCombination(idx,3,5);if(i<0) break;
        printf("i=%d:idx=%d,%d,%d\n",i,idx[0],idx[1],idx[2]);
    }
    while(1)
    {
        int i=mPermutation(idx,3,5);if(i<0) break;
        printf("i=%d:idx=%d,%d,%d\n",i,idx[0],idx[1],idx[2]);
    }
}

void test2()
{
    int data[10] = {1,2,3,4,5,6,7,8,9,10};
    int idx[4];
    while(mCombination(idx,4,10)>=0)
    {
        int d0=data[idx[0]];int d1=data[idx[1]];int d2=data[idx[2]];int d3=data[idx[3]];
        if(d0+d1+d2+d3==20) printf("%d+%d+%d+%d==20\n",d0,d1,d2,d3);
    }
}

void test3()
{
    MImagePoint p[4];
    for(int i=0;i<4;i++) {p[i].x=mRand(0,100);p[i].y=mRand(0,100);}
    printf("point:\n(%f,%f),(%f,%f),(%f,%f),(%f,%f)\n",p[0].x,p[0].y,p[1].x,p[1].y,p[2].x,p[2].y,p[3].x,p[3].y);
    MImagePoint *p0,*p1,*p2,*p3;
    p3=p+3;
    int idx[3];
    while(mPermutation(idx,3,3)>=0)
    {
        p0=p+idx[0];p1=p+idx[1];p2=p+idx[2];
        
        if((mLineCross(p0,p1,p2,p3,NULL)==0)&&(mLineCross(p0,p3,p1,p2,NULL)==0))
            printf("Quadrangle:\n(%f,%f),(%f,%f),(%f,%f),(%f,%f)\n",p0->x,p0->y,p1->x,p1->y,p2->x,p2->y,p3->x,p3->y);
    }
}

int main()
{
    test1();
    // test2();
    // test3();
    return 0;
}