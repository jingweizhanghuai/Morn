// gcc -O2 -fopenmp test_message2.c -o test_message2.exe -lmorn -lzmq -lstdc++ -lws2_32 -lIphlpapi -lm
#include <zmq.h>
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

    struct ZMQData *buffer = malloc(sizeof(struct ZMQData)+n*sizeof(int));
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

void zeromq_recive(int n)
{
    int pid=getpid();
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_SUB);
    zmq_bind(responder, "tcp://*:5555");

    struct ZMQData *buffer = malloc(sizeof(struct ZMQData)+n*sizeof(int));
    strcpy(buffer->topic,"zmqtest");
    zmq_setsockopt(responder,ZMQ_SUBSCRIBE,buffer->topic,strlen(buffer->topic));

    double t_sum=0;double t_max=0;
    struct timeval tv;
    int i;for(i=1;;i++) 
    {
        int n=zmq_recv(responder,buffer,sizeof(struct ZMQData)+n*sizeof(int),0);
        gettimeofday(&tv,NULL);
        // printf("buffer->data[0]=%d\n",buffer->data[0]);
        double t = ((tv.tv_sec-buffer->data[1])*1000000+(tv.tv_usec-buffer->data[2]));
        t_sum += t; t_max=MAX(t,t_max);
        if(buffer->data[0]==T-1) break;
    }
    printf("recive %d %s data (size %d), average delay %fus.\n",i,buffer->topic,n*sizeof(int),t_sum/i);
    free(buffer);
}

void morn_send(int n)
{
    struct timeval tv;
    int *msg=malloc(n*sizeof(int));
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

void morn_recive(int n)
{
    int pid=getpid();
    struct timeval tv;
    double t_sum=0;double t_max=0;
    int i;for(i=1;;i++)
    {
        int *msg=mProcTopicRead("topictest",NULL,NULL);
        gettimeofday(&tv,NULL);
        // printf("msg[0]=%d\n",msg[0]);
        double t = (double)((tv.tv_sec-msg[1])*1000000+(tv.tv_usec-msg[2]));
        t_sum += t; t_max = MAX(t,t_max);
        if(msg[0]==T-1) break;
    }
    printf("recive %d topictest data (size %d), average delay %fus.\n",i,n*sizeof(int),t_sum/i);
}

int main(int argc,char **argv)
{
    int n=1024;
    if(argc==3) n=atoi(argv[2]);
    n=n/sizeof(int);

         if(strcmp(argv[1], "zmq_send"  )==0)   zeromq_send(n);
    else if(strcmp(argv[1],"morn_send"  )==0)     morn_send(n);
    else if(strcmp(argv[1], "zmq_recive")==0) zeromq_recive(n);
    else if(strcmp(argv[1],"morn_recive")==0)   morn_recive(n);
    else mException(1,EXIT,"invalid input");
    return 0;
}