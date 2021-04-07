/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_util.h"
#ifdef MORN_USE_CL
#include <CL/cl.h>
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

#ifdef MORN_USE_CL
struct HandleCLMemory
{
    cl_context       cl_ctx  [192];
    cl_command_queue cl_queue[192];
    int device;
};
void endCLMemory(struct HandleCLMemory *handle)
{
    for(int i=1;i<192;i++)
    {
        if(handle->cl_ctx[i]!=NULL)
        {
            clReleaseCommandQueue(handle->cl_queue[i]);
            clReleaseContext(handle->cl_ctx[i]);
        }
    }
}
#define HASH_CLMemory 0x65265f97
struct HandleCLMemory *morn_cl_memory_handle=NULL;

void DeviceSetup(int device)
{
    if(device==MORN_HOST) return;
    mException(device>192,EXIT,"invalid device %d\n",device);
    if(morn_cl_memory_handle ==NULL)
    {
        MHandle *hdl = mHandle(mMornObject(NULL,DFLT),CLMemory);
        morn_cl_memory_handle = (struct HandleCLMemory *)(hdl->handle);
        morn_cl_memory_handle->device = DFLT;
    }
    struct HandleCLMemory *handle = morn_cl_memory_handle;
    if(device == handle->device) return;
    
    int idx= device&0x0000003F;
    int dev= device&0xFFFFFFC0;
    
    int type=CL_DEVICE_TYPE_GPU; 
         if(dev==        MORN_CL_CPU(0)) {type = CL_DEVICE_TYPE_CPU;        }
    else if(dev==        MORN_CL_GPU(0)) {type = CL_DEVICE_TYPE_GPU;        }
    else if(dev==MORN_CL_ACCELERATOR(0)) {type = CL_DEVICE_TYPE_ACCELERATOR;}
    else mException(1,EXIT,"invalid device type");
    
    handle->device = device;

    if(handle->cl_ctx[device]==NULL)
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

        handle->cl_ctx[device] = clCreateContext(cps,1,&(cl_device[idx]),NULL, NULL, &ret);
        mException((ret!=CL_SUCCESS)||(handle->cl_ctx[device]==NULL),EXIT,"cannot get device context");

        handle->cl_queue[device] = clCreateCommandQueue(handle->cl_ctx[device],cl_device[idx],0,&ret);
        mException((ret!=CL_SUCCESS)||(handle->cl_queue[device]==NULL),EXIT,"cannot get device command queue");
    }
}

cl_context mDeviceContext(int device)
{
    mException(morn_cl_memory_handle==NULL||(device<=0)||(device>=192),EXIT,"invalid device");
    return morn_cl_memory_handle->cl_ctx[device];
}
cl_command_queue mDeviceQueue(int device)
{
    mException(morn_cl_memory_handle==NULL||(device<=0)||(device>=192),EXIT,"invalid device");
    return morn_cl_memory_handle->cl_queue[device];
}

#else
void DeviceSetup(int device)
{
    mException(device!=MORN_HOST,EXIT,"cannot find any device");
}
#endif

MMemoryBlock *mMemoryBlockCreate(int size,int device)
{
    MMemoryBlock *block = malloc(sizeof(MMemoryBlock)+size);
    block->data = (void *)(block+1);
    block->size = size;
    block->device = MORN_HOST;
    #ifdef MORN_USE_CL
    block->flag= MORN_HOST;
    block->cl_data = NULL;
    block->cl_evt  = NULL;
    if(device!=MORN_HOST)
    {
        DeviceSetup(device);
        block->device = device;
        int ret;
        block->cl_data=clCreateBuffer(morn_cl_memory_handle->cl_ctx[device],CL_MEM_READ_WRITE,size,NULL,&ret);
        mException((ret!=CL_SUCCESS),EXIT,"cannot create device memory");
        block->cl_evt =clCreateUserEvent(morn_cl_memory_handle->cl_ctx[device],&ret);
        
        mException((ret!=CL_SUCCESS)||(block->cl_evt==NULL),EXIT,"cannot create user event");
        ret = clSetUserEventStatus(block->cl_evt,CL_COMPLETE);
        mException((ret!=CL_SUCCESS),EXIT,"cannot create user event");
    }
    #endif
    return block;
}
void mMemoryBlockRelease(MMemoryBlock *block)
{
    #ifdef MORN_USE_CL
    if(block->cl_data!=NULL) clReleaseMemObject(block->cl_data);
    if(block->cl_evt !=NULL) clReleaseEvent(block->cl_evt);
    #endif
    free(block);
}
MMemoryBlock *mMemoryBlock(void *data)
{
    MMemoryBlock *block = ((MMemoryBlock *)data)-1;
    mException(block->data!=data,EXIT,"invalid memory block");
    return block;
}

#ifdef MORN_USE_CL
void mMemoryBlockWrite(MMemoryBlock *block)
{
    if(block->device==MORN_HOST  ) return;
    if(block->flag  ==MORN_DEVICE) return;
    
    cl_event event = block->cl_evt;
    cl_int ret =clEnqueueWriteBuffer(morn_cl_memory_handle->cl_queue[block->device],block->cl_data,CL_TRUE,0,block->size,block->data,0,NULL,&event);
    mException((ret!=CL_SUCCESS),EXIT,"error%d when write data to opencl device",ret);
    block->flag = MORN_DEVICE;
}
void mMemoryBlockRead(MMemoryBlock *block)
{
    if(block->device==MORN_HOST) return;
    if(block->flag  ==MORN_HOST) return;
    // printf("block->size=%d\n",block->size);
    
    cl_event event = block->cl_evt;
    cl_int ret = clEnqueueReadBuffer(morn_cl_memory_handle->cl_queue[block->device],block->cl_data,CL_TRUE,0,block->size,block->data,0,NULL,&event);
    mException((ret!=CL_SUCCESS),EXIT,"error%d when read data from opencl device",ret);
    block->flag = MORN_HOST;
}
void mMemoryBlockCopy(MMemoryBlock *block,int device)
{
    if(device<0) device=block->device;

    if(device==MORN_HOST)    {mMemoryBlockRead (block);return;}
    if(device==block->device){mMemoryBlockWrite(block);return;}

    mMemoryBlockRead(block);
    
    if(block->cl_data!=NULL) clReleaseMemObject(block->cl_data);
    cl_int ret;
    DeviceSetup(device);
    block->cl_data = clCreateBuffer(morn_cl_memory_handle->cl_ctx[device],CL_MEM_READ_WRITE,block->size,NULL,&ret);
    mException((ret!=CL_SUCCESS),EXIT,"cannot create device memory");
    block->device = device;

    mMemoryBlockWrite(block);
}

static __thread MMemoryBlock *morn_cl_memory_block=NULL;
static __thread cl_event morn_cl_function_event[16];
static __thread int morn_cl_function_event_num=0;
__thread void *morn_cl_function_para[16];
__thread int morn_cl_function_para_size[16];
__thread int morn_cl_function_para_num=0;
__thread size_t morn_cl_size[4];
__thread int morn_cl_dim;

struct HandleCLFunction
{
    cl_program program;
    cl_kernel kernel;
};
void endCLFunction(struct HandleCLFunction *handle)
{
    if(handle->kernel ) clReleaseKernel( handle->kernel );
    if(handle->program) clReleaseProgram(handle->program);
}
#define HASH_CLFunction 0x1c9ebdd4
void CLFunction(const char *source,const char *name,int para_num,void **para,int *para_size)
{
    int ret;
    mException(INVALID_POINTER(source),EXIT,"invalid input");
    mException(morn_cl_memory_block==NULL,EXIT,"invalid input");
    
    int device    = morn_cl_memory_block->device;
    cl_event event= morn_cl_memory_block->cl_evt;
    // printf("morn_cl_dim=%d,morn_cl_size=%d,%d,%d,%d\n",morn_cl_dim,morn_cl_size[0],morn_cl_size[1],morn_cl_size[2],morn_cl_size[3]);
    cl_command_queue queue=mDeviceQueue(device);
    
    MHandle *hdl = mHandle(mMornObject((void *)source,DFLT),CLFunction);
    struct HandleCLFunction *handle = (struct HandleCLFunction *)(hdl->handle);
    if(hdl->valid==0)
    {
        cl_context context=mDeviceContext(device);

        if(handle->program) clReleaseProgram(handle->program);
        handle->program = clCreateProgramWithSource(context,1,&source,NULL,&ret);
        mException((handle->program==NULL)||(ret!=CL_SUCCESS),EXIT,"invalid input cl program source");

        ret = clBuildProgram(handle->program,0,NULL,NULL,NULL,NULL);
        mException((ret!=CL_SUCCESS),EXIT,"invalid input cl program source");

        // printf("name=%s\n",name);
        if(handle->kernel) clReleaseKernel(handle->kernel);
        handle->kernel = clCreateKernel(handle->program,name,NULL);
        mException(handle->kernel==NULL,EXIT,"invalid input cl program");
        
        hdl->valid = 1;
    }

    // printf("para_num=%d\n",para_num);
    for(int i=0;i<para_num;i++)
    {
        // printf("para_size[i]=%d\n",para_size[i]);
        ret=clSetKernelArg(handle->kernel,i,para_size[i],para[i]);
        mException((ret!=CL_SUCCESS),EXIT,"invalid input cl program parameter");
    }

    cl_event *event_list = morn_cl_function_event;
    int event_num = morn_cl_function_event_num;
    // printf("event_num=%d\n",event_num);
    // for(int i=0;i<event_num;i++)printf("event=%p\n",event_list[i]);

    ret = clEnqueueNDRangeKernel(queue,handle->kernel,morn_cl_dim,NULL,morn_cl_size,NULL,event_num,event_list,&event);
    mException((ret!=CL_SUCCESS),EXIT,"invalid input cl program");
}

int CLOUT(MMemoryBlock *block)
{
    int n=morn_cl_function_para_num++;
    if(n==0){morn_cl_function_event_num=0;morn_cl_memory_block=NULL;}
    
    morn_cl_memory_block=block;
    block->flag = MORN_DEVICE;
    morn_cl_function_para[n]=&(block->cl_data);
    morn_cl_function_para_size[n]=sizeof(cl_mem);
    return DFLT;
}
int CLIN(MMemoryBlock *block)
{
    int n=morn_cl_function_para_num++;
    if(n==0){morn_cl_function_event_num=0;morn_cl_memory_block=NULL;}
    
    if(block->flag==MORN_HOST) mMemoryBlockWrite(block);
    morn_cl_function_event[morn_cl_function_event_num++]=block->cl_evt;
    // printf("block->cl_evt=%p\n",block->cl_evt);
    morn_cl_function_para[n]=&(block->cl_data);
    morn_cl_function_para_size[n]=sizeof(cl_mem);
    return DFLT;
}
int CLINOUT(MMemoryBlock *block)
{
    int n=morn_cl_function_para_num++;
    if(n==0){morn_cl_function_event_num=0;morn_cl_memory_block=NULL;}
    
    if(block->flag==MORN_HOST) mMemoryBlockWrite(block);
    morn_cl_function_event[morn_cl_function_event_num++]=block->cl_evt;
    
    morn_cl_memory_block=block;
    
    morn_cl_function_para[n]=&(block->cl_data);
    morn_cl_function_para_size[n]=sizeof(cl_mem);
    return DFLT;
}
int CLPARA(void *para,int size)
{
    int n=morn_cl_function_para_num++;
    if(n==0){morn_cl_function_event_num=0;morn_cl_memory_block=NULL;}
    
    morn_cl_function_para[n]=para;
    morn_cl_function_para_size[n]=size;
    return DFLT;
}
int CLSize(int n,int s1,int s2,int s3,int s4)
{
    morn_cl_dim=n;
    mException(n==0,EXIT,"invalid cl size");\
    morn_cl_size[0]=s1;morn_cl_size[1]=s2;morn_cl_size[2]=s3;morn_cl_size[3]=s4;
    return (DFLT+DFLT);
}
#else
void mMemoryBlockWrite(MMemoryBlock *block) {mException(block->device!=MORN_HOST,EXIT,"invalid memory device");}
void mMemoryBlockRead(MMemoryBlock *block)  {mException(block->device!=MORN_HOST,EXIT,"invalid memory device");}
void mMemoryBlockCopy(MMemoryBlock *block,int device) {mException((block->device!=MORN_HOST)||(device!=MORN_HOST),EXIT,"invalid memory device");}
#endif

struct HandleMemory
{
    int devflag;
    int device;
    
    int mem_num;
    
    int write_idx;
    char *write_pdata;
    int write_size;

    int collect_num;
    int *collect_valid;
    int collect_idx;
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
    handle->devflag= MORN_HOST;
    memory->device = MORN_HOST;
    
    handle->mem_num = num;
    memory->num = handle->mem_num;
    
    memory->data = (void **)malloc(num*sizeof(MMemoryBlock *));
    if(size<0)
    {
        memset(memory->data,0,num*sizeof(MMemoryBlock *));
        return memory;
    }

    size=((size+15)>>4)<<4;
    for(int i=0;i<num;i++) memory->data[i]=(void *)mMemoryBlockCreate(size,device);
    
    return memory;
}

void mMemoryRelease(MMemory *memory)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    struct HandleMemory *handle = ((MHandle *)(memory->handle->data[0]))->handle;
    
    for(int i=0;i<handle->mem_num;i++)
    {
        if(memory->data[i]!=NULL) mMemoryBlockRelease((MMemoryBlock *)(memory->data[i]));
    }
    
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
    if(device<0) {device=memory->device;}
    
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
    if((size<=0)&&(device==handle->device)) return;

    for(int i=0;i<num;i++)
    {
        MMemoryBlock *block = memory->data[i];
        int s = size;
        if(block==NULL){if(s>0) {memory->data[i]=mMemoryBlockCreate(s,device);}}
        else
        {
            if(s<=0) s=block->size;
            
            if((s>block->size)||(block->device!=device))
                {mMemoryBlockRelease(block);memory->data[i]=mMemoryBlockCreate(s,device);}
            else mMemoryBlockRead(block);
        }
    }
    
    if(device>0) handle->device=device;
    handle->devflag= MORN_HOST;
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
    memmove(memory->data+1,memory->data,memory_num*sizeof(MMemoryBlock *));
    
    // if(handle->write_pdata==NULL) handle->write_idx+=1;

    size = ((size+15)>>4)<<4;
    memory->data[0] = mMemoryBlockCreate(size,handle->device);
    
    for(int i=1;i<memory->num;i++) mMemoryBlockRead(memory->data[i]);
    handle->devflag= MORN_HOST;
    memory->device = MORN_HOST;
    return (((MMemoryBlock *)(memory->data[0]))->data);
}

void mMemoryClear(MMemory *memory)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    MHandle *hdl = (MHandle *)(memory->handle->data[0]);
    mException(hdl->flag!= HASH_Memory,EXIT,"invalid memory");
    struct HandleMemory *handle = (struct HandleMemory *)(hdl->handle);

    for(int i=1;i<memory->num;i++)
    {
        if(memory->data[i]!=NULL) {mMemoryBlockRelease((MMemoryBlock *)(memory->data[i]));memory->data[i]=NULL;}
    }
    memory->num=1;

    MMemoryBlock *p = memory->data[0];
    handle->write_idx=0;
    handle->write_pdata=p->data;
    handle->write_size =p->size;
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
        if(handle->devflag==MORN_HOST) return;
        for(int i=0;i<memory->num;i++) mMemoryBlockRead((MMemoryBlock *)(memory->data[i]));
        handle->devflag = MORN_HOST;
    }
    else if(device==handle->device)
    {
        memory->device = handle->device;
        if(handle->devflag==MORN_DEVICE) return;
        for(int i=0;i<memory->num;i++) mMemoryBlockWrite((MMemoryBlock *)(memory->data[i]));
        handle->devflag = MORN_DEVICE;
    }
    else
    {
        for(int i=0;i<memory->num;i++) mMemoryBlockCopy((MMemoryBlock *)(memory->data[i]),device);
        memory->device = device;
        handle->device = device;
        handle->devflag= MORN_DEVICE;
    }
    #endif
}

int mMemorySize(MMemory *memory)
{
    int size = 0;
    for(int i=0;i<memory->num;i++) size += ((MMemoryBlock *)(memory->data[i]))->size;
    return size;
}

int mMemoryCheck(MMemory *memory,void *check)
{
    mException(INVALID_POINTER(memory),EXIT,"invalid input");
    for(int i=0;i<memory->num;i++)
    {
        MMemoryBlock *block = (MMemoryBlock *)(memory->data[i]);
        if(check < block->data) continue;
        if((char *)check >= (char *)(block->data)+block->size)continue;
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
    if(handle->collect_idx<0) handle->collect_idx = 0;

    int idx=handle->collect_idx;
    int i=idx;do
    {
        MMemoryBlock *block = (MMemoryBlock *)(memory->data[i]);
        if(data>=block->data)if((char *)data<((char *)(block->data))+block->size)
            {
                handle->collect_valid[i]=1;
                handle->collect_idx=i;
                return;
            }
        i++;if(i>=memory->num) i=0;
    }while(i!=idx);
}

void MemoryDefrag(MMemory *memory)
{
    struct HandleMemory *handle = ((MHandle *)(memory->handle->data[0]))->handle;
    mException((handle->collect_num!=memory->num)||(handle->collect_idx<0),EXIT,"invalid defrag memory");
    handle->collect_idx = DFLT;
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
        else mMemoryBlockRelease(memory->data[i]);
    }
    memory->num = n;
    memset(memory->data+n,0,(handle->mem_num-n)*sizeof(void *));
    memset(handle->collect_valid,0,n*sizeof(int));
}

void mMemoryCopy(MMemory *src,void ***isrc,MMemory *dst,void ***idst,int batch,int *num)
{
    mException(INVALID_POINTER(src)||INVALID_POINTER(dst)||(dst==src),EXIT,"invalid input");
    mMemoryRedefine(dst,src->num,DFLT,DFLT);
    struct HandleMemory *handle1 = ((MHandle *)(src->handle->data[0]))->handle;
    struct HandleMemory *handle2 = ((MHandle *)(dst->handle->data[0]))->handle;
    int dev1=handle1->device;int dev2=handle2->device;
    for(int i=0;i<src->num;i++)
    {
        MMemoryBlock *src_data = src->data[i];
        MMemoryBlock *dst_data = dst->data[i];
        if(dst_data->size<src_data->size) {mMemoryBlockRelease(dst_data);dst->data[i]=mMemoryBlockCreate(src_data->size,dev2);dst_data = dst->data[i];}
        
        mMemoryBlockRead(src_data);
        memcpy(dst_data->data,src->data,src_data->size);
    }
    handle1->devflag = MORN_HOST;
    handle2->devflag = MORN_HOST;
    src->device = MORN_HOST;
    dst->device = MORN_HOST;

    for(int j=0;j<batch;j++)for(int i=0;i<num[j];i++)
    {
        char *p = (char *)(isrc[j][i]);
        int n;for(n=0;n<src->num;n++)
        {
            MMemoryBlock *src_data = src->data[n];
            MMemoryBlock *dst_data = dst->data[n];
            if((p>=(char *)(src_data->data))&&(p<((char *)(src_data->data))+src_data->size))
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
    mException(((memory1==NULL)&&(memory2==NULL))||(memory1==memory2),EXIT,"invalid input");
    if(dst==memory2) {mMemoryMerge(memory2,memory1,dst);return;}
    
    struct HandleMemory *handle=NULL;
    struct HandleMemory *handle1=NULL;int num1=0;if(memory1!=NULL) {handle1 = ((MHandle *)(memory1->handle->data[0]))->handle;num1 = memory1->num;}
    struct HandleMemory *handle2=NULL;int num2=0;if(memory2!=NULL) {handle2 = ((MHandle *)(memory2->handle->data[0]))->handle;num2 = memory2->num;}

    if((memory1!=NULL)&&(memory2!=NULL))
    {
        mException(handle1->device!=handle2->device,EXIT,"source memory with different device");
        if(handle1->devflag!=handle2->devflag) {mMemoryDevice(memory1,MORN_HOST);mMemoryDevice(memory2,MORN_HOST);}
    }
    
    mMemoryRedefine(dst,num1+num2,DFLT,DFLT);
    if(dst==memory1) handle=handle1;
    else
    {
        handle  = ((MHandle *)(dst->handle->data[0]))->handle;
        for(int i=0;i<dst->num;i++) {if(dst->data[i]!=NULL) mMemoryBlockRelease(dst->data[i]);} dst->num=0;
        handle->device = handle1->device;

        if(num1>0)
        {
            memcpy(dst->data,memory1->data,num1*sizeof(MMemoryBlock *));
            memset(memory1->data,0,num1*sizeof(MMemoryBlock *));
            memory1->num=0;
        }
    }
    
    if(num2>0)
    {
        memcpy(dst->data+num1,memory2->data,num2*sizeof(MMemoryBlock *));
        memset(memory2->data,0,num2*sizeof(MMemoryBlock *));
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
        MMemoryBlock *block = (MMemoryBlock *)(memory->data[j]);
        mException((row*col_size>block->size),EXIT,"invalid memory index,with row=%d,col_size=%d,memory size=%d",row,col_size,block->size);
        index[j][0] = block->data;
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
    int device = handle->device;
    if(memory->device!=MORN_HOST) mMemoryDevice(memory,MORN_HOST);

    if(memory->num==0) mMemoryRedefine(memory,1,MAX(8192,(size<<6)),device);
    
    if(handle->write_pdata==NULL)
    {
        MMemoryBlock *block =(MMemoryBlock *)(memory->data[0]);
        handle->write_idx=0;
        handle->write_pdata=block->data;
        handle->write_size =block->size;
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
            int s = MAX(MAX(8192,(size<<6)),((MMemoryBlock *)(memory->data[idx-1]))->size*2);
            memory->data[idx] = mMemoryBlockCreate(s,device);
        }
        MMemoryBlock *block =(MMemoryBlock *)(memory->data[idx]);
        handle->write_pdata=block->data;
        handle->write_size =block->size;
        goto MemoryWrite_Check;
    }
    
    if(data!=NULL) memcpy(handle->write_pdata,data,size);
    
    char *memory_data = handle->write_pdata;
    handle->write_pdata= handle->write_pdata+ size;
    handle->write_size = handle->write_size - size;
    
    return ((void *)memory_data);
}
