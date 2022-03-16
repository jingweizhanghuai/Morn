// g++ -O2 -fopenmp test_message2.cpp -o test_message2.exe -lmorn -lzmq -lcppipc -lws2_32 -lIphlpapi -lm
#include <zmq.h>
#include "libipc/ipc.h"
#include "morn_ptc.h"

#ifdef __linux__
#include <sys/time.h>
#endif
#ifdef __MINGW32__
#define gettimeofday mingw_gettimeofday
#endif

// #define N (1024*128/4)
#define T 1000

struct ZMQData
{
    char topic[8];
    int data[0];
};

void zeromq_send(int n)
{
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_PUB);
    zmq_connect(requester, "tcp://localhost:5555");

    struct ZMQData *buffer = (struct ZMQData *)malloc(sizeof(struct ZMQData)+n*sizeof(int));
    strcpy(buffer->topic,"zmqtest");
    mSleep(10);

    struct timeval tv;
    for (int i=0;i<T;i++)
    {
        for(int j=3;j<n;j++) buffer->data[j]=rand();
        buffer->data[0]=i;
        gettimeofday(&tv,NULL);buffer->data[1]=tv.tv_sec;buffer->data[2]=tv.tv_usec;
        
        zmq_send(requester,buffer,sizeof(struct ZMQData)+n*sizeof(int),0);
        mSleep(1);
    }
    zmq_close(requester);
    zmq_ctx_destroy(context);
    free(buffer);
}

void zeromq_receive(int n)
{
    int pid=getpid();
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_SUB);
    zmq_bind(responder, "tcp://*:5555");

    struct ZMQData *msg = (struct ZMQData *)malloc(sizeof(struct ZMQData)+n*sizeof(int));
    strcpy(msg->topic,"zmqtest");
    zmq_setsockopt(responder,ZMQ_SUBSCRIBE,msg->topic,strlen(msg->topic));

    double t_sum=0;double t_max=0;
    struct timeval tv;
    int i;for(i=1;;i++) 
    {
        int n=zmq_recv(responder,msg,sizeof(struct ZMQData)+n*sizeof(int),0);
        gettimeofday(&tv,NULL);
        double t = ((tv.tv_sec-msg->data[1])*1000000+(tv.tv_usec-msg->data[2]));
        t_sum += t;
        if(msg->data[0]==T-1) break;
    }
    printf("receive %d %s data (size %d), average delay %fus.\n",i,msg->topic,n*sizeof(int),t_sum/i);
    free(msg);
}

void cppipc_send(int n) 
{
    struct timeval tv;
    int *msg=(int *)malloc(n*sizeof(int));
    
    ipc::channel ipc {"cppipctest", ipc::sender};
    for(int i=0;i<T;i++)
    {
        for(int j=3;j<n;j++) msg[j]=rand();
        msg[0]= i;
        gettimeofday(&tv,NULL);msg[1]=tv.tv_sec;msg[2]=tv.tv_usec;
        ipc.send(ipc::buff_t(msg,n*sizeof(int)));
        mSleep(1);
    }
}

void cppipc_receive(int n)
{
    struct timeval tv;
    double t_sum=0;
    
    ipc::channel ipc {"cppipctest", ipc::receiver};
    int i=0;while(1)
    {
        ipc::buff_t recv = ipc.recv(0);
        if(!recv.empty())
        {
            // printf("i=%d,data=%s\n",i++,recv.data());
            gettimeofday(&tv,NULL);
            int *msg = (int*)recv.data();
            double t = (double)((tv.tv_sec-msg[1])*1000000+(tv.tv_usec-msg[2]));
            t_sum += t;i++;
            if(msg[0]==T-1) break;
        }
    }
    printf("receive %d topictest data (size %d), average delay %fus.\n",i,n*sizeof(int),t_sum/i);
}

void morn_send(int n)
{
    struct timeval tv;
    int *msg=(int *)malloc(n*sizeof(int));
    for(int i=0;i<T;i++)
    {
        for(int j=3;j<n;j++) msg[j]=rand();
        msg[0]= i;
        gettimeofday(&tv,NULL);msg[1]=tv.tv_sec;msg[2]=tv.tv_usec;
        mProcTopicWrite("topictest",msg,n*sizeof(int));
        mSleep(1);
    }
    free(msg);
}

void morn_receive(int n)
{
    int pid=getpid();
    struct timeval tv;
    double t_sum=0;

    int *msg=(int *)malloc(n*sizeof(int));
    int i=0;while(1)
    {
        mProcTopicRead("topictest",msg,NULL);
        gettimeofday(&tv,NULL);
        double t = (double)((tv.tv_sec-msg[1])*1000000+(tv.tv_usec-msg[2]));
        t_sum += t;i++;
        if(msg[0]==T-1) break;
    }
    printf("receive %d topictest data (size %d), average delay %fus.\n",i,n*sizeof(int),t_sum/i);
    free(msg);
}

int main(int argc,char **argv)
{
    int n=1024;
    if(argc==3) n=atoi(argv[2]);
    n=n/sizeof(int);

         if(strcmp(argv[1],   "zmq_send"   )==0)    zeromq_send(n);
    else if(strcmp(argv[1],"cppipc_send"   )==0)    cppipc_send(n);
    else if(strcmp(argv[1],  "morn_send"   )==0)      morn_send(n);
    else if(strcmp(argv[1],   "zmq_receive")==0) zeromq_receive(n);
    else if(strcmp(argv[1],"cppipc_receive")==0) cppipc_receive(n);
    else if(strcmp(argv[1],  "morn_receive")==0)   morn_receive(n);
    else mException(1,EXIT,"invalid input");
    return 0;
}