#include "morn_util.h"

void server()
{
    char data[64];
    while(1)
    {
        mSleep(10);
        int size=64;
        char *ip=mTCPServerRead(":4433",data,&size);
        if(ip==NULL) continue;
        printf("receive from %s, size=%d, data=%s\n",ip,size,data);
    }
}

void client()
{
    printf("1111111111111111111111\n");
    char data[64];
    while(1)
    {
        mSleep(1000);
        mRandString(data,32,64);
        char *ip=mTCPClientWrite("192.168.10.182:4433",data,DFLT);
        printf("send to %s, data=%s\n",ip,data);
    }
}

int main(int argc,char *argv[])
{
    if(strcmp(argv[1],"server")==0) server();
    else                            client();
    return 0;
}
    
    
