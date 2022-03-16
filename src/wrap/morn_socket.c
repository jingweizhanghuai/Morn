
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
#include <sys/select.h>

#define SOCKET int
#define closesocket close
#define ADDR(S) (S).s_addr
#define SOCKET_ERROR (-1)
#endif
 
#ifdef __GNUC__
#define strnicmp strncasecmp
#endif

void IPAddress(const char *ip,uint32_t *addr,uint32_t *port)
{
    if(ip==NULL) return;
    unsigned char *p = (unsigned char *)addr;
    if(strnicmp(ip,"localhost",9)==0) {p[0]=127,p[1]=0;p[2]=0;p[3]=1;ip=ip+9;}
    *port=0; if(ip[0]==':') {*port=atoi(ip+1);return;}

    int a[4];
    sscanf(ip,"%d.%d.%d.%d:%d",a,a+1,a+2,a+3,port);
    p[0]=a[0];p[1]=a[1];p[2]=a[2];p[3]=a[3];
}

struct HandleUDPWrite
{
    // uint32_t addr;
    // uint32_t port;
    SOCKET udp;
    char addr_str[32];
};
void endUDPWrite(struct HandleUDPWrite *handle)
{
    closesocket(handle->udp);
    #if defined _WIN64
    WSACleanup();
    #endif
}
#define HASH_UDPWrite 0xc953a45
char *mUDPWrite(const char *udpaddress,void *data,int size)
{
    mException((data==NULL),EXIT,"invalid input size");
    if(size<0) size=strlen(data);mException(size>65507,EXIT,"size overflow");

    mException(udpaddress==NULL,EXIT,"invalid udp address");
    uint32_t addr=0,port=0;
    IPAddress(udpaddress,&addr,&port);

    char address[32];sprintf(address,"UDP:%d",port);
    MHandle *hdl = mHandle(address,UDPWrite);
    struct HandleUDPWrite *handle=hdl->handle;
    if(hdl->valid==0)
    {
        #if defined _WIN64
        WSADATA wsa;
        WORD version = MAKEWORD(2,2);
        if(WSAStartup(version, &wsa)) return NULL;
        #endif
        
        handle->udp=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

        // bool bBroadcast=1;
        // setsockopt(handle->udp,SOL_SOCKET,SO_BROADCAST,(const char*)&bBroadcast,sizeof(bool));

        hdl->valid=1;
    }
    // uint32_t port=0,addr=0;
    // if(address!=NULL) IPAddress(address,&addr,&port);
    // if(port==0) port=handle->port;else handle->port=port;
    // if(addr==0) addr=handle->addr;else handle->addr=addr;mException(addr==0,EXIT,"udp error");
    
    struct sockaddr_in send_addr;
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(port);
    ADDR(send_addr.sin_addr) = addr;
    sendto(handle->udp,data,size,0,(struct sockaddr *)(&send_addr),sizeof(struct sockaddr_in));
    
    uint8_t *s=(uint8_t *)(&addr);
    sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],port);
    return handle->addr_str;
}

/*
struct HandleUDPRead
{
    MChain *map;
    int port,addr;
    SOCKET *udp;
};
void closeUDP(const void *key,const int key_size,SOCKET *udp,int value_size,void *para)
{
    closesocket(*udp);
}
void endUDPRead(struct HandleUDPRead *handle)
{
    mornMapNodeOperate(handle->map,closeUDP,NULL);
    mChainRelease(handle->map);
    #if defined _WIN64
    WSACleanup();
    #endif
}
#define HASH_UDPRead 0xf222ef4a
char morn_udp_addr_str[32];
int morn_udp_wait_time=DFLT;

SOCKET *GetUDP(uint32_t port,uint32_t addr)
{
    MHandle *hdl=mHandle("UDP",UDPRead);
    struct HandleUDP *handle = hdl->handle;
    if(hdl->valid==0)
    {
        #if defined _WIN64
        WSADATA wsa;
        WORD version = MAKEWORD(2,2);
        if(WSAStartup(version, &wsa)) return NULL;
        #endif

        mPropertyVariate("UDP","UDP_wait",&morn_udp_wait_time);
        
        handle->map = mChainCreate();
        hdl->valid=1;
    }
    if(port==handle->port)&&(addr=handle->addr) return handle->udp;
    
    uint32_t key[2]={addr,port};
    handle->udp=mornMapRead(handle->map,key,2*sizeof(uint32_t),NULL,NULL);
    if(handle->udp!=NULL) return handle->udp;
    
    SOCKET udp;
    udp = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);if(udp<0) return NULL;
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    ADDR(addr.sin_addr) = addr;
    if(bind(udp,(struct sockaddr *)&addr,sizeof(struct sockaddr))<0) return NULL;
    
    handle->udp=(struct UDPPara *)mornMapWrite(handle->map,key,2*sizeof(uint32_t),&udp,sizeof(SOCKET));
    return handle->udp;
}
*/

struct HandleUDPRead
{
    uint32_t addr;
    uint32_t port;
    SOCKET udp;
    struct sockaddr_in recive_addr;
    int wait_time;
    char addr_str[32];
    
    int *buff;
    MList *list;
    int order;
};
void endUDPRead(struct HandleUDPRead *handle)
{
    closesocket(handle->udp);
    #if defined _WIN64
    WSACleanup();
    #endif
    if(handle->buff!=NULL) mFree(handle->buff);
    if(handle->list!=NULL) mListRelease(handle->list);
}
#define HASH_UDPRead 0xf222ef4a
char *mUDPRead(const char *udpaddress,void *data,int *size)
{
    int ret;
    mException((size==NULL)||(data==NULL),EXIT,"invalid input size");
    mException(udpaddress==NULL,EXIT,"invalid udp address");
    uint32_t addr=htonl(INADDR_ANY),port=0;
    IPAddress(udpaddress,&addr,&port);
    
    char address[32];sprintf(address,"UDP:%d",port);
    MHandle *hdl = mHandle(address,UDPRead);
    struct HandleUDPRead *handle = hdl->handle;
    
    if(hdl->valid==0)
    {
        #if defined _WIN64
        WSADATA wsa;
        WORD version = MAKEWORD(2,2);
        if(WSAStartup(version, &wsa)) return NULL;
        #endif

        handle->port=port;
        handle->addr=addr;

        handle->udp=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

        // bool bReuseaddr=1;
        // setsockopt(handle->udp,SOL_SOCKET ,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(bool));

        handle->recive_addr.sin_family = AF_INET;
        handle->recive_addr.sin_port = htons(handle->port);
        ADDR(handle->recive_addr.sin_addr) = htonl(INADDR_ANY);
        ret=bind(handle->udp,(struct sockaddr *)&(handle->recive_addr),sizeof(struct sockaddr));
        mException(ret<0,EXIT,"udp bind error");

        if(handle->addr!=htonl(INADDR_ANY))
        {
            uint8_t *s=(uint8_t *)(&(handle->addr));
            sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],handle->port);

            if(handle->buff==NULL) handle->buff=mMalloc(65508+2*sizeof(int));
            if(handle->list==NULL) handle->list=mListCreate();
        }

        handle->wait_time=DFLT;
        mPropertyVariate(address,"UDP_wait",&(handle->wait_time));
        
        hdl->valid=1;
    }

    int *pdata=data;
    int *psize=size;
    if(handle->addr!=htonl(INADDR_ANY))
    {
        for(int i=0;i<handle->list->num;i++)
        {
            int n=handle->order+i;if(n>=handle->list->num) n-=handle->list->num;
            int *p = handle->list->data[n];
            if(p[0]!=handle->addr) continue;
            *size=MIN(*size,p[1]);
            memcpy(data,p+2,*size);
            mListElementDelete(handle->list,n);
            handle->order--;if(handle->order<0) handle->order=handle->list->num-1;
            return handle->addr_str;
        }
        pdata=handle->buff+2;
        psize=handle->buff+1;*psize=65507;
    }
    
    // printf("handle->wait_time=%d\n",handle->wait_time);
    if(handle->wait_time>=0)
    {
        struct timeval wait_time;wait_time.tv_sec=handle->wait_time/1000;wait_time.tv_usec=(handle->wait_time%1000)*1000;

        fd_set fds;
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
    ret = recvfrom(handle->udp,(char *)pdata,*psize,0,(struct sockaddr *)&(handle->recive_addr),(void *)(&len));
    if(ret<*psize) ((char *)data)[ret]=0;
    *psize = ret;
    
    if(handle->addr==htonl(INADDR_ANY))
    {
        uint8_t *s=(uint8_t *)(&(ADDR(handle->recive_addr.sin_addr)));
        sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],handle->port);
        return handle->addr_str;
    }
    else
    {
        pdata[-2]=ADDR(handle->recive_addr.sin_addr);
        if(pdata[-2]==handle->addr)
        {
            *size=MIN(*size,*psize);
            memcpy(data,pdata,*size);
            return handle->addr_str;
        }
        else
        {
            mListWrite(handle->list,handle->order,handle->buff,ret+2*sizeof(int));
            handle->order++;if(handle->order==128)handle->order=0;
            return NULL;
        }
    }
}


/*
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
char *m_UDPWrite(MObject *obj,const char *address,void *data,int size)
{
    mException((data==NULL),EXIT,"invalid input size");
    if(size<0) size=strlen(data);
    
    MHandle *hdl=mHandle(obj,UDPWrite);
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
        // if(connect(handle->udp,(struct sockaddr*)&(handle->send_addr),sizeof(struct sockaddr))<0) return 0;

        hdl->valid=1;
    }
    
    sendto(handle->udp,data,size,0, (struct sockaddr *)&(handle->send_addr),sizeof(struct sockaddr_in));

    uint8_t *s=(uint8_t *)(&(ADDR(handle->send_addr.sin_addr)));
    sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],handle->port);
    
    return handle->addr_str;
}

#define HandleUDPRead HandleUDP
void endUDPRead(struct HandleUDP *handle) {endUDP(handle);}
#define HASH_UDPRead 0xf222ef4a
char *m_UDPRead(MObject *obj,const char *address,void *data,int *size)
{
    mException((size==NULL)||(data==NULL),EXIT,"invalid input size");

    MHandle *hdl=mHandle(obj,UDPRead);
    struct HandleUDP *handle = hdl->handle;
    uint32_t addr=htonl(INADDR_ANY);uint32_t port=handle->port;
    if(address!=NULL) IPAddress(address,&addr,&port);
    if((hdl->valid==0)||(handle->addr!=addr)||(handle->port!=port))
    {
        handle->wait_time=DFLT;
        mPropertyVariate(obj,"UDP_wait",&(handle->wait_time));
        handle->addr = addr;handle->port=port;
        mException(handle->port==0,EXIT,"invalid UDP port");
        // printf("handle->port = %d\n",handle->port);
        if(hdl->valid==0) {if(UDPSetup(handle)==MORN_FAIL) return NULL;}

        handle->recive_addr.sin_family = AF_INET;
        handle->recive_addr.sin_port = htons(handle->port);
        ADDR(handle->recive_addr.sin_addr) = handle->addr;
        if(bind(handle->udp,(struct sockaddr *)&(handle->recive_addr),sizeof(struct sockaddr))<0) return NULL;

        hdl->valid=1;
    }
    ADDR(handle->recive_addr.sin_addr) = handle->addr;

    if(handle->wait_time>=0)
    {
        struct timeval wait_time;wait_time.tv_sec=handle->wait_time/1000;wait_time.tv_usec=(handle->wait_time%1000)*1000;

        fd_set fds;
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
*/

/*
struct HandleTCPSClient
{
    SOCKET tcp;
    uint32_t addr;
    uint32_t port;
    int wait_time;
    char addr_str[32];
}
void endTCPSClient(struct HandleTCPSClient *handle)
{
    closesocket(handle->tcp);
    #if defined _WIN64
    WSACleanup();
    #endif
}
#define HASH_TCPSClient 0x649112a4
char *m_TCPSClientWrite(const char *server_address,void *data,int size)
{
    if(size<0) size=strlen(data);
    
    MHandle *hdl=mHandle("TCP",TCPSClient);
    struct HandleTCPSClient *handle = hdl->handle;
    if(hdl->valid==0)
    {
        #if defined _WIN64
        WSADATA wsa;
        WORD version = MAKEWORD(2,2);
        mException(WSAStartup(version,&wsa),EXIT,"tcp error");
        #endif
        handle->tcp = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        mException(handle->tcp<0,EXIT,"tcp error");
        hdl->valid=1;
    }
    
    uint32_t addr=handle->addr,port=handle->port;
    if(server_address!=NULL)
    {
        IPAddress(server_address,&addr,&port);
        handle->addr=addr;handle->port=port;
    }
    mException((addr==0)||(port==0),EXIT,"invalid tcp address");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    ADDR(server_addr.sin_addr) = addr;
    int ret = connect(handle->tcp,(struct sockaddr*)&server_addr,sizeof(struct sockaddr));
    if(ret<0) return NULL;
    
    send(handle->tcp,data,size,0);

    uint8_t *s=(uint8_t *)(&addr);
    sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],port);
    return handle->addr_str;
}

char *m_TCPSClientRead(const char *server_address,void *data,int *size)
{
    mException(data==NULL,EXIT,"invalid input");
    int s=0x8fffffff; if(size==NULL) size=&s;
    
    MHandle *hdl=mHandle("TCP",TCPSClient);
    struct HandleTCPSClient *handle = hdl->handle;
    if(hdl->valid==0)
    {
        #if defined _WIN64
        WSADATA wsa;
        WORD version = MAKEWORD(2,2);
        mException(WSAStartup(version,&wsa),EXIT,"tcp error");
        #endif
        handle->tcp = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        mException(handle->tcp<0,EXIT,"tcp error");
        handle->wait_time=DFLT;
        mPropertyVariate("TCP","TCPS_wait",&(handle->wait_time));////////////////
        hdl->valid=1;
    }

    uint32_t addr=handle->addr,port=handle->port;
    if(server_address!=NULL)
    {
        IPAddress(server_address,&addr,&port);
        handle->addr=addr;handle->port=port;
    }
    mException((addr==0)||(port==0),EXIT,"invalid tcp address");
    
}





struct HandleTCPClient
{
    SOCKET tcp;
    uint32_t addr;
    uint32_t port;
    struct sockaddr_in server_addr;
    
    char addr_str[32];
};
void endTCPClient(struct HandleTCPClient *handle)
{
    closesocket(handle->tcp);
    #if defined _WIN64
    WSACleanup();
    #endif
}

#define HASH_TCPClient 0xc33ebe53
struct HandleTCPClient *m_TCPClient(MObject *obj,const char *server_address)
{
    MHandle *hdl=mHandle(obj,TCPClient);
    struct HandleTCPClient *handle = hdl->handle;
    uint32_t addr=handle->addr,port=handle->port;
    if(server_address!=NULL) IPAddress(server_address,&addr,&port);
    if((hdl->valid==0)||(handle->addr!=addr)||(handle->port!=port))
    {
        handle->addr = addr;handle->port=port;
        if(hdl->valid==0) 
        {
            #if defined _WIN64
            WSADATA wsa;
            WORD version = MAKEWORD(2,2);
            mException(WSAStartup(version,&wsa),EXIT,"tcp error");
            #endif

            handle->tcp = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
            mException(handle->tcp<0,EXIT,"tcp error");
        }

        handle->server_addr.sin_family = AF_INET;
        handle->server_addr.sin_port = htons(handle->port);
        ADDR(handle->server_addr.sin_addr) = handle->addr;
        int ret = connect(handle->tcp,(struct sockaddr*)&(handle->server_addr),sizeof(struct sockaddr));
        if(ret<0) return NULL;

        uint8_t *s=(uint8_t *)(&(ADDR(handle->server_addr.sin_addr)));
        sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],handle->port);
        
        hdl->valid=1;
    }
    return handle;
}

char *m_TCPClientWrite(MObject *obj,const char *server_address,void *data,int size)
{
    struct HandleTCPClient *handle = m_TCPClient(obj,server_address);
    if(handle==NULL) return NULL;
    if(size<0) size=strlen(data);
    send(handle->tcp,data,size,0);
    return handle->addr_str;
}
char *m_TCPClientRead(MObject *obj,const char *server_address,void *data,int *size)
{
    struct HandleTCPClient *handle = m_TCPClient(obj,server_address);
    if(handle==NULL) return NULL;
    int ret = recv(handle->tcp,data,*size,0);
    if(ret<0) return NULL;
    if(ret<*size) ((char *)data)[ret]=0;
    *size = ret;return handle->addr_str;
}












struct HandleTCPServer
{
    SOCKET server;
    struct sockaddr_in server_addr;
    uint32_t server_port;
    
    struct sockaddr_in client_addr[128];
    SOCKET client_socket[128];
    int client_order0;
    int client_order1;
    
    char addr_str[32];
};
void endTCPServer(struct HandleTCPServer *handle)
{
    closesocket(handle->server);
    #if defined _WIN64
    WSACleanup();
    #endif
}
#define HASH_TCPServer 0x6fafa477
struct HandleTCPServer *m_TCPServer(MObject *obj,uint32_t port)
{
    MHandle *hdl=mHandle(obj,TCPServer);
    struct HandleTCPServer *handle = hdl->handle;
    if(hdl->valid==0)
    {
        mException(port==0,EXIT,"invalid TCP port");
        handle->server_port=port;
        if(hdl->valid==0) 
        {
            #if defined _WIN64
            WSADATA wsa;
            WORD version = MAKEWORD(2,2);
            mException(WSAStartup(version,&wsa),EXIT,"tcp error");
            #endif
            handle->server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            mException(handle->server<0,EXIT,"tcp error");
            hdl->valid=1;
        }
        handle->server_addr.sin_family = AF_INET;
        handle->server_addr.sin_port = htons(handle->server_port);
        ADDR(handle->server_addr.sin_addr) = htonl(INADDR_ANY);
        mException(bind(handle->server,(struct sockaddr *)&(handle->server_addr),sizeof(struct sockaddr))<0,EXIT,"tcp error");

        listen(handle->server,10);
    }
    
    return handle;
}
char *m_TCPServerRead(MObject *obj,const char *address,void *data,int *size)
{
    uint32_t addr=0,port=0;
    if(address!=NULL) IPAddress(address,&addr,&port);
    struct HandleTCPServer *handle = m_TCPServer(obj,port);
    
    int ret,n;
    for(int i=handle->client_order0;i<handle->client_order0+128;i++)
    {
        n=i%128;
        if(addr>0) {if(memcmp(handle->client_addr+n,&addr,4)!=0) continue;}
        if(handle->client_socket[n]==0) continue;
        ret = recv(handle->client_socket[n],data,*size,0);
        // if(ret<=0) continue;
        if(ret<=0) return NULL;
        
        if(ret<*size) ((char *)data)[ret]=0;
        *size = ret;
        handle->client_order0 = n;
        uint8_t *s=(uint8_t *)(&(ADDR(handle->server_addr.sin_addr)));
        sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],handle->server_port);
        return handle->addr_str;
    }

    struct timeval wait_time;memset(&wait_time,0,sizeof(struct timeval));
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(handle->server,&fds);
    #if defined(_WIN64)||defined(_WIN32)
    int maxfd=0;
    #else
    int maxfd=handle->server+1;
    #endif
    int flag = select(maxfd,&fds,NULL,NULL,&wait_time);
    if(flag==0) return NULL;
    mException(flag==SOCKET_ERROR,EXIT,"tcp error");
    
    n=handle->client_order1;if(n==128) n=0;
    handle->client_order1=n+1;
    int addr_size = sizeof(struct sockaddr_in);
    handle->client_socket[n] = accept(handle->server,(struct sockaddr *)(handle->client_addr+n),&addr_size);
    if(handle->client_socket[n]<=0) return NULL;

    FD_ZERO(&fds);
    FD_SET(handle->client_socket[n],&fds);
    #if defined(_WIN64)||defined(_WIN32)
    maxfd=0;
    #else
    maxfd=handle->client_socket[n]+1;
    #endif
    flag = select(maxfd,&fds,NULL,NULL,&wait_time);
    if(flag==0) return NULL;
    mException(flag==SOCKET_ERROR,EXIT,"tcp error");
    
    if(addr>0) {if(memcmp(handle->client_addr+n,&addr,4)!=0) return NULL;}
    ret = recv(handle->client_socket[n],data,*size,0);
    if(ret<0) return NULL;
    
    if(ret<*size) ((char *)data)[ret]=0;
    *size = ret;
    
    uint8_t *s=(uint8_t *)(&(ADDR(handle->server_addr.sin_addr)));
    sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],handle->server_port);
    return handle->addr_str;
}

char *m_TCPServerWrite(MObject *obj,const char *address,void *data,int size)
{
    uint32_t addr=0,port=0;
    if(address!=NULL) IPAddress(address,&addr,&port);
    mException(addr==0,EXIT,"invalid input");
    struct HandleTCPServer *handle = m_TCPServer(obj,port);

    int n;
    for(int i=handle->client_order0;i<handle->client_order0+128;i++)
    {
        n=i%128;
        if(memcmp(handle->client_addr+n,&addr,4)!=0) continue;
        if(handle->client_socket[n]==0) continue;
        
        send(handle->client_socket[n],data,size,0);

        handle->client_order0 = n;
        uint8_t *s=(uint8_t *)(&(ADDR(handle->server_addr.sin_addr)));
        sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],handle->server_port);
        return handle->addr_str;
    }

    struct timeval wait_time;memset(&wait_time,0,sizeof(struct timeval));
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(handle->server,&fds);
    #if defined(_WIN64)||defined(_WIN32)
    int maxfd=0;
    #else
    int maxfd=handle->server+1;
    #endif
    int flag = select(maxfd,&fds,NULL,NULL,&wait_time);
    if(flag==0) return NULL;
    mException(flag==SOCKET_ERROR,EXIT,"tcp error");

    n=handle->client_order1;if(n==128) n=0;
    handle->client_order1=n+1;
    int addr_size = sizeof(struct sockaddr_in);
    handle->client_socket[n] = accept(handle->server,(struct sockaddr *)(handle->client_addr+n),&addr_size);
    if(handle->client_socket[n]<=0) return NULL;

    FD_ZERO(&fds);
    FD_SET(handle->client_socket[n],&fds);
    #if defined(_WIN64)||defined(_WIN32)
    maxfd=0;
    #else
    maxfd=handle->client_socket[n]+1;
    #endif
    flag = select(maxfd,&fds,NULL,NULL,&wait_time);
    if(flag==0) return NULL;
    mException(flag==SOCKET_ERROR,EXIT,"tcp error");

    if(memcmp(handle->client_addr+n,&addr,4)!=0) return NULL;
    send(handle->client_socket[n],data,size,0);
    
    uint8_t *s=(uint8_t *)(&(ADDR(handle->server_addr.sin_addr)));
    sprintf(handle->addr_str,"%d.%d.%d.%d:%d",s[0],s[1],s[2],s[3],handle->server_port);
    return handle->addr_str;
}
*/

#endif
