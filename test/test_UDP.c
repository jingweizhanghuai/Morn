/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
//build_mingw: gcc -O2 -fopenmp test_UDP.c -o test_UDP.exe -lmorn -lWs2_32
//build_gnu: gcc -O2 -fopenmp test_UDP.c -o test_UDP.exe -I ../include/ -L ../lib/x64_gnu/ -lmorn -lm
//build_msvc: cl.exe -O2 -openmp -I ..\include\ ..\lib\x64_msvc\libmorn.lib .\test_UDP.c

#include "morn_util.h"

char *IP="\0";

void test_send()
{
    char addr[32];
    if(IP[0]==0) IP="localhost";

    char data[64];
    for(int i=0;i<100;i++)
    {
        mRandString(data,32,64);
        sprintf(addr,"%s:%d",IP,rand()%4+1000);
        char *ip=mUDPWrite(addr,data,DFLT);
        printf("send: to %s, size=%zd, data is %s\n",ip,strlen(data),data);
        mSleep(10);
    }
}

void test_receive()
{
    char addr[32];
    sprintf(addr,"%s:1002",IP);
    
    int wait_time=100;
    mPropertyWrite("UDP:1002","UDP_wait",&wait_time,sizeof(int));
    
    char data[64];
    while(1)
    {
        int size=64;
        char *ip;
        ip=mUDPRead(addr,data,&size);
        if(ip==NULL) printf("receive error\n");
        else if(strcmp(data,"exit")==0) break;
        else printf("receive: from %s, size=%d, data is %s\n",ip,size,data);
    }
}

int main(int argc,char *argv[])
{
    if(argc>2) IP=argv[2];
    
         if(strcmp(argv[1],"send"  )==0) test_send();
    else if(strcmp(argv[1],"receive")==0) test_receive();
    else 
    {
        char addr[32];
        if(IP[0]==0) IP="localhost";
        sprintf(addr,"%s:1002",IP);
        mUDPWrite(addr,argv[1],DFLT);
    }
    return 0;
}
