/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include "morn_ptc.h"

struct BuffData
{
    int size;
    char data[0];
};

#define WAIT(Condition,Wait_Time) do{\
    if(Wait_Time==0) return NULL;\
    else if(Wait_Time< 0) while(Condition);\
    else\
    {\
        uint64_t t0 = mTime();\
        while(Condition)\
        {\
            if(mTime()-t0>Wait_Time) return NULL;\
        }\
    }\
}while(0)

struct HandleQueue
{
    MList *queue;
    
    int read_rear;
    int write_rear;
    
    volatile int read_order;
    volatile int write_order;
    volatile int flag;

    int wait_time;
};
void endQueue(void *info) {}
#define HASH_Queue 0xd98b43dc

void QueqeNum(int *q_num,struct HandleQueue *handle)
{
    int queue_num = *q_num;
    
    if(queue_num==handle->write_rear) return;
    
    if(queue_num > handle->queue->num) mListAppend(handle->queue,queue_num);
    else handle->queue->num= queue_num;
    
    if(handle->write_order>queue_num)
    {
        handle->read_rear = handle->write_order;
        handle->write_order = 0;
    }
    handle->write_rear= queue_num;
}

int mQueueSize(MList *queue)
{
    mException(INVALID_POINTER(queue),EXIT,"invalid queue");
    MHandle *hdl=mHandle(queue,Queue);
    struct HandleQueue *handle = (struct HandleQueue *)(hdl->handle);
    
    // if(handle->flag>0) return queue->num;
    // if(handle->flag<0) return 0;
    
    int n = handle->write_order - handle->read_order;
    return ((n>0)?n:(handle->read_rear+n));
}

void *mQueueWrite(MList *queue,void *data,int size)
{
    mException(INVALID_POINTER(queue),EXIT,"invalid queue");
    mException((data==NULL)&&(size<=0),EXIT,"invalid input");
    if(size<0) size=strlen(data);
    
    MHandle *hdl=mHandle(queue,Queue);
    struct HandleQueue *handle = (struct HandleQueue *)(hdl->handle);
    if(hdl->valid == 0) 
    {
        handle->read_order=0;
        handle->write_order=0;
        handle->wait_time=DFLT;
        mPropertyVariate(queue,"wait_time",&(handle->wait_time),sizeof(int));
        mPropertyFunction(queue,"queue_num",QueqeNum,handle);
    }
    hdl->valid = 1;
    
    WAIT(handle->flag>0,handle->wait_time);

    int order=mAtomicAdd(&(handle->write_order),1);
    if(order==handle->write_rear-1) {handle->read_rear = handle->write_rear;handle->write_order=0;}
    if(order>=handle->write_rear) order=order-handle->write_rear;

    struct BuffData *p = mListWrite(queue,order,NULL,sizeof(struct BuffData)+size);
    p->size=size;if(data!=NULL) memcpy(p->data,data,size);
    
    
    handle->flag =(handle->write_order == handle->read_order)?1:0;
    return (p->data);
}

void *mQueueRead(MList *queue,void *data,int *size)
{
    mException(INVALID_POINTER(queue),EXIT,"invalid queue");
    
    MHandle *hdl=mHandle(queue,Queue);
    struct HandleQueue *handle = (struct HandleQueue *)(hdl->handle);
    WAIT((hdl->valid==0)||(handle->flag<0),handle->wait_time);
    
    int order = mAtomicAdd(&(handle->read_order),1);
    if(order==handle->read_rear-1) handle->read_order=0;
    if(order>=handle->read_rear) order=order-handle->read_rear;
    
    struct BuffData *p = queue->data[order];
    handle->flag =(handle->write_order == handle->read_order)?-1:0;

    int s=p->size;
    if(!INVALID_POINTER(size)) {s=*size;*size = p->size;}
    if(!INVALID_POINTER(data)) memcpy(data,p->data,s);
    return (p->data);
}


struct HandleFlow
{
    MSheet *sheet;
    MArray *id_array;
    MArray *sgn_array;
    int row;

    MThreadSignal sgn;

    int flow_exit;

    int buff_num;//16
    int wait_time;
};
void endFlow(struct HandleFlow *handle)
{
    if(handle->id_array !=NULL) mArrayRelease(handle-> id_array);
    if(handle->sgn_array!=NULL) mArrayRelease(handle->sgn_array);
}
#define HASH_Flow 0xb8be12fb

// void *mFlowRead(MSheet *sheet,int order,void *data,int size)
// {
//     mException(INVALID_POINTER(sheet)||order<0,EXIT,"invalid input");
//     MHandle *hdl = mHandle(sheet,Flow);
//     struct HandleFlow *handle = (struct HandleFlow *)(hdl->handle);
//     if(hdl->valid==0)
//     {
//         handle->sheet = sheet;
//         int n = MAX(MAX(order,sheet->row),64);
//         if(handle-> id_array==NULL) handle->id_array =mArrayCreate(n,sizeof(MThread));
//         if(handle->sgn_array==NULL) handle->sgn_array=mArrayCreate(n,sizeof(MThreadSignal));
//         memset(handle->id_array->data,0,sizeof(MThread)*n);
        
//         handle->row=MAX(handle->row,order);
//         handle->buff_num=16;
//         handle->wait_time=-1;

//         hdl->valid = 1;
//     }
    

//     if(order>sheet->row)
//     {
//         mThreadLockBegin(handle->sgn);
//         mSheetRowAppend(sheet,order);
//         int n = handle->id_array->num;
//         if(order<n)
//         {
//             mArrayAppend(handle->id_array ,order);
//             mArrayAppend(handle->sgn_array,order);
//             memset(handle->id_array->dataU8+n*sizeof(MThread),0,(order-n)*sizeof(MThread));
//         }
//         mThreadLockEnd(handle->sgn);
//     }

//     if(order == handle->flow_exit)
//     {
//         while(sheet->col[order]==0)
//         {
//             if(!mThreadActive(tid[order]) return NULL;
//         }
//     }
//     else
//         mThreadWake();

//     n=sheet->col[order]-1;
    
    
    
// }









/*
int pthread_kill(pthread_t thread, int sig);

struct HandleFlow
{
    int buff_num;
    int *order;
    pthread_mutex_t *mutex;
    int *flag;
    
    int thread_num;
    int *locate;
    pthread_cond_t *cond;
    pthread_t *ID;
    
    int locate_over;
};
void endFlow(void *info)
{
    struct HandleFlow *handle = (struct HandleFlow *)info;
    if(handle->order  != NULL) mFree(handle->order);
    if(handle->mutex  != NULL) mFree(handle->mutex);
    if(handle->flag   != NULL) mFree(handle->flag);
    if(handle->locate != NULL) mFree(handle->locate);
    if(handle->cond   != NULL) mFree(handle->cond);
    if(handle->ID     != NULL) mFree(handle->ID);
}
#define HASH_Flow 0xb8be12fb

static pthread_t undefined_thread;
#define INVALID_ID(ID) (pthread_equal((ID),undefined_thread))

void mFlowSet(MList *buff,int buff_num,int thread_num)
{
    mException((buff==NULL),EXIT,"invalid buffer");
    mException((thread_num<1),EXIT,"invalid thread num");
    
    if(buff_num <=0) buff_num = buff->num;
    mException((buff_num < thread_num),EXIT,"invalid input");
    
    mListAppend(buff,buff_num);
    
    MHandle *hdl=mHandle(buff,Flow);
    struct HandleFlow *handle = (struct HandleFlow *)(hdl->handle);
    if(hdl->valid == 0)
    {
        if(handle->order != NULL) if(handle->buff_num < buff_num) {mFree(handle->order);handle->order = NULL;}
        if(handle->order == NULL) handle->order = (int *)mMalloc(buff_num*sizeof(int));
        
        if(handle->mutex != NULL) if(handle->buff_num < buff_num) {mFree(handle->mutex);handle->mutex = NULL;}
        if(handle->mutex == NULL) handle->mutex = (pthread_mutex_t *)mMalloc(buff_num*sizeof(pthread_mutex_t));
       
        if(handle->flag != NULL) if(handle->buff_num < buff_num) {mFree(handle->flag);handle->flag = NULL;}
        if(handle->flag == NULL) handle->flag = (int *)mMalloc(buff_num*sizeof(int));
        
        for(int i=0;i<buff_num;i++)
        {
            handle->order[i] = thread_num-1;
            handle->mutex[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
            handle->flag[i] = 0;
        }
        if(handle->buff_num<buff_num) handle->buff_num = buff_num;
        
        if(handle->locate != NULL) if(handle->thread_num < thread_num) {mFree(handle->locate);handle->locate = NULL;}
        if(handle->locate == NULL) handle->locate = (int *)mMalloc(thread_num*sizeof(int));
       
        if(handle->cond != NULL) if(handle->thread_num < thread_num) {mFree(handle->cond);handle->cond = NULL;}
        if(handle->cond == NULL) handle->cond = (pthread_cond_t *)mMalloc(thread_num*sizeof(pthread_cond_t));
        
        if(handle->ID != NULL) if(handle->thread_num < thread_num) {mFree(handle->ID);handle->ID = NULL;}
        if(handle->ID == NULL) handle->ID = (pthread_t *)mMalloc(thread_num*sizeof(pthread_t));
        undefined_thread=pthread_self();
        
        for(int i=0;i<thread_num;i++)
        {
            handle->locate[i] = 0;
            handle->cond[i] = (pthread_cond_t)PTHREAD_COND_INITIALIZER;
            memcpy(handle->ID+i,&undefined_thread,sizeof(pthread_t));
        }
        handle->thread_num = thread_num;
        
        handle->locate_over = -1;
        
        hdl->valid = 1;
    }
}

void *mFlowRead(MList *buff,int order,void *data,int size)
{
    mException((buff==NULL),EXIT,"invalid input buffer");
    
    MHandle *hdl=mHandle(buff,Flow);
    struct HandleFlow *handle = (struct HandleFlow *)(hdl->handle);
    mException((hdl->valid == 0),EXIT,"invalid buffer");
    
    mException((order>=handle->thread_num),EXIT,"invalid input");
    
    int locate = handle->locate[order];
    // printf("read order is %d,locate is %d\n",order,locate);
    if(((handle->flag[locate]&0x01) != 0)&&(handle->order[locate] == order))
    {
        int l = locate;
        if(l == handle->locate_over) return NULL;
        
        locate = locate + 1;
        if(locate >= buff->num) locate = 0;
        handle->locate[order] = locate;
        
        handle->flag[l] = 0;
        pthread_cond_signal(&(handle->cond[order]));
        pthread_mutex_unlock(&(handle->mutex[l]));
        // printf("read order%d uuuuuuuuuuuuuuuuuuuuuuuuuuuuunlock %d\n",order,l);
    }
    
    if((handle->order[locate]!=order)||(handle->flag[locate]==0))
    {
        int next = (order==handle->thread_num-1)?order:(order+1);
        int last = (order==0)?(handle->thread_num-1):(order-1);
   
        while(pthread_mutex_trylock(&(handle->mutex[locate]))!=0)
        {
            if((handle->locate[next] == locate)&&(!INVALID_ID(handle->ID[last])))
            {
                int state = pthread_kill(handle->ID[last],0);
                if((state == ESRCH)||(state == EINVAL))
                {
                    handle->flag[locate] = 0;
                    handle->locate_over = handle->locate[last];
                    printf("read last is %d,handle->locate_over is %d\n",last,handle->locate_over);
                    pthread_cond_signal(&(handle->cond[last]));
                    break;
                }
            }
        }
        while(handle->order[locate]!=last)
        {
            pthread_cond_wait(&(handle->cond[last]),&(handle->mutex[locate]));
        }
    }
    
    handle->flag[locate] |= 0x01;
    handle->order[locate] = order;
    if(INVALID_ID(handle->ID[order])) 
        handle->ID[order] = pthread_self();
    
    void *p= buff->data[locate];
    if(data!=NULL) 
    {
        if(size>0) memcpy(data,p,size);
        else       strcpy((char *)data,(char *)p);
    }
    return p;
}

void *mFlowWrite(MList *buff,int order,void *data,int size)
{
    mException((buff==NULL),EXIT,"invalid input buffer");
    
    MHandle *hdl=mHandle(buff,Flow);
    struct HandleFlow *handle = (struct HandleFlow *)(hdl->handle);
    mException((hdl->valid == 0),EXIT,"invalid buffer");
    
    mException((order>=handle->thread_num),EXIT,"invalid input");
    
    int locate = handle->locate[order];
    // printf("order is %d,locate is %d,flag is %d\n",order,locate,handle->flag[locate]);
    // printf("order is %d,handle->flag[locate] is %d\n",order,handle->flag[locate]);
    // printf("order is %d,handle->order[locate] is %d\n",order,handle->order[locate]);
    if(((handle->flag[locate]&0x02) != 0)&&(handle->order[locate] == order))
    {
        int l = locate;
        if(l == handle->locate_over) return NULL;
        
        locate = locate + 1;
        if(locate >= buff->num) locate = 0;
        handle->locate[order] = locate;
        
        handle->flag[l] = 0;
        // printf("write order%d uuuuuuuuuuuuuuuuuuuuuuuuuuuuunlock %d\n",order,l);
        pthread_cond_signal(&(handle->cond[order]));
        pthread_mutex_unlock(&(handle->mutex[l]));
    }
    
    if((handle->order[locate]!=order)||(handle->flag[locate]==0))
    {
        int next = (order==handle->thread_num-1)?order:(order+1);
        int last = (order==0)?(handle->thread_num-1):(order-1);
        while(pthread_mutex_trylock(&(handle->mutex[locate]))!=0)
        {
            if((handle->locate[next] == locate)&&(!INVALID_ID(handle->ID[last])))
            {
                int state = pthread_kill(handle->ID[last],0);
                if((state == ESRCH)||(state == EINVAL))
                {
                    handle->flag[locate] = 0;
                    handle->locate_over = handle->locate[last];
                    printf("write last is %d,handle->locate_over is %d\n",last,handle->locate_over);
                    pthread_cond_signal(&(handle->cond[last]));
                    break;
                }
            }
        }
        // printf("write order%d llllllllllllllllllllllock %d\n",order,locate);
        while(handle->order[locate]!=last)
            pthread_cond_wait(&(handle->cond[last]),&(handle->mutex[locate]));
    }
    
    handle->flag[locate] |= 0x02;
    handle->order[locate] = order;
    if(INVALID_ID(handle->ID[order])) 
        handle->ID[order] = pthread_self();
    
    void *p= mListWrite(buff,locate,data,size);
    return p;
}

*/
