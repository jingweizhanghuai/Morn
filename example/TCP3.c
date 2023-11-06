#include "morn_util.h"

void func(char *str)
{
    for(int i=0;str[i];i++)
    {
             if((str[i]>='a')&&(str[i]<='z')) str[i]+=('A'-'a');
        else if((str[i]>='A')&&(str[i]<='Z')) str[i]+=('a'-'A');
    }
}

void server()
{
    char data[64];char *ip;
    while(1)
    {
        mSleep(10);
        int size=64;
        
        ip=mTCPServerRead(":1234",data,&size);
        if(ip==NULL) continue;
        printf("recv from %s, size=%d, data=%s\n",ip,size,data);
        
        func(data);
        
        ip=mTCPServerWrite(ip,data,size);
        printf("send  to  %s, size=%d, data=%s\n",ip,size,data);
    }
}

void client()
{
    char data[64];char *ip;
    while(1)
    {
        mSleep(1000);
        mRandString(data,32,64);
        ip=mTCPClientWrite("localhost:1234",data,DFLT);
        printf("send  to  %s, data=%s\n",ip,data);
        
        int size=64;
        ip=mTCPClientRead("localhost:1234",data,&size);
        printf("recv from %s, data=%s\n",ip,data);
    }
}

int main(int argc,char *argv[])
{
    if(strcmp(argv[1],"server")==0) server();
    else                            client();
    return 0;
}
    
    
