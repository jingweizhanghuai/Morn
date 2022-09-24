## Morn：进程间通信

Morn提供了基于共享内存的进程间通信，支持：单点通信模式（Request-Reply）和发布订阅模式（Publish-Subscribe） 



### 接口

#### 主题发布

```c
void *mProcTopicWrite(const char *topic_name,char *string);
void *mProcTopicWrite(const char *topic_name,void *data,int size);
```

此为在发布订阅模式下的数据写入函数。

其中`topic_name`为topic名称，当发布为字符串时，可不指定数据大小，否则应设置`size`为传输数据的字节数。

返回值是所写入的信息在共享内存中的地址，信息发送失败时返回NULL。

#### 主题订阅

```c
void *mProcTopicRead(const char *topic_name);
void *mProcTopicRead(const char *topic_name,void *data);
void *mProcTopicRead(const char *topic_name,void *data,int *size);
```

此为在发布订阅模式下的数据读出函数。

其中`topic_name`为topic名称，数据将被复制到`data`所指向的内存空间（如果无需复制，则可不设置`data`），`size`是读取到的数据字节数（可不设置）。

返回值为读出的信息在共享内存中的地址，信息接收失败时返回NULL。



在单点通信模式下需要给参与通信的各个进程起个名字，各进程间应避免重名，信息接收方与信息发送方名称一致时，才能够接收到信息。

#### 信息写入

```c
void *mProcMessageWrite(const char *name,char *string);
void *mProcMessageWrite(const char *name,void *data,int size);
```

此为单点通信模式下的信息写入函数。

`name`为目标进程名称，当发布为字符串时，可不指定数据大小，否则应设置`size`为传输数据的字节数。

返回值是所写入的信息在共享内存中的地址，信息发送失败时返回NULL。

#### 信息读出

```c
void *m_ProcMessageRead(const char *name);
void *m_ProcMessageRead(const char *name,void *data,int *size);
void *m_ProcMessageRead(const char *name,void *data,int *size);
```

此为单点通信模式下的信息读出函数。

`name`为目标进程名称，数据将被复制到`data`所指向的内存空间（如果无需复制，则可不设置`data`），`size`是读取到的数据字节数（可不设置）。

返回值是所读出的信息在共享内存中的地址，信息接收失败时返回NULL。



### 示例

#### 示例一







### 性能

此例用以测试在不同进程间用共享内存（Morn）和socket（zeromq）两种方式通信的信息延迟。信息发送方在信息发送前记录当前时间戳，并将此时间戳作为通信内容的一部分发送给接收方。信息接收方在接收到信息后，首先记录信息接收的时间戳，并计算与发送时间的时间间隔，此间隔即为信息发送-接收的时间延迟。

程序中，每间隔1ms发送/接收一组数据，共计发送/接收1000组数据，并计算1000次收发的平均延迟。

Morn测试程序如下：

```c
void morn_send(int n)
{
    struct timeval tv;
    int *msg=malloc(n*sizeof(int));
    for(int i=0;i<1000;i++)
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
    struct timeval tv;
    double t_sum=0;

    int *msg=malloc(n*sizeof(int));
    int i;for(i=1;;i++)
    {
        mProcTopicRead("topictest",msg,NULL);
        gettimeofday(&tv,NULL);
        double t = (double)((tv.tv_sec-msg[1])*1000000+(tv.tv_usec-msg[2]));
        t_sum += t;
        if(msg[0]==1000-1) break;
    }
    printf("recive %d topictest data (size %d), average delay %fus.\n", i,n*sizeof(int),t_sum/i);
    free(msg);
}

int main(int argc,char **argv)
{
    int n=1024;
    if(argc==3) n=atoi(argv[2]);
    n=n/sizeof(int);

         if(strcmp(argv[1],"morn_send"  )==0)     morn_send(n);
    else if(strcmp(argv[1],"morn_recive")==0)   morn_recive(n);
    else mException(1,EXIT,"invalid input");
    return 0;
}
```

zeromq测试程序如下：

```c
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
    for (int i=0;i<1000;i++)
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
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_SUB);
    zmq_bind(responder, "tcp://*:5555");

    struct ZMQData *msg = malloc(sizeof(struct ZMQData)+n*sizeof(int));
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
        if(msg->data[0]==1000-1) break;
    }
    printf("recive %d %s data (size %d), average delay %fus.\n",i,msg->topic,n*sizeof(int),t_sum/i);
    free(msg);
}

int main(int argc,char **argv)
{
    int n=1024;
    if(argc==3) n=atoi(argv[2]);
    n=n/sizeof(int);

         if(strcmp(argv[1], "zmq_send"  )==0)   zeromq_send(n);
    else if(strcmp(argv[1], "zmq_recive")==0) zeromq_recive(n);
    else mException(1,EXIT,"invalid input");
    return 0;
}
```

测试结果如下：

[![5uUyGQ.png](https://z3.ax1x.com/2021/10/13/5uUyGQ.png)](https://imgtu.com/i/5uUyGQ)

由此可见：

在传输的信息量小时（<10kByte），使用共享内存的Morn延迟要远快于使用socket的zeromq，前者延迟为微秒级，而后者是毫秒级。

在传输的信息量大时（>100kByte），Morn的延迟迅速增加，相比zeromq的优势缩小。



