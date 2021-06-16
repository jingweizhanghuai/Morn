// gcc -O2 -fopenmp test_message2.c -o test_message2.exe -lmorn -lzmq -lstdc++ -lws2_32 -lIphlpapi -lm
#include <zmq.h>
#include "morn_util.h"

#ifdef __linux__
#include <sys/time.h>
#endif
#ifdef __MINGW32__
#define gettimeofday mingw_gettimeofday
#endif

void *mProcTopicWrite(const char *msgname,void *data,int write_size);
void *mProcTopicRead(const char *msgname,void *data,int *read_size);

#define N (1024*1024/4)
#define T 10000

struct ZMQData
{
    char topic[8];
    int data[N];
};

void zeromq_send()
{
    void *context = zmq_ctx_new();
    void *requester = zmq_socket(context, ZMQ_PUB);
    zmq_connect(requester, "tcp://localhost:5555");

    struct ZMQData *buffer = malloc(sizeof(struct ZMQData));
    strcpy(buffer->topic,"zmqtest");
    
    struct timeval tv;
    for (int i=0;i<T;i++)
    {
        for(int j=3;j<N;j++) buffer->data[j]=rand();
        buffer->data[0]=i;
        gettimeofday(&tv,NULL);buffer->data[1]=tv.tv_sec;buffer->data[2]=tv.tv_usec;
        
        zmq_send(requester,buffer,sizeof(struct ZMQData),0);
        mSleep(5);
    }
    zmq_close(requester);
    zmq_ctx_destroy(context);
}

void zeromq_recive()
{
    int pid=getpid();
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_SUB);
    zmq_bind(responder, "tcp://*:5555");

    struct ZMQData *buffer = malloc(sizeof(struct ZMQData));
    strcpy(buffer->topic,"zmqtest");
    zmq_setsockopt(responder,ZMQ_SUBSCRIBE,buffer->topic,strlen(buffer->topic));

    double t_sum=0;
    struct timeval tv;
    int i;for(i=1;;i++) 
    {
        int n=zmq_recv(responder,buffer,sizeof(struct ZMQData),0);
        gettimeofday(&tv,NULL);
        t_sum += ((tv.tv_sec-buffer->data[1])*1000000+(tv.tv_usec-buffer->data[2]));
        if(buffer->data[0]==T-1) break;
    }
    printf("recive %d topic data, average delay %fus\n",i,t_sum/i);
}

void morn_send()
{
    struct timeval tv;
    int *msg=malloc(N*sizeof(int));
    for(int i=0;i<T;i++)
    {
        for(int i=3;i<N;i++) msg[i]=rand();
        msg[0]= i;
        gettimeofday(&tv,NULL);msg[1]=tv.tv_sec;msg[2]=tv.tv_usec;

        mProcTopicWrite("topictest",msg,N*sizeof(int));
        // mSleep(5);
    }
    free(msg);
}

void morn_recive()
{
    int pid=getpid();
    struct timeval tv;
    double t_sum=0;
    int i;for(i=1;;i++)
    {
        int *msg=mProcTopicRead("topictest",NULL,NULL);
        gettimeofday(&tv,NULL);
        t_sum += (double)((tv.tv_sec-msg[1])*1000000+(tv.tv_usec-msg[2]));
        if(msg[0]==T-1) break;
    }
    printf("recive %d topic data, average delay %fus\n",i,t_sum/i);
}

int main(int argc,char **argv)
{
         if(strcmp(argv[1], "zmq_send"  )==0)   zeromq_send();
    else if(strcmp(argv[1],"morn_send"  )==0)     morn_send();
    else if(strcmp(argv[1], "zmq_recive")==0) zeromq_recive();
    else if(strcmp(argv[1],"morn_recive")==0)   morn_recive();
    else mException(1,EXIT,"invalid input");
    return 0;
}