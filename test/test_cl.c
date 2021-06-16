/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: gcc -O2 -fopenmp test_cl.c -o test_cl.exe -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -lOpenCL
#include "morn_util.h"

char *vector_add_gpu1 = mString(
__kernel void vector_add_gpu1(__global const float* a,__global const float* b,__global float* c)
{
    const int y = get_global_id(0);
    const int x = get_global_id(1);
    const int width = get_global_size(1);
    const int idx = y*width+x;
    c[idx] = a[idx] + b[idx];
});

char *vector_add_gpu2 = mString(
__kernel void vector_add_gpu2(__global float* a,const float d)
{
    const int idx = get_global_id(0);
    a[idx] = a[idx] + d;
});

char *vector_add_gpu3 = mString(
float add_gpu(const float a,const float b)
{
    return (a+b);
}
__kernel void vector_add_gpu3(__global const float* a,__global const float* b,__global float* c)
{
    const int idx = get_global_id(0);
    c[idx]=add_gpu(a[idx],b[idx]);
});

int main()
{
    MMemoryBlock *block_a = mMemoryBlockCreate(1000*sizeof(float),MORN_CL_GPU(0));float *a=block_a->data;
    MMemoryBlock *block_b = mMemoryBlockCreate(1000*sizeof(float),MORN_CL_GPU(0));float *b=block_b->data;
    MMemoryBlock *block_c = mMemoryBlockCreate(1000*sizeof(float),MORN_CL_GPU(0));float *c=block_c->data;
    
    for(int i=0;i<1000;i++){a[i]=(float)i;b[i]=(float)i;}
    
    // mCLFunction(vector_add_gpu1,CLSIZE(20,50),CLIN(block_a),CLIN(block_b),CLOUT(block_c));
    // mMemoryBlockRead(block_c);
    // for(int i=0;i<200;i++) printf("a=%f,b=%f,c=%f\n",a[i],b[i],c[i]);

    // float d=0.3;
    // mCLFunction(vector_add_gpu2,CLSIZE(1000),CLINOUT(block_a),CLPARA(&d,sizeof(float)));
    // mMemoryBlockRead(block_a);
    // for(int i=0;i<200;i++) printf("a=%f\n",a[i]);

    mCLFunction(vector_add_gpu3,CLSIZE(1000),CLIN(block_a),CLIN(block_b),CLOUT(block_c));
    mMemoryBlockRead(block_c);
    for(int i=0;i<200;i++) printf("a=%f,b=%f,c=%f\n",a[i],b[i],c[i]);
    
    mMemoryBlockRelease(block_a);
    mMemoryBlockRelease(block_b);
    mMemoryBlockRelease(block_c);
}