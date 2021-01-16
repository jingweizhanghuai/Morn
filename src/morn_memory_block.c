/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_util.h"
#ifdef MORN_USE_CL
#include <clBLAS.h>
#endif
/*
#if defined MORN_USE_CUDA
#include "cuda_runtime.h"
#include "cublas_v2.h"

int morn_cuda_device_count=0;
int current_cuda_device = MORN_HOST;
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
*/

struct MemoryData
{
    void *data;
    #ifdef MORN_USE_CL
    cl_mem cl_data;
    #endif
    int mem_size;
    int dev;
    int flag;
};

#ifdef MORN_USE_CL
struct HandleCLMemory
{
    int device;
    int dev;
    cl_context         cl_ctx[13];// 1 host+12 device
    cl_command_queue cl_queue[13];
    cl_event         cl_event[13];
};
void endCLMemory(struct HandleCLMemory *handle)
{
    for(int i=1;i<13;i++)
    {
        if(handle->cl_ctx[i]!=NULL)
        {
            clReleaseEvent(handle->cl_event[i]);
            clReleaseCommandQueue(handle->cl_queue[i]);
            clReleaseContext(handle->cl_ctx[i]);
        }
    }
}
#define HASH_CLMemory 0x65265f97
struct HandleCLMemory *morn_cl_memory_handle=NULL;

int m_DeviceOrder(int device)
{
    if(device==MORN_HOST) return 0;
    if(morn_cl_memory_handle ==NULL)
    {
        MHandle *hdl = mHandle(mMornObject(NULL,DFLT),CLMemory);
        morn_cl_memory_handle = (struct HandleCLMemory *)(hdl->handle);
        morn_cl_memory_handle->device = DFLT;
        morn_cl_memory_handle->dev = DFLT;
    }
    struct HandleCLMemory *handle = morn_cl_memory_handle;
    if(device == handle->device) return handle->dev;
    
    int idx= device&0x0000FFFF;
    mException(idx>4,EXIT,"invalid input device type");
    device=device&0xFFFF0000;
    int type=CL_DEVICE_TYPE_GPU; 
         if(device==        MORN_CL_CPU(0)) {type = CL_DEVICE_TYPE_CPU;        }
    else if(device==        MORN_CL_GPU(0)) {type = CL_DEVICE_TYPE_GPU;        }
    else if(device==MORN_CL_ACCELERATOR(0)) {type = CL_DEVICE_TYPE_ACCELERATOR;}
    else mException(1,EXIT,"invalid device type");
    
    handle->dev = ((device>>16)-2)*4+idx+1;
    int order = handle->dev;

    if(handle->cl_ctx[order]==NULL)
    {
        unsigned int n,m;int i;
        cl_platform_id platform[32];
        cl_device_id cl_device[32];
        
        int ret = clGetPlatformIDs(32,platform,&n);
        mException((ret!=CL_SUCCESS)||(n==0),EXIT,"cannot get device platform");
        for(i=0;i<n;i++)
        {
            ret = clGetDeviceIDs(platform[i],type,32,cl_device,&m);
            if((ret==CL_SUCCESS)&&(m>idx)) break;
        }
        mException(i==n,EXIT,"cannot get device platform");
        cl_context_properties cps[3] = {CL_CONTEXT_PLATFORM,(cl_context_properties)platform[i],0};
    
        handle->cl_ctx[order] = clCreateContext(cps,1,&(cl_device[idx]),NULL, NULL, &ret);
        mException((ret!=CL_SUCCESS)||(handle->cl_ctx[order]==NULL),EXIT,"cannot get device context");

        handle->cl_queue[order] = clCreateCommandQueue(handle->cl_ctx[order],cl_device[idx],0,&ret);
        mException((ret!=CL_SUCCESS)||(handle->cl_queue[order]==NULL),EXIT,"cannot get device command queue");

        handle->cl_event[order]=clCreateUserEvent(handle->cl_ctx[order],&ret);
        mException((ret!=CL_SUCCESS)||(handle->cl_event[order]==NULL),EXIT,"cannot create user event");
        ret = clSetUserEventStatus(handle->cl_event[order],CL_COMPLETE);
        mException((ret!=CL_SUCCESS),EXIT,"cannot create user event");
    }
    return order;
}
#else
int m_DeviceOrder(int device)
{
    mException(device!=0,EXIT,"cannot find any device");
    return 0;
}
#endif

struct MemoryData *MemAlloc(int size,int dev)
{
    struct MemoryData *mem_data = malloc(sizeof(struct MemoryData)+size);
    mem_data->data = (void *)(mem_data+1);
    mem_data->mem_size = size;
    mem_data->dev = MORN_HOST;
    mem_data->flag= MORN_HOST;
    #ifdef MORN_USE_CL
    mem_data->cl_data = NULL;
    if(dev!=MORN_HOST)
    {
        int ret;
        mem_data->dev = dev;
        mem_data->cl_data=clCreateBuffer(morn_cl_memory_handle->cl_ctx[mem_data->dev],CL_MEM_READ_WRITE,size,NULL,&ret);
        mException((ret!=CL_SUCCESS),EXIT,"cannot create device memory");
    }
    #endif
    return mem_data;
}
void MemFree(struct MemoryData *mem_data)
{
    #ifdef MORN_USE_CL
    if(mem_data->cl_data!=NULL) clReleaseMemObject(mem_data->cl_data);
    #endif
    free(mem_data);
}
void MemDeviceWrite(struct MemoryData *mem_data)
{
    #ifdef MORN_USE_CL
    if(mem_data->dev ==MORN_HOST  ) return;
    if(mem_data->flag==MORN_DEVICE) return;
    cl_int ret =clEnqueueWriteBuffer(morn_cl_memory_handle->cl_queue[mem_data->dev],mem_data->cl_data,CL_TRUE,0,mem_data->mem_size,mem_data->data,0,NULL,NULL);
    mException((ret!=CL_SUCCESS),EXIT,"error when write data to opencl device");
    mem_data->flag = MORN_DEVICE;
    #endif
}
void MemDeviceRead(struct MemoryData *mem_data)
{
    #ifdef MORN_USE_CL
    if(mem_data->dev ==MORN_HOST) return;
    if(mem_data->flag==MORN_HOST) return;
    cl_int ret = clEnqueueReadBuffer(morn_cl_memory_handle->cl_queue[mem_data->dev],mem_data->cl_data,CL_TRUE,0,mem_data->mem_size,mem_data->data,0,NULL,NULL);
    mException((ret!=CL_SUCCESS),EXIT,"error when read data from opencl device");
    mem_data->flag = MORN_HOST;
    #endif
}

void MemDeviceCopy(struct MemoryData *mem_data,int dev)
{
    #ifdef MORN_USE_CL
    if(dev<0) dev=mem_data->dev;
    
    if(dev==MORN_HOST)    {MemDeviceRead (mem_data);return;}
    if(dev==mem_data->dev){MemDeviceWrite(mem_data);return;}

    MemDeviceRead (mem_data);
    
    if(mem_data->cl_data!=NULL) clReleaseMemObject(mem_data->cl_data);
    cl_int ret;
    mem_data->cl_data = clCreateBuffer(morn_cl_memory_handle->cl_ctx[dev],CL_MEM_READ_WRITE,mem_data->mem_size,NULL,&ret);
    mException((ret!=CL_SUCCESS),EXIT,"cannot create device memory");
    mem_data->dev = dev;

    MemDeviceWrite(mem_data);
    #endif
}

struct HandleMemory
{
    int dev;
    int dev_flag;
    int device;
    
    int mem_num;
    
    int write_idx;
    char *write_pdata;
    int write_size;

    int collect_num;
    int *collect_valid;
};
void endMemory(struct HandleMemory *handle)
{
    if(handle->collect_valid!=NULL) free(handle->collect_valid);
}
#define HASH_Memory 0x25793220
MMemory *mMemoryCreate(int num,int size,int device)
{
    MMemory *memory = (MMemory *)malloc(sizeof(MMemory));
    memset(memory,0,sizeof(MMemory));

    memory->handle = mHandleCreate();
    MHandle *hdl=mHandle(memory,Memory);hdl->valid = 1;
    if(num<0) {mException(size>0,EXIT,"invalid input"); return memory;}
    
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    if(device<0) device = MORN_CL_GPU(0);
    handle->device = device;
    handle->dev = m_DeviceOrder(device);
    handle->dev_flag = MORN_HOST;
    memory->device = MORN_HOST;
    
    handle->mem_num = num;
    memory->num = handle->mem_num;
    
    memory->data = (void **)malloc(num*sizeof(struct MemoryData *));
    if(size<0)
    {
        memset(memory->data,0,num*sizeof(struct MemoryData *));
        return memory;
    }

    size=((size+15)>>4)<<4;
    for(int i=0;i<num;i++) memory->data[i]=(void *)MemAlloc(size,handle->dev);
    
    return memory;
}

void mMemoryRelease(MMemory *memory)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    struct HandleMemory *handle = ((MHandle *)(memory->handle->data[0]))->handle;
    
    for(int i=0;i<handle->mem_num;i++)
        if(memory->data[i]!=NULL) MemFree((struct MemoryData *)(memory->data[i]));
    
    if(!INVALID_POINTER(memory->handle))
        mHandleRelease(memory->handle);
    
    if(memory->data!=NULL) free(memory->data);
    free(memory);
}

void mMemoryRedefine(MMemory *memory,int num,int size,int device)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    
    if(num<=0) {num=memory->num;} if(num<=0) {return;}
    if(device<0) {device=memory->device;} if(device>0) {handle->device=device;}
    int dev=device;if(device>0) {dev=m_DeviceOrder(device);}
    
    if(num>handle->mem_num)
    {
        int n = MAX(handle->mem_num+8,num);
        handle->mem_num = n;
        
        void **data_buff=malloc(n*sizeof(void **));
        if(memory->num>0) memcpy(data_buff,memory->data,memory->num*sizeof(void *));
        memset(data_buff+memory->num,0,(n-memory->num)*sizeof(void *));
        memory->data = data_buff;
    }
    memory->num = num;
    if((size<=0)&&(dev==handle->dev)) return;

    for(int i=0;i<num;i++)
    {
        struct MemoryData *mem_data = memory->data[i];
        int s = size;
        if(mem_data==NULL){if(s>0) {memory->data[i]=MemAlloc(s,dev);}}
        else
        {
            if(s<=0) s=mem_data->mem_size;
            if((s>mem_data->mem_size)||(mem_data->dev!=dev))
                {MemFree(mem_data);memory->data[i]=MemAlloc(s,dev);}
            else MemDeviceRead(mem_data);
        }
        
        // int mem_size = DFLT; if(mem_data!=NULL) mem_size = mem_data->mem_size;
        // printf("ffffffffffffmem_size=%d\n",mem_size);
        
        // if((handle->dev==mem_data->dev)&&(mem_size>=s)) {MemDeviceRead(mem_data);continue;}
        // printf("ffffffffffff\n");
        // if(memory->data[i]!=NULL) {MemFree(mem_data);memory->data[i]=NULL;}
        // if(s>0) memory->data[i]=MemAlloc(s,dev);
    }
    
    handle->dev = dev;
    handle->dev_flag = MORN_HOST;
    memory->device = MORN_HOST;
}

void *mMemoryAppend(MMemory *memory,int size)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    mException((size<=0),EXIT,"invalid input");
    
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    
    int memory_num = memory->num;
    mMemoryRedefine(memory,memory_num+1,DFLT,DFLT);
    memmove(memory->data+1,memory->data,memory_num*sizeof(struct MemoryData *));
    
    if(handle->write_pdata==NULL) handle->write_idx+=1;

    size = ((size+15)>>4)<<4;
    memory->data[0] = MemAlloc(size,handle->dev);
    
    for(int i=1;i<memory->num;i++) MemDeviceRead(memory->data[i]);
    handle->dev_flag = MORN_HOST;
    memory->device = MORN_HOST;
    return (((struct MemoryData *)(memory->data[0]))->data);
}

void mMemoryClear(MMemory *memory)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);

    struct MemoryData *p = memory->data[0];
    int size = p->mem_size;
    for(int i=1;i<handle->mem_num;i++)
    {
        struct MemoryData *mem_data = (struct MemoryData *)(memory->data[i]);
        if(mem_data->mem_size<=size) MemFree(mem_data);
        else {MemFree(p);p=mem_data;size=p->mem_size;}
    }
    memset(memory->data,0,(handle->mem_num)*sizeof(void *));
    memory->data[0]=p;
    memory->num = 1;
}

void mMemoryDevice(MMemory *memory,int device)
{
    #ifdef MORN_USE_CL
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);

    if(device<0) device=memory->device;
    if(device==MORN_HOST)
    {
        memory->device = MORN_HOST;
        if(handle->dev_flag==MORN_HOST) return;
        for(int i=0;i<memory->num;i++) MemDeviceRead((struct MemoryData *)(memory->data[i]));
        handle->dev_flag = MORN_HOST;
    }
    else if(device==handle->device)
    {
        memory->device = handle->device;
        if(handle->dev_flag==MORN_DEVICE) return;
        for(int i=0;i<memory->num;i++) MemDeviceWrite((struct MemoryData *)(memory->data[i]));
        handle->dev_flag = MORN_DEVICE;
    }
    else
    {
        int dev=device;if(device>0){dev=m_DeviceOrder(device);}
        for(int i=0;i<memory->num;i++) MemDeviceCopy((struct MemoryData *)(memory->data[i]),dev);
        handle->dev = dev;
        memory->device = device;
        handle->device = device;
        handle->dev_flag = MORN_DEVICE;
    }
    #endif
}

// void mMemoryDevice(MMemory *memory,int dev,void ***index,int batch,int row,int col)
// {
//     mException(INVALID_POINTER(memory),EXIT,"invalid input");
//     MHandle *hdl = (MHandle *)(memory->handle->data[0]);
//     mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
//     struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);
    
//     if(dev!=handle->mem_device)
//     {
//         for(int i=0;i<memory->num;i++)
//         {
//             void *data=MemAlloc(handle->mem_size[i],dev);
//             MemCopy(data,dev,memory->data[i],handle->mem_device,handle->mem_size[i]);
//             MemFree(memory->data[i],handle->mem_device);
//             memory->data[i] = data;
//         }
//         handle->mem_device=dev;
//     }
//     if(index!=NULL) mMemoryIndex(memory,row,col,index,batch);
// }

int mMemorySize(MMemory *memory)
{
    int size = 0;
    for(int i=0;i<memory->num;i++) size += ((struct MemoryData *)(memory->data[i]))->mem_size;
    return size;
}

int mMemoryCheck(MMemory *memory,void *check)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    for(int i=0;i<memory->num;i++)
    {
        struct MemoryData *mem_data = (struct MemoryData *)(memory->data[i]);
        if(check < mem_data->data) continue;
        if((char *)check >= (char *)(mem_data->data)+mem_data->mem_size)continue;
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
        if(handle->collect_valid!=NULL) free(handle->collect_valid);
        handle->collect_valid = malloc(memory->num*sizeof(int));
        memset(handle->collect_valid,0,memory->num*sizeof(int));
    }
    handle->collect_num=memory->num;
    
    for(int i=0;i<memory->num;i++)
    {
        struct MemoryData *mem_data = (struct MemoryData *)(memory->data[i]);
        if(data>=mem_data->data)
            if((char *)data<((char *)(mem_data->data))+mem_data->mem_size)
                {handle->collect_valid[i]=1;break;}
    }
}

void MemoryDefrag(MMemory *memory)
{
    struct HandleMemory *handle = ((MHandle *)(memory->handle->data[0]))->handle;
    mException((handle->collect_num!=memory->num),EXIT,"invalid defrag memory");
    // printf("hhhhhhhhdddddddddddddh2hhhhhandle->write_idx=%d,memory->num=%d\n",handle->write_idx,memory->num);
    handle->collect_valid[handle->write_idx] = 1;
    
    int n=0;
    for(int i=0;i<memory->num;i++)
    {
        if(handle->collect_valid[i]==1)
        {
            memory->data[n]=memory->data[i];
            if(i==handle->write_idx) handle->write_idx=n;
            n++;
        }
        else MemFree(memory->data[i]);
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
    int dev1=handle1->dev;int dev2=handle2->dev;
    for(int i=0;i<src->num;i++)
    {
        struct MemoryData *src_data = src->data[i];
        struct MemoryData *dst_data = dst->data[i];
        if(dst_data->mem_size<src_data->mem_size) {MemFree(dst_data);dst->data[i]=MemAlloc(src_data->mem_size,dev2);dst_data = dst->data[i];}
        
        MemDeviceRead(src_data);
        memcpy(dst_data->data,src->data,src_data->mem_size);
        
        dst_data->flag = MORN_HOST;
    }
    handle1->dev_flag = MORN_HOST;
    handle2->dev_flag = MORN_HOST;
    src->device = MORN_HOST;
    dst->device = MORN_HOST;

    for(int j=0;j<batch;j++)for(int i=0;i<num[j];i++)
    {
        char *p = (char *)(isrc[j][i]);
        int n;for(n=0;n<src->num;n++)
        {
            struct MemoryData *src_data = src->data[n];
            struct MemoryData *dst_data = dst->data[n];
            if((p>=(char *)(src_data->data))&&(p<((char *)(src_data->data))+src_data->mem_size))
            {
                idst[j][i]=((char *)(dst_data->data)+(int)(p-(char *)(src_data->data)));
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
    mException(handle1->dev!=handle2->dev,EXIT,"source memory with different device");
    if(handle1->dev_flag!=handle2->dev_flag) {mMemoryDevice(memory1,MORN_HOST);mMemoryDevice(memory2,MORN_HOST);}
    int num1 = memory1->num;
    int num2 = memory2->num;
    
    mMemoryRedefine(dst,num1+num2,DFLT,DFLT);
    if(dst==memory1) handle=handle1;
    else
    {
        handle  = ((MHandle *)(dst->handle->data[0]))->handle;
        for(int i=0;i<dst->num;i++) if(dst->data[i]!=NULL) MemFree(dst->data[i]);
        dst->num=0;
        handle->dev = handle1->dev;

        if(num1>0)
        {
            memcpy(dst->data,memory1->data,num1*sizeof(struct MemoryData *));
            memset(memory1->data,0,num1*sizeof(struct MemoryData *));
            memory1->num=0;
        }
    }
    
    if(num2>0)
    {
        memcpy(dst->data+num1,memory2->data,num2*sizeof(struct MemoryData *));
        memset(memory2->data,0,num2*sizeof(struct MemoryData *));
        memory2->num=0;
    }
}

void mMemoryIndex(MMemory *memory,int row,int col_size,void ***index,int num)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    mException(((row<=0)||(col_size<=0)),EXIT,"invalid input");
    mException(INVALID_POINTER(index),EXIT,"invalid input");
    
    if(num<=0) num=memory->num;
    
    for(int j=0;j<num;j++)
    {
        struct MemoryData *mem_data = (struct MemoryData *)(memory->data[j]);
        mException((row*col_size>mem_data->mem_size),EXIT,"invalid memory index,with row=%d,col_size=%d,memory size=%d",row,col_size,mem_data->mem_size);
        index[j][0] = mem_data->data;
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
    int dev = handle->dev;
    if(dev!=MORN_HOST) mMemoryDevice(memory,MORN_HOST);

    if(memory->num==0) mMemoryRedefine(memory,1,MAX(8192,(size<<6)),dev);
    
    if(handle->write_pdata==NULL)
    {
        struct MemoryData *mem_data =(struct MemoryData *)(memory->data[0]);
        handle->write_idx=0;
        handle->write_pdata=mem_data->data;
        handle->write_size =mem_data->mem_size;
    }
    mException((handle->write_idx>=memory->num),EXIT,"invalid write memory with write_idx=%d,memory->num=%d\n",handle->write_idx,memory->num);

    // printf("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhandle->write_idx=%d,memory->num=%d\n",handle->write_idx,memory->num);
    MemoryWrite_Check:
    if(handle->write_size < size)
    {
        int idx = handle->write_idx+1;handle->write_idx=idx;
        if(idx>=memory->num)
        {
            mMemoryRedefine(memory,idx+1,DFLT,DFLT);
            int s = MAX(MAX(8192,(size<<6)),((struct MemoryData *)(memory->data[idx-1]))->mem_size*2);
            memory->data[idx] = MemAlloc(s,dev);
        }
        struct MemoryData *mem_data =(struct MemoryData *)(memory->data[idx]);
        handle->write_pdata=mem_data->data;
        handle->write_size =mem_data->mem_size;
        goto MemoryWrite_Check;
    }
    
    if(data!=NULL) memcpy(handle->write_pdata,data,size);
    
    char *memory_data = handle->write_pdata;
    handle->write_pdata= handle->write_pdata+ size;
    handle->write_size = handle->write_size - size;
    
    return ((void *)memory_data);
}
