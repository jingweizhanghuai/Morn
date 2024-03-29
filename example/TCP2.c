#include "morn_util.h"

void client()
{
    char data[64];
    while(1)
    {
        mSleep(100);
        int size=64;
        char *ip=mTCPClientRead("localhost:1234",data,&size);
        if(ip==NULL) continue;
        printf("receive from %s, size=%d, data=%s\n",ip,size,data);
    }
}

void server()
{
    char data[64];
    while(1)
    {
        mSleep(1000);
        mRandString(data,32,64);
        char *ip=mTCPServerWrite("localhost:1234",data,DFLT);
        printf("send to %s, data=%s\n",ip,data);
    }
}

int main(int argc,char *argv[])
{
    if(strcmp(argv[1],"server")==0) server();
    else                            client();
    return 0;
}
