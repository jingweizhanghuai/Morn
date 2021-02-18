/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_pipeline.c -o test_pipeline.exe -lmorn 

#include "morn_ptc.h"
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
 
void step0(void *para) {for(int i=0;i<100;i++) {struct PipeData *p=mPipeline(pipe,0);     p->data0=mRand(0,100);  } mPipelineComplete(pipe,0);}
void step1(void *para) {while(1) {struct PipeData *p=mPipeline(pipe,1);if(p==NULL)return;p->data1=p->data0+5;    }}
void step2(void *para) {while(1) {struct PipeData *p=mPipeline(pipe,2);if(p==NULL)return;p->data2=p->data1*2;    }}
void step3(void *para) {while(1) {struct PipeData *p=mPipeline(pipe,3);if(p==NULL)return;p->data3=sqrt(p->data2);}}
void step4(void *para) {while(1) {struct PipeData *p=mPipeline(pipe,4);if(p==NULL)return;p->data4=p->data3/3.0;  }}
void step5(void *para) {while(1) {struct PipeData *p=mPipeline(pipe,5);if(p==NULL)return;printf("%d,%d,%d,%f,%f\n",p->data0,p->data1,p->data2,p->data3,p->data4);}}

int main()
{
    pipe = mListCreate();
    mListPlace(pipe,NULL,6,sizeof(struct PipeData));

    mThread((step0,NULL),(step1,NULL),(step2,NULL),(step3,NULL),(step4,NULL),(step5,NULL));

    mListRelease(pipe);
}

// struct Test
// {
//     int v1;
//     float v2;
//     int valid;
// };
// #define DFLT_TEST {.valid=7}
// struct Test t=DFLT_TEST;
// int main()
// {
//     // int a=6;int *p=&a;int c=100;
//     int b=(__sync_val_compare_and_swap(&(t.valid),7,100)==7);
//     printf("a=%d,b=%d\n",t.valid,b);
// }