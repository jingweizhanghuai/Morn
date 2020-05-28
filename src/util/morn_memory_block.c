/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "morn_util.h"

#if defined MORN_USE_CUDA
#include "cuda_runtime.h"
#include "cublas_v2.h"

int morn_cuda_device_count=0;
int current_cuda_device = MORN_HOST_CPU;
int morn_cuda_memory_time[8] = {0,0,0,0,0,0,0,0};
cublasHandle_t morn_cublas_handle[8]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

void CUDADevice(int dev)
{
    int order = dev&0x0000FFFF;
    int type  = dev&0xFFFF0000;
    mException(type!=MORN_CUDA_GPU(0),EXIT,"invalid device dev");
    int ret;
    if(morn_cuda_device_count==0) 
    {
        ret = cudaGetDeviceCount(&morn_cuda_device_count);
        mException((ret!=cudaSuccess)||(morn_cuda_device_count<=0),EXIT,"cannot get cuda device");
    }
    mException(order>=MIN(morn_cuda_device_count,8),EXIT,"invalid  device");
    cudaSetDevice(order);
    current_cuda_device = dev;
}

void *CUDAMalloc(int size,int dev)
{
    if(current_cuda_device != dev) CUDADevice(dev);
    void *p;cudaMalloc(&p,size);
    mException(p==NULL,EXIT,"error with cudaMalloc");
    morn_cuda_memory_time[dev&0x0000FFFF]++;
    return p;
}
void CUDAFree(void *ptr,int dev)
{
    if(current_cuda_device != dev) CUDADevice(dev);
    cudaFree(ptr);
    int order = dev&0x0000FFFF;
    morn_cuda_memory_time[order]--;
    mException(morn_cuda_memory_time[order]<0,EXIT,"invalid memory free");
    if(morn_cuda_memory_time[order]==0) cublasDestroy(morn_cublas_handle[order]);
}

void CUDAWrite(void *ptr,void *data,int size,int dev)
{
    if(current_cuda_device != dev) CUDADevice(dev);
    cudaMemcpy(ptr,data,size,cudaMemcpyHostToDevice);
}
void CUDARead(void *ptr,void *data,int size,int dev)
{
    if(current_cuda_device != dev) CUDADevice(dev);
    cudaMemcpy(data,ptr,size,cudaMemcpyDeviceToHost);
}
void CUDACopy(void *dst,void *src,int size,int dev)
{
    if(current_cuda_device != dev) CUDADevice(dev);
    cudaMemcpy(data,ptr,size,cudaMemcpyDeviceToDevice);
}


#else
#define CUDAMalloc(Size,dev) NULL
#define CUDAFree(Ptr,dev) mException(1,EXIT,"CL is invalid")
#define CUDAWrite(Ptr,Data,Size,dev) mException(1,EXIT,"CUDA is invalid")
#define CUDARead(Ptr,Data,Size,dev) mException(1,EXIT,"CUDA is invalid")
#define CUDACopy(Dst,Src,Size,dev) mException(1,EXIT,"CUDA is invalid")
#endif

#if defined MORN_USE_CL
#include <cl/cl.h>
#include <clBLAS.h>

int morn_cl_memory_time[12]       = {0,0,0,0,0,0,0,0,0,0,0,0};
int morn_cl_blas_flag[12]         = {0,0,0,0,0,0,0,0,0,0,0,0};
cl_context morn_cl_ctx[12]        = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
cl_command_queue morn_cl_queue[12]= {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
cl_event morn_cl_event[12]        = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

void CLDevice(int dev)
{
    int idx= dev&0x0000FFFF;
    mException(idx>4,EXIT,"invalid input device type");
    int type; dev=dev&0xFFFF0000;
         if(dev==        MORN_CL_CPU(0)) {type = CL_DEVICE_TYPE_CPU;        }
    else if(dev==        MORN_CL_GPU(0)) {type = CL_DEVICE_TYPE_GPU;        }
    else if(dev==MORN_CL_ACCELERATOR(0)) {type = CL_DEVICE_TYPE_ACCELERATOR;}
    else mException(1,EXIT,"invalid device type");
    
    unsigned int n,m;int i;
    cl_platform_id platform[32];
    cl_device_id device[32];
    
    int ret = clGetPlatformIDs(32,platform,&n);
    mException((ret!=CL_SUCCESS)||(n==0),EXIT,"cannot get device platform");
    for(i=0;i<n;i++)
    {
        ret = clGetDeviceIDs(platform[i],type,32,device,&m);
        if((ret==CL_SUCCESS)&&(m>idx)) break;
    }
    mException(i==n,EXIT,"cannot get device platform");
    
    int order = ((dev>>16)-2)*4+idx;

    morn_cl_ctx[order] = clCreateContext(NULL,1,&(device[idx]),NULL, NULL, &ret);
    mException((ret!=CL_SUCCESS)||(morn_cl_ctx[order]==NULL),EXIT,"cannot get device context");
    morn_cl_queue[order] = clCreateCommandQueue(morn_cl_ctx[order],device[idx],0,&ret);
    mException((ret!=CL_SUCCESS)||(morn_cl_queue[order]==NULL),EXIT,"cannot get device command queue");

    morn_cl_event[order]=clCreateUserEvent(morn_cl_ctx[order],&ret);
    mException((ret!=CL_SUCCESS)||(morn_cl_event[order]==NULL),EXIT,"cannot create user event");
    ret = clSetUserEventStatus(morn_cl_event[order],CL_COMPLETE);
    mException((ret!=CL_SUCCESS),EXIT,"cannot create user event");
}

void *CLMalloc(int size,int dev)
{
    int order = ((dev>>16)-2)*4+(dev&0x0000FFFF);
    int ret;
    if(morn_cl_ctx[order] == NULL) CLDevice(dev);
    void *ptr = (void *)clCreateBuffer(morn_cl_ctx[order],CL_MEM_READ_WRITE,size,NULL,&ret);
    mException((ret!=CL_SUCCESS),EXIT,"cannot create user event");
    morn_cl_memory_time[order]++;
    return ptr;
}
void CLFree(void *ptr,int dev)
{
    int order = ((dev>>16)-2)*4+(dev&&0x0000FFFF);
    clReleaseMemObject((cl_mem)ptr);
    morn_cl_memory_time[order]--;
    mException(morn_cl_memory_time[order]<0,EXIT,"invalid memory free");
    if(morn_cl_memory_time[order]==0)
    {
        if(morn_cl_blas_flag[order]) clblasTeardown();
        if(morn_cl_event[order]!=NULL) {clReleaseEvent(morn_cl_event[order]);       morn_cl_event[order]=NULL;}
        if(morn_cl_queue[order]!=NULL) {clReleaseCommandQueue(morn_cl_queue[order]);morn_cl_queue[order]=NULL;}
        if(morn_cl_ctx[order]  !=NULL) {clReleaseContext(morn_cl_ctx[order]);       morn_cl_ctx[order]  =NULL;}
    }
}
void CLWrite(void *ptr,void *data,int size,int dev)
{
    int order = ((dev>>16)-2)*4+(dev&&0x0000FFFF);
    cl_int ret =clEnqueueWriteBuffer(morn_cl_queue[order],(cl_mem)ptr,CL_TRUE,0,size,data,0,NULL,NULL);
    mException((ret!=CL_SUCCESS),EXIT,"error when write data to opencl device");
}
void CLRead(void *ptr,void *data,int size,int dev)
{
    int order = ((dev>>16)-2)*4+(dev&&0x0000FFFF);
    cl_int ret = clEnqueueReadBuffer(morn_cl_queue[order],(cl_mem)ptr,CL_TRUE,0,size,data,0,NULL,NULL);
    mException((ret!=CL_SUCCESS),EXIT,"error when read data from opencl device");
}
void CLCopy(void *dst,void *src,int size,int dev)
{
    int order = ((dev>>16)-2)*4+(dev&&0x0000FFFF);
    cl_int ret = clEnqueueCopyBuffer(orn_cl_queue[order],(cl_mem)src,(cl_mem)dst,0,0,size,0,NULL,NULL);
    mException((ret!=CL_SUCCESS),EXIT,"error when copy data with opencl device");
}

#else
#define CLMalloc(Size,dev) NULL
#define CLFree(Ptr,dev) mException(1,EXIT,"OpenCL is invalid")
#define CLWrite(Ptr,Data,Size,dev) mException(1,EXIT,"OpenCL is invalid")
#define CLRead(Ptr,Data,Size,dev) mException(1,EXIT,"OpenCL is invalid")
#define CLCopy(Dst,Src,Size,dev) mException(1,EXIT,"OpenCL is invalid")
#endif

void *MemAlloc(int size,int dev)
{
    void *p=NULL;
    if(dev==MORN_HOST_CPU) {return mMalloc(size+16);}
    int ram = dev&0xFFFF0000;
         if(ram==MORN_CUDA_GPU(0))       p=CUDAMalloc(size,dev);
    else if(ram==MORN_CL_CPU(0))         p=  CLMalloc(size,dev);
    else if(ram==MORN_CL_GPU(0))         p=  CLMalloc(size,dev);
    else if(ram==MORN_CL_ACCELERATOR(0)) p=  CLMalloc(size,dev);
    mException(p==NULL,EXIT,"invalid device");
    return p;
}
void MemFree(void *ptr,int dev)
{
    if(dev==MORN_HOST_CPU) {mFree(ptr);return;}
    int ram = dev&0xFFFF0000;
         if(ram==MORN_CUDA_GPU(0))     CUDAFree(ptr,dev);
    else if(ram==MORN_CL_CPU(0))         CLFree(ptr,dev);
    else if(ram==MORN_CL_GPU(0))         CLFree(ptr,dev);
    else if(ram==MORN_CL_ACCELERATOR(0)) CLFree(ptr,dev);
    else mException(1,EXIT,"invalid device");
}

void MemCopy(void *dst,int dst_dev,void *src,int src_dev,int size)
{
    if((src_dev==MORN_HOST_CPU)&&(dst_dev==MORN_HOST_CPU))
        {memcpy(dst,src,size);return;}
    if(src_dev==dst_dev)
    {
        int ram = dst_dev&0xFFFF0000;
             if(ram==MORN_CUDA_GPU(0))     CUDACopy(dst,src,size,dst_dev);
        else if(ram==MORN_CL_CPU(0))         CLCopy(dst,src,size,dst_dev);
        else if(ram==MORN_CL_GPU(0))         CLCopy(dst,src,size,dst_dev);
        else if(ram==MORN_CL_ACCELERATOR(0)) CLCopy(dst,src,size,dst_dev);
        else mException(1,EXIT,"invalid device");
        return;
    }
    if(src_dev==MORN_HOST_CPU)
    {
        int ram = dst_dev&0xFFFF0000;
             if(ram==MORN_CUDA_GPU(0))     CUDAWrite(dst,src,size,dst_dev);
        else if(ram==MORN_CL_CPU(0))         CLWrite(dst,src,size,dst_dev);
        else if(ram==MORN_CL_GPU(0))         CLWrite(dst,src,size,dst_dev);
        else if(ram==MORN_CL_ACCELERATOR(0)) CLWrite(dst,src,size,dst_dev);
        else mException(1,EXIT,"invalid device");
        return;
    }
    if(dst_dev==MORN_HOST_CPU)
    {
        int ram = src_dev&0xFFFF0000;
             if(ram==MORN_CUDA_GPU(0))     CUDARead(src,dst,size,src_dev);
        else if(ram==MORN_CL_CPU(0))         CLRead(src,dst,size,src_dev);
        else if(ram==MORN_CL_GPU(0))         CLRead(src,dst,size,src_dev);
        else if(ram==MORN_CL_ACCELERATOR(0)) CLRead(src,dst,size,src_dev);
        else mException(1,EXIT,"invalid device");
        return;
    }
    void *buff = mMalloc(size);
    MemCopy(buff,MORN_HOST_CPU,src,src_dev,size);
    MemCopy(dst,dst_dev,buff,MORN_HOST_CPU,size);
    mFree(buff);
}

struct HandleMemory
{
    int mem_device;
    int mem_num;
    int *mem_size;
    
    int write_idx;
    char *write_pdata;
    int write_size;

    int collect_num;
    int *collect_valid;
};
void endMemory(void *info)
{
    struct HandleMemory *handle = (struct HandleMemory *)info;
    if(handle->mem_size!=NULL) mFree(handle->mem_size);

    if(handle->collect_valid!=NULL) mFree(handle->collect_valid);
}
#define HASH_Memory 0x25793220
MMemory *mMemoryCreate(int num,int size,int dev)
{
    MMemory *memory = (MMemory *)mMalloc(sizeof(MMemory));
    memset(memory,0,sizeof(MMemory));

    memory->handle = mHandleCreate();
    MHandle *hdl=mHandle(memory,Memory);hdl->valid = 1;
    
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    if(dev<0) dev = MORN_HOST_CPU;
    handle->mem_device = dev;
    
    if(num<0) {mException(size>0,EXIT,"invalid input"); return memory;}
    handle->mem_num = num;
    memory->num = handle->mem_num;
    
    memory->data = (void **)mMalloc(num*sizeof(void*));
    handle->mem_size=(int *)mMalloc(num*sizeof(int  ));
    if(size<0)
    {
        memset(memory->data    ,0,num*sizeof(void*));
        memset(handle->mem_size,0,num*sizeof(int  ));
        return memory;
    }

    size=((size+15)>>4)<<4;
    for(int i=0;i<num;i++)
    {
        memory->data[i]=MemAlloc(size,dev);
        handle->mem_size[i]=size;
    }
    
    return memory;
}

void mMemoryRelease(MMemory *memory)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    struct HandleMemory *handle = ((MHandle *)(memory->handle->data[0]))->handle;
    int dev = handle->mem_device;
    
    for(int i=0;i<handle->mem_num;i++)
        if(memory->data[i]!=NULL) MemFree(memory->data[i],dev);
        
    if(!INVALID_POINTER(memory->handle))
        mHandleRelease(memory->handle);
    
    if(memory->data!=NULL) mFree(memory->data);
    mFree(memory);
}

void mMemoryRedefine(MMemory *memory,int num,int size,int dev)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    
    if(num<=0) num=memory->num;
    if(num<=0) return;
    
    if(dev< 0) dev=handle->mem_device;
    if(num>handle->mem_num)
    {
        int n = MAX(handle->mem_num+8,num);
        handle->mem_num = n;
        
        void **data_buff=mMalloc(n*sizeof(void **));
        if(memory->num>0) memcpy(data_buff,memory->data,memory->num*sizeof(void *));
        memset(data_buff+memory->num,0,(n-memory->num)*sizeof(void *));
        memory->data = data_buff;

        int *size_buff = mMalloc(n*sizeof(int));
        if(memory->num>0) memcpy(size_buff,handle->mem_size,memory->num*sizeof(int));
        memset(size_buff+memory->num,0,(n-memory->num)*sizeof(int));
        handle->mem_size = size_buff;
    }
    memory->num = num;
    if((size<=0)&&(dev==handle->mem_device)) return;
    
    for(int i=0;i<num;i++)
    {
        int s = size;if(s<=0) {s=handle->mem_size[i];}
        if((dev==handle->mem_device)&&(handle->mem_size[i]>=s)) continue;
        if(memory->data[i]!=NULL) {MemFree(memory->data[i],dev);memory->data[i]=NULL;}
        if(s>0) memory->data[i]=MemAlloc(s,dev);
        handle->mem_size[i]=s;
    }
}

void *mMemoryAppend(MMemory *memory,int size)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    mException((size<=0),EXIT,"invalid input");
    
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    int dev = handle->mem_device;
    
    int memory_num = memory->num;
    mMemoryRedefine(memory,memory_num+1,DFLT,DFLT);
    memmove(memory->    data+1,memory->    data,memory_num*sizeof(void *));
    memmove(handle->mem_size+1,handle->mem_size,memory_num*sizeof(int   ));
    
    if(handle->write_pdata==NULL)handle->write_idx+=1;

    size = ((size+15)>>4)<<4;
    memory->    data[0] = MemAlloc(size,dev);
    handle->mem_size[0] = size;
    return (memory->data[0]);
}

void mMemoryDevice(MMemory *memory,int dev,void ***index,int batch,int row,int col)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    
    if(dev!=handle->mem_device)
    {
        for(int i=0;i<memory->num;i++)
        {
            void *data=MemAlloc(handle->mem_size[i],dev);
            MemCopy(data,dev,memory->data[i],handle->mem_device,handle->mem_size[i]);
            MemFree(memory->data[i],handle->mem_device);
            memory->data[i] = data;
        }
        handle->mem_device=dev;
    }
    if(index!=NULL) mMemoryIndex(memory,row,col,index,batch);
}

int mMemorySize(MMemory *memory)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    int size = 0;for(int i=0;i<memory->num;i++)size = size + handle->mem_size[i];
    return size;
}

int mMemoryCheck(MMemory *memory,void *check)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    struct HandleMemory *handle = ((MHandle *)(memory->handle->data[0]))->handle;
    
    for(int i=0;i<memory->num;i++)
    {
        if(check < memory->data[i])
            continue;
        if((char *)check >= (char *)(memory->data[i])+handle->mem_size[i])
            continue;
        return 1;
    }
    return 0;
}

void MemoryCollect(void *data,void *mem)
{
    MMemory *memory = (MMemory *)mem;
    struct HandleMemory *handle = ((MHandle *)(memory->handle->data[0]))->handle;
    if(handle->collect_num<memory->num) 
    {
        if(handle->collect_valid!=NULL) mFree(handle->collect_valid);
        handle->collect_valid = mMalloc(memory->num*sizeof(int));
        memset(handle->collect_valid,0,memory->num*sizeof(int));
        handle->collect_num = memory->num;
    }
    
    for(int i=0;i<memory->num;i++)
    {
        if(data>memory->data[i])
            if((char *)data<((char *)(memory->data[i]))+handle->mem_size[i])
                {handle->collect_valid[i]=1;break;}
    }
}
void MemoryDefrag(MMemory *memory)
{
    struct HandleMemory *handle = ((MHandle *)(memory->handle->data[0]))->handle;
    mException((handle->collect_num!=memory->num),EXIT,"invalid defrag memory");
    int n=0;
    for(int i=0;i<memory->num;i++)
    {
        if(handle->collect_valid[i]==1)
        {
            memory->data[n]=memory->data[i];
            handle->mem_size[n]=handle->mem_size[i];
            n++;
        }
        else MemFree(memory->data[i],handle->mem_device);
    }
    memory->num = n;
    memset(handle->collect_valid,0,memory->num*sizeof(int));
}

/*
struct HandleMemoryCollect
{
    int collect_num;
    void **collect_data;
    int *collect_size;
};
void endMemoryCollect(void *info)
{
    struct HandleMemoryCollect *handle = (struct HandleMemoryCollect *)info;
    if(handle->collect_data!=NULL) mFree(handle->collect_data);
    if(handle->collect_size!=NULL) mFree(handle->collect_size);
}
#define HASH_MemoryCollect 0xc53f340e
void MemoryCollect(void *data,void *mem)
{
    MMemory *memory = (MMemory *)mem;
    struct HandleMemory *handle0 = ((MHandle *)(memory->handle->data[0]))->handle;
    MHandle *hdl = mHandle(memory,MemoryCollect);
    struct HandleMemoryCollect *handle = (struct HandleMemoryCollect *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->collect_data!=NULL) mFree(handle->collect_data);
        handle->collect_data  = (void **)mMalloc(memory->num*sizeof(void *));
        memcpy(handle->collect_data,memory->data,memory->num*sizeof(void *));
        
        if(handle->collect_size!=NULL) mFree(handle->collect_size);
        handle->collect_size = (int *)mMalloc(memory->num*sizeof(int));
        memcpy(handle->collect_size,handle0->mem_size,memory->num*sizeof(int));
        handle->collect_num = memory->num;
        hdl->valid = 1;
    }
    for(int i=0;i<handle->collect_num;i++)
    {
        void *mem_data = handle->collect_data[i];
        int size = handle->collect_size[i];
        if((data>=mem_data)&&(data<mem_data+size))
        {
            handle->collect_num = handle->collect_num-1;
            memmove(handle->collect_data+i,handle->collect_data+i+1,(handle->collect_num-i)*sizeof(void *));
            memmove(handle->collect_size+i,handle->collect_data+i+1,(handle->collect_num-i)*sizeof(int   ));
            break;
        }
    }
}
void MemoryDefrag(MMemory *memory)
{
    struct HandleMemory *handle0 = ((MHandle *)(memory->handle->data[0]))->handle;
    int dev = handle0->mem_device;
    
    MHandle *hdl = mHandle(memory,MemoryCollect);
    mException((hdl ->valid == 0),EXIT,"invalid input");
    struct HandleMemoryCollect *handle = (struct HandleMemoryCollect *)(hdl->handle);
    if(handle->collect_num==0) return;
    
    int n=0;
    for(int i=0;i<memory->num;i++)
    {
        int j;for(j=0;j<handle->collect_num;j++)if(memory->data[i]==handle->collect_data[j]) break;
        if(j==handle->collect_num) 
        {
            memory->data[n]=memory->data[j];
            handle0->mem_size[n]=handle0->mem_size[j];
            n++;
        }
        else MemFree(memory->data[j],dev);
    }
    memory->num = n;
}
*/

void mMemoryCopy(MMemory *src,void ***isrc,MMemory *dst,void ***idst,int batch,int *num)
{
    mException(INVALID_POINTER(src)||INVALID_POINTER(dst)||(dst==src),EXIT,"invalid input");
    mMemoryRedefine(dst,src->num,DFLT,DFLT);
    struct HandleMemory *handle1 = ((MHandle *)(src->handle->data[0]))->handle;
    struct HandleMemory *handle2 = ((MHandle *)(dst->handle->data[0]))->handle;
    int dev1=handle1->mem_device;int dev2=handle2->mem_device;
    for(int i=0;i<src->num;i++)
    {
        int size = handle1->mem_size[i];
        if((dev1==dev2)&&(handle2->mem_size[i]==size)) continue;
        MemFree(dst->data[i],dev2);
        handle2->mem_size[i]=size;
        dst->data[i] = MemAlloc(size,dev2);
        MemCopy(dst->data[i],dev2,src->data[i],dev1,size);
    }

    for(int j=0;j<batch;j++)for(int i=0;i<num[j];i++)
    {
        char *p = (char *)(isrc[j][i]);
        int n;for(n=0;n<src->num;n++)
        {
            if((p>=(char *)(src->data[n]))&&(p<((char *)(src->data[n]))+handle1->mem_size[n]))
            {
                idst[j][i]=((char *)(dst->data[n])+(int)(p-(char *)(src->data[n])));
                break;
            }
        }
        if(n==src->num)
        {
            mException((dev1!=dev2),EXIT,"invalid memory copy");
            idst[j][i]=p;
        }
    }
}

void mMemoryMerge(MMemory *memory1,MMemory *memory2,MMemory *dst)
{
    mException((memory1==NULL)||(memory2==NULL)||(dst==memory2),EXIT,"invalid input");

    struct HandleMemory *handle;
    struct HandleMemory *handle1 = ((MHandle *)(memory1->handle->data[0]))->handle;
    struct HandleMemory *handle2 = ((MHandle *)(memory2->handle->data[0]))->handle;
    mException(handle1->mem_device!=handle2->mem_device,EXIT,"source memory with different device");
    int num1 = memory1->num;
    int num2 = memory2->num;
    
    mMemoryRedefine(dst,num1+num2,DFLT,DFLT);

    if(dst==memory1) handle=handle1;
    else
    {
        handle  = ((MHandle *)(dst->handle->data[0]))->handle;
        for(int i=0;i<dst->num;i++) if(dst->data[i]!=NULL) MemFree(dst->data[i],handle->mem_device);
        memset(handle->mem_size,0,handle->mem_num*sizeof(int));
        dst->num=0;
        handle->mem_device = handle1->mem_device;

        if(num1>0)
        {
            memcpy(dst->data,memory1->data,num1*sizeof(void *));
            memset(memory1->data,0,num1*sizeof(void *));
            memcpy(handle->mem_size,handle1->mem_size,num1*sizeof(int));
            memset(handle1->mem_size,0,num1*sizeof(int));
            memory1->num=0;
        }
    }
    
    if(num2>0)
    {
        memcpy(dst->data+num1,memory2->data,num2*sizeof(void *));
        memset(memory2->data,0,num2*sizeof(void *));
        memcpy(handle->mem_size+num1,handle2->mem_size,num2*sizeof(int));
        memset(handle2->mem_size,0,num2*sizeof(int));
        memory2->num=0;
    }
}

void mMemoryIndex(MMemory *memory,int row,int col_size,void ***index,int num)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    mException(((row<=0)||(col_size<=0)),EXIT,"invalid input");
    mException(INVALID_POINTER(index),EXIT,"invalid input");
    
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    if(num<=0) num=memory->num;
    
    for(int j=0;j<num;j++)
    {
        mException((row*col_size>handle->mem_size[j]),EXIT,"invalid memory index,with row=%d,col_size=%d,memory size=%d",row,col_size,handle->mem_size[j]);
        index[j][0] = memory->data[j];
        for(int i=1;i<row;i++) index[j][i]=((char *)(index[j][i-1]))+col_size;
    }
}

void *mMemoryWrite(MMemory *memory,void *data,int size)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    if(size<0){mException(data==NULL,EXIT,"invalid input data");size=strlen(data);}
    
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    int dev = handle->mem_device;

    if(memory->num==0) mMemoryRedefine(memory,1,MAX(8192,(size<<6)),dev);
    
    if(handle->write_pdata==NULL)
    {
        handle->write_idx=0;
        handle->write_pdata=memory->data[0];
        handle->write_size=handle->mem_size[0];
    }
    mException((handle->write_idx>=memory->num),EXIT,"invalid write memory");

    MemoryWrite_Check:
    if(handle->write_size < size)
    {
        int idx = handle->write_idx+1;
        handle->write_idx = idx;
        if(idx>=memory->num)
        {
            mMemoryRedefine(memory,idx+1,DFLT,DFLT);
            int s = MAX(8192,(size<<6));
            memory->data[idx] = MemAlloc(s,dev);
            handle->mem_size[idx] = s;
        }
        
        handle->write_pdata=memory->data[idx];
        handle->write_size=handle->mem_size[idx];
        goto MemoryWrite_Check;
    }

    if(data!=NULL) MemCopy(handle->write_pdata,dev,data,MORN_HOST_CPU,size);
    
    char *memory_data = handle->write_pdata;
    handle->write_pdata= handle->write_pdata+ size;
    handle->write_size = handle->write_size - size;
    
    return ((void *)memory_data);
}
