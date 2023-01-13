// build: gcc -O2 test_process_topic2.c -liceoryx_binding_c -liceoryx_posh -liceoryx_utils -liceoryx_platform -lstdc++ -lpthread -lrt -lmorn -lm -o test_process_topic2.exe

#include "morn_ptc.h"
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/subscriber.h"

#define T 1000000

iox_pub_storage_t m_publisherStorage;
iox_sub_storage_t m_subscriberStorage;
iox_pub_t m_publisher;
iox_sub_t m_subscriber;

void construct(const char * publisherName, const char * subscriberName)
{
    iox_pub_options_t publisherOptions;
    iox_pub_options_init(&publisherOptions);
    publisherOptions.historyCapacity = 0U;
    publisherOptions.nodeName = "SlapStick";
    m_publisher = iox_pub_init(&m_publisherStorage, "Comedians", publisherName, "Duo", &publisherOptions);

    iox_sub_options_t subscriberOptions;
    iox_sub_options_init(&subscriberOptions);
    subscriberOptions.queueCapacity = 10U;
    subscriberOptions.historyRequest = 0U;
    subscriberOptions.nodeName = "Slapstick";
    m_subscriber = iox_sub_init(&m_subscriberStorage, "Comedians", subscriberName, "Duo", &subscriberOptions);
}

void init()
{
    iox_pub_offer(m_publisher);
    iox_sub_subscribe(m_subscriber);

    while (iox_sub_get_subscription_state(m_subscriber) != SubscribeState_SUBSCRIBED) mSleep(1);
    while (!iox_pub_has_subscribers(m_publisher)) mSleep(1);
}

void destruct()
{
    iox_pub_deinit(m_publisher);
    iox_sub_deinit(m_subscriber);
}

void sendPerfTopic(int count, int size)
{
    void* userPayload = NULL;
    if (iox_pub_loan_chunk(m_publisher, &userPayload, size) == AllocationResult_SUCCESS)
    {
        int *data = (int *)userPayload;
        data[0]=count;data[1]=size;
        iox_pub_publish_chunk(m_publisher, userPayload);
    }
}

void receivePerfTopic(int *count,int *size)
{
    int hasReceivedSample=0;
    while(!hasReceivedSample)
    {
        const void* userPayload = NULL;
        if (iox_sub_take_chunk(m_subscriber, &userPayload) == ChunkReceiveResult_SUCCESS)
        {
            int *data = (int *)userPayload;
            *count = data[0];*size=data[1];
            hasReceivedSample = 1;
            iox_sub_release_chunk(m_subscriber, userPayload);
        }
    }
}

void pingPongLeader(int size)
{
    mTimerBegin("iceoryx");
    int count=0;
    for (int i = 0; i < T; ++i)
    {
        sendPerfTopic(count,size);
        receivePerfTopic(&count,&size);
        count++;
    }
    mTimerEnd("iceoryx");
    printf("count=%d,size=%d\n\n",count,size);
}

void pingPongFollower()
{
    int count,size;
    while(1)
    {
        receivePerfTopic(&count,&size);
        if(count<0) break;
        count++;
        sendPerfTopic(count,size);
    }
}

void iceoryx_leader()
{
    iox_runtime_init("iox-leader-app");
    construct("to_follower","to_leader");
    init();
    for(int size=64;size<=256*1024;size*=4)
        pingPongLeader(size);
    sendPerfTopic(-1,2*sizeof(int));
    destruct();
}

void iceoryx_follower()
{
    iox_runtime_init("iox-follower-app");
    construct("to_leader","to_follower");
    init();
    pingPongFollower();
    destruct();
}


void morn_leader()
{
    int size=256*1024;
    int *data=mMalloc(size);
    int *p=NULL;
    mPropertyWrite("to_follower","topic_size",&size,sizeof(int));
    for(size=64;size<=256*1024;size*=4)
    {
        data[0]=0;
        mTimerBegin("morn");
        for(int i=0;i<T;i++)
        {
            mProcTopicWrite("to_follower",data,size);
            do{p=mProcTopicRead("to_leader");}while(p==NULL);
            data[0]=p[0]+1;
        }
        mTimerEnd("morn");
        printf("count=%d,size=%d\n\n",data[0],size);
    }
    data[0]=-1;
    mProcTopicWrite("to_follower",data,sizeof(int));
    mFree(data);
}

void morn_follower()
{
    int *p=NULL;int size=256*1024;
    mPropertyWrite("to_leader","topic_size",&size,sizeof(int));
    while(1)
    {
        do{p=mProcTopicRead("to_follower",NULL,&size);}while(p==NULL);
        if(p[0]<0) break;
        p[0]++;
        mProcTopicWrite("to_leader",p,size);
    }
}

void test_memcpy()
{
    int size = 1024*1024;
    int *data1=mMalloc(size);
    int *data2=mMalloc(size);
    for(size=64;size<=256*1024;size*=4)
    {
        data1[0]=0;
        mTimerBegin("memcpy");
        for(int i=0;i<T;i++)
        {
            memcpy(data2,data1,size);
            data2[0]++;
            memcpy(data1,data2,size);
            data1[0]++;
        }
        mTimerEnd("memcpy");
        printf("count=%d,size=%d\n\n",data1[0],size);
    }
    mFree(data1);
    mFree(data2);
}

int main(int argc,char *argv[])
{
         if(strcmp(argv[1],"iceoryx_leader"  )==0) iceoryx_leader();
    else if(strcmp(argv[1],"iceoryx_follower")==0) iceoryx_follower();
    else if(strcmp(argv[1],   "morn_leader"  )==0) morn_leader();
    else if(strcmp(argv[1],   "morn_follower")==0) morn_follower();
    else if(strcmp(argv[1],     "memcpy"     )==0) test_memcpy();
    else printf("run as: \"test_process_topic.exe morn_leader\", or \"test_process_topic.exe morn_follower\"\n");
    return 0;
}
