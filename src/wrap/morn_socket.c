#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "morn_util.h"

#if defined MORN_USE_SOCKET

#if defined(_WIN64)||defined(_WIN32)
#include <winsock2.h>
#define ADDR(S) (S).S_un.S_addr
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
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

void IPAddress(const char *ip,uint32_t *addr,uint32_t *port)
{
    if(ip==NULL) return;
    unsigned char *p = (unsigned char *)addr;
    if(strnicmp(ip,"localhost",9)==0) {p[0]=127,p[1]=0;p[2]=0;p[3]=1;ip=ip+9;}
    if(ip[0]==':') {*port=atoi(ip+1);return;}

    int a[4];
    sscanf(ip,"%d.%d.%d.%d:%d",a,a+1,a+2,a+3,port);
    p[0]=a[0];p[1]=a[1];p[2]=a[2];p[3]=a[3];
}

struct HandleUDP
{
    SOCKET udp;
    uint32_t addr;
    uint32_t port;
    struct sockaddr_in send_addr;
    struct sockaddr_in recive_addr;
    char addr_str[32];
    int wait_time;
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

#define HandleUDPWrite HandleUDP
void endUDPWrite(struct HandleUDP *handle) {endUDP(handle);}
#define HASH_UDPWrite 0xc953a45
char *mUDPWrite(const char *address,void *data,int size)
{
    mException((data==NULL),EXIT,"invalid input size");
    if(size<0) size=strlen(data);
    
    MHandle *hdl=mHandle("UDP",UDPWrite);
    struct HandleUDP *handle = hdl->handle;
    uint32_t addr=handle->addr;uint32_t port=handle->port;
    if(address!=NULL) IPAddress(address,&addr,&port);
    if((hdl->valid==0)||(handle->addr!=addr)||(handle->port!=port))
    {
        handle->addr = addr;handle->port=port;
        if(hdl->valid==0) {if(UDPSetup(handle)==MORN_FAIL) return 0;}

        handle->send_addr.sin_family = AF_INET;
        handle->send_addr.sin_port = htons(handle->port);
        ADDR(handle->send_addr.sin_addr) = handle->addr;
        if(connect(handle->udp,(struct sockaddr*)&(handle->send_addr),sizeof(struct sockaddr))<0) return 0;

        hdl->valid=1;
    }

    if(size<=0) size = strlen(data);
    sendto(handle->udp,data,size,0, (struct sockaddr *)&(handle->send_addr),sizeof(struct sockaddr_in));

    uint8_t *s=(uint8_t *)(&(ADDR(handle->send_addr.sin_addr)));
    sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],handle->port);
    
    return handle->addr_str;
}

#define HandleUDPRead HandleUDP
void endUDPRead(struct HandleUDP *handle) {endUDP(handle);}
#define HASH_UDPRead 0xf222ef4a
char *mUDPRead(const char *address,void *data,int *size)
{
    mException((size==NULL)||(data==NULL),EXIT,"invalid input size");

    MHandle *hdl=mHandle("UDP",UDPRead);
    struct HandleUDP *handle = hdl->handle;
    uint32_t addr=htonl(INADDR_ANY);uint32_t port=handle->port;
    if(address!=NULL) IPAddress(address,&addr,&port);
    if((hdl->valid==0)||(handle->addr!=addr)||(handle->port!=port))
    {
        handle->wait_time=DFLT;
        mPropertyVariate("UDP","wait_time",&(handle->wait_time));
        handle->addr = addr;handle->port=port;
        mException(handle->port==0,EXIT,"invalid UDP port");
        if(hdl->valid==0) {if(UDPSetup(handle)==MORN_FAIL) return 0;}

        handle->recive_addr.sin_family = AF_INET;
        handle->recive_addr.sin_port = htons(handle->port);
        ADDR(handle->recive_addr.sin_addr) = handle->addr;
        if(bind(handle->udp,(struct sockaddr *)&(handle->recive_addr),sizeof(struct sockaddr))<0) return 0;
        
        hdl->valid=1;
    }
    ADDR(handle->recive_addr.sin_addr) = handle->addr;

    if(handle->wait_time>=0)
    {
        struct timeval wait_time;wait_time.tv_sec=handle->wait_time/1000;wait_time.tv_usec=(handle->wait_time%1000)*1000;

        struct fd_set fds;
        FD_ZERO(&fds);
        FD_SET(handle->udp,&fds);
        #if defined(_WIN64)||defined(_WIN32)
        int maxfd=0;
        #else
        int maxfd=handle->udp+1;
        #endif
        int flag = select(maxfd,&fds,NULL,NULL,&wait_time);
        if(flag==0) return NULL;
        mException(flag==SOCKET_ERROR,EXIT,"udp error");
    }
    
    unsigned int len = sizeof(struct sockaddr_in);
    int ret = recvfrom(handle->udp,data,*size,0,(struct sockaddr *)&(handle->recive_addr),(void *)(&len));
    if(ret<*size) ((char *)data)[ret]=0;
    *size = ret;

    uint8_t *s=(uint8_t *)(&(ADDR(handle->recive_addr.sin_addr)));
    sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],handle->port);
    
    return handle->addr_str;
}

#endif
