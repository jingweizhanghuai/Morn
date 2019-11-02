/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_pipeline2.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_pipeline2.exe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_util.h"
void *mPipeline(MList *list,int thread);
void mPipelineComplete(MList *list,int thread);

struct PipeData
{
    int data0;
    int data1;
    int data2;
    float data3;
    float data4;
};
MList *pipe;

void step0() {for(int i=0;i<10;i++) {struct PipeData *p=mPipeline(pipe,0);     p->data0=mRand(0,100);  } mPipelineComplete(pipe,0);}
void step1() {while(1) {struct PipeData *p=mPipeline(pipe,1);if(p==NULL)return;p->data1=p->data0+5;    }}
void step2() {while(1) {struct PipeData *p=mPipeline(pipe,2);if(p==NULL)return;p->data2=p->data1*2;    }}
void step3() {while(1) {struct PipeData *p=mPipeline(pipe,3);if(p==NULL)return;p->data3=sqrt(p->data2);}}
void step4() {while(1) {struct PipeData *p=mPipeline(pipe,4);if(p==NULL)return;p->data4=p->data3/3.0;  }}
void step5() {while(1) {struct PipeData *p=mPipeline(pipe,5);if(p==NULL)return;printf("%d,%d,%d,%f,%f\n",p->data0,p->data1,p->data2,p->data3,p->data4);}}

int main()
{
    pipe = mListCreate(DFLT,NULL);
    mListPlace(pipe,6,sizeof(struct PipeData));

    mThread(6,step0(),step1(),step2(),step3(),step4(),step5());

    mListRelease(pipe);
}