// gcc -O2 -fopenmp test_message3.c -o test_message3.exe -lmorn -lm
// gcc -O2 -fopenmp test_message3.c -o test_message3.exe -I C:\ProgramFiles\CPackage\zeromq\include\ -I ../include/ -L C:\ProgramFiles\CPackage\zeromq\lib_x64_mingw -L ../lib/x64_mingw/ -lzmq -lmorn -lstdc++ -lws2_32 -lIphlpapi
// #include <zmq.h>

#include "morn_ptc.h"

void func(char *str)
{
    for(char *p=str;*p;p++)
    {
             if((*p>='a')&&(*p<='z')) *p+=('A'-'a');
        else if((*p>='A')&&(*p<='Z')) *p+=('a'-'A');
    }
}

struct MessageData
{
    char clientname[32];
    char data[64];
};

void morn_server()
{
    while(1)
    {
        struct MessageData *msg=mProcMessageRead("server",NULL,NULL);
        if(strcmp((char *)msg,"exit")==0) break;
        func(msg->data);
        mProcMessageWrite(msg->clientname,msg,sizeof(struct MessageData));
    }
}

void morn_client(char *clientname)
{
    // int wait_time=100;
    // mPropertyWrite("ProcMessage","wait_time",&wait_time,sizeof(int));

    struct MessageData msg;
    strcpy(msg.clientname,clientname);
    for(int i=0;i<100;i++)
    {
        mRandString(msg.data,32,64);
        printf("%d: send   %s\n",i,msg.data);
        mProcMessageWrite("server",&msg,sizeof(struct MessageData));
        if(mProcMessageRead(clientname,&msg,NULL)==NULL) printf("%d: recive error!\n\n",i);
        else printf("%d: recive %s\n\n",i,msg.data);
        mSleep(100);
    }
}

int main(int argc,char **argv)
{
    if(strcmp(argv[1],"server")==0) morn_server();
    else if(strcmp(argv[1],"exit")==0) mProcMessageWrite("server","exit",DFLT);
    else morn_client(argv[1]);
    return 0;
}
