#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morn_util.h"

#if defined MORN_USE_SOCKET

#if defined(_WIN64)||defined(_WIN32)
#include <winsock2.h>
#define ADDR(S) (S).S_un.S_addr
#else
#include <netinet/in.h>
#include <sys/socket.h>
#define SOCKET int
#define closesocket close
#define ADDR(S) (S).s_addr
#endif

#ifdef __GNUC__
#define strnicmp strncasecmp
#endif

void IPAddress(const char *ip,int *addr,int *port)
{
    unsigned char *p = (unsigned char *)addr;
    p[0]=127,p[1]=0;p[2]=0;p[3]=1;
    *port=8888;

    if(ip==NULL) return;
    if(strnicmp(ip,"localhost",9)==0) return;

    sscanf(ip,"%d.%d.%d.%d:%d",(int *)p,(int *)(p+1),(int *)(p+2),(int *)(p+3),port);
}

struct HandleUDP
{
    SOCKET udp;
    int server;
    int addr;
    int port;
};
void endUDP(struct HandleUDP *handle)
{
    closesocket(handle->udp);
    #if defined _WIN64
    WSACleanup();
    #endif
}

int UDPSetup(struct HandleUDP *handle)
{
    #if defined _WIN64
    WSADATA wsa;
    WORD version = MAKEWORD(2,2);
    if(WSAStartup(version, &wsa)) return MORN_FAIL;
    #endif
    
    handle->udp = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(handle->udp<0) return MORN_FAIL;

    return MORN_SUCCESS;
}

#define HandleUDPSend HandleUDP
void endUDPSend(struct HandleUDP *handle) {endUDP(handle);}
#define HASH_UDPSend 0x4b5a5fc
int mUDPSend(const char *address,void *data,int size)
{
    mException((size==0)||(data==NULL),EXIT,"invalid input size");
    
    MHandle *hdl=mHandle(mMornObject(NULL,DFLT),UDPSend);
    struct HandleUDP *handle = hdl->handle;
    if(hdl->valid==0)
    {
        if(UDPSetup(handle)==MORN_FAIL) return 0;
        IPAddress(address,&(handle->addr),&(handle->port));
        address=NULL;
        hdl->valid=1;
    }

    if(address!=NULL) IPAddress(address,&(handle->addr),&(handle->port));
    // printf("handle->addr=%x,handle->port=%d\n",handle->addr,handle->port);

    struct sockaddr_in send_addr;
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(handle->port);
    ADDR(send_addr.sin_addr) = handle->addr;
    // printf("addr[0]=%x,addr[1]=%d\n",send_addr.sin_addr.S_un.S_addr,send_addr.sin_port);

    if(size<=0) size = strlen(data);
    sendto(handle->udp,data,size,0, (struct sockaddr *)&send_addr,sizeof(struct sockaddr_in));
    
    return size;
}

#define HandleUDPRecive HandleUDP
void endUDPRecive(struct HandleUDP *handle) {endUDP(handle);}
#define HASH_UDPRecive 0x655fa546
int mUDPRecive(const char *address,void *data,int size)
{
    mException((size==0)||(data==NULL),EXIT,"invalid input size");
    
    MHandle *hdl=mHandle(mMornObject(NULL,DFLT),UDPRecive);
    struct HandleUDP *handle = hdl->handle;
    if(hdl->valid==0)
    {
        if(UDPSetup(handle)==MORN_FAIL) return 0;
        if(address==NULL) {handle->addr=0;handle->port=8888;}
        else IPAddress(address,&(handle->addr),&(handle->port));
        address=NULL;
        hdl->valid=1;
    }

    if(address!=NULL) IPAddress(address,&(handle->addr),&(handle->port));
    
    struct sockaddr_in recive_addr;
    recive_addr.sin_family = AF_INET;
    recive_addr.sin_port = htons(handle->port);
    ADDR(recive_addr.sin_addr) = handle->addr;

    if((handle->addr==INADDR_ANY)&&(handle->server==0))
    {
        if(bind(handle->udp,(struct sockaddr *)&recive_addr,sizeof(struct sockaddr))<0)
            return 0;
        handle->server = 1;
    }

    // printf("handle->addr=%x,handle->port=%d\n",handle->addr,handle->port);
    
    unsigned int len = sizeof(struct sockaddr_in);
    int ret = recvfrom(handle->udp,data,size,0,(struct sockaddr *)&(recive_addr),(void *)(&len));
    if(ret>0) ((char *)data)[ret]=0;
    
    return ret;
}

#endif
