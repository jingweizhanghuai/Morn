#include "morn_util.h"

void client1()
{
    char data[64];
    while(1)
    {
        mSleep(100);
        int size=64;
        char *ip=mTCPClientRead("localhost:1234",data,&size);
        if(ip==NULL) printf("no data receive\n");
        else         printf("receive from %s, size=%d, data=%s\n",ip,size,data);
    }
}

void client2()
{
    ////////////////////////////////////////////////////////////////////////////////////////
    int t=0;mPropertyWrite("localhost:2345","TCP_wait",&t,sizeof(int));
    ////////////////////////////////////////////////////////////////////////////////////////
    
    char data[64];
    while(1)
    {
        mSleep(100);
        int size=64;
        char *ip=mTCPClientRead("localhost:2345",data,&size);
        if(ip==NULL) printf("no data receive\n");
        else         printf("receive from %s, size=%d, data=%s\n",ip,size,data);
    }
}

void client3()
{
    ////////////////////////////////////////////////////////////////////////////////////////
    int t=100;mPropertyWrite("localhost:3456","TCP_wait",&t,sizeof(int));
    ////////////////////////////////////////////////////////////////////////////////////////
    
    char data[64];
    while(1)
    {
        mSleep(100);
        int size=64;
        char *ip=mTCPClientRead("localhost:3456",data,&size);
        if(ip==NULL) printf("no data receive\n");
        else         printf("receive from %s, size=%d, data=%s\n",ip,size,data);
    }
}

void server()
{
    char data[64];
    char *ip;
    while(1)
    {
        mSleep(1000);
        mRandString(data,32,64);
        ip=mTCPServerWrite("localhost:1234",data,DFLT);
        printf("send to %s, data=%s\n",ip,data);
        ip=mTCPServerWrite("localhost:2345",data,DFLT);
        printf("send to %s, data=%s\n",ip,data);
        ip=mTCPServerWrite("localhost:3456",data,DFLT);
        printf("send to %s, data=%s\n",ip,data);
    }
}

int main(int argc,char *argv[])
{
         if(strcmp(argv[1],"client1")==0) client1();
    else if(strcmp(argv[1],"client2")==0) client2();
    else if(strcmp(argv[1],"client3")==0) client3();
    else                                   server();
    return 0;
}
