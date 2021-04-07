/*
// gcc -O2 -fopenmp test51_2.c -o test51.exe -I ../../include/ -L ../../lib/x64/gnu/ -lmorn -ljpeg -lpng -lz -lm
// scp test51_2.c nvidia@192.168.14.88:/home/nvidia/qyli
*/
#define STRSAFE_NO_DEPRECATE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "morn_image.h"

#if defined MORN_USE_USBCAMERA

#define HASH_USBCamera 0x43aaadbc

#if defined(__linux__)
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

extern short v_to_r[256];
extern short u_to_g[256];
extern short v_to_g[256];
extern short u_to_b[256];

void YUV422ToRGB(unsigned char *data,MImage *img)
{
    int n=0;
    for(int j=0;j<img->height;j++)for(int i=0;i<img->width;i+=2)
    {
        unsigned char y1=data[n  ];
        unsigned char u =data[n+1];
        unsigned char y2=data[n+2];
        unsigned char v =data[n+3];
        n+=4;

        int r,g,b;
        int r0,g0,b0;
        
        r0=v_to_r[v]; g0=0-u_to_g[u]-v_to_g[v]; b0=u_to_b[u];
        r=r0+y1;if(r<0){r=0;} else if(r>255){r=255;}
        g=g0+y1;if(g<0){g=0;} else if(g>255){g=255;}
        b=b0+y1;if(b<0){b=0;} else if(b>255){b=255;}
        
        img->data[0][j][i  ]=b;img->data[1][j][i  ]=g;img->data[2][j][i  ]=r;

        r=r0+y2;if(r<0){r=0;} else if(r>255){r=255;}
        g=g0+y2;if(g<0){g=0;} else if(g>255){g=255;}
        b=b0+y2;if(b<0){b=0;} else if(b>255){b=255;}
        
        img->data[0][j][i+1]=b;img->data[1][j][i+1]=g;img->data[2][j][i+1]=r;
    }
}

#define ioctl(Fd,Cmd,Pointer) mException((ioctl(Fd,Cmd,Pointer)==-1),EXIT,"error ioctl with %s",#Cmd);

struct HandleUSBCamera
{
    int fd;
    struct v4l2_buffer buf;
    unsigned char *buffer[2];
    int buffer_size;
    int index;
};
void endUSBCamera(struct HandleUSBCamera *handle)
{
    if(handle->fd!=0)
    {
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(handle->fd, VIDIOC_STREAMOFF, &type);

        munmap(handle->buffer[0],handle->buffer_size);
        munmap(handle->buffer[1],handle->buffer_size);
    
        close(handle->fd);
    }
}

void mUSBCamera(MImage *img,const char *devicename)
{
    int height,width;
    
    MHandle *hdl =mHandle(img,USBCamera);
    struct HandleUSBCamera *handle = (struct HandleUSBCamera *)(hdl->handle);
    if(hdl->valid == 0)
    {
        char device[16];
        struct v4l2_format fmt;
        struct v4l2_capability cap;

        int length = strlen(devicename);
        
        for(int i=0;i<4;i++)
        {
            sprintf(device,"/dev/video%d",i);
            handle->fd = open(device,O_RDWR);
            // printf("filename=%s,fd=%d\n",device,handle->fd);
            if(handle->fd==-1) continue;

            ioctl(handle->fd, VIDIOC_QUERYCAP, &cap);
            printf("device name is %s\n",cap.card);
            if(devicename==NULL) break;
            else if(memcmp((const char*)(cap.card),devicename,length)==0) break;
            
            handle->fd=-1;
        }
        mException((handle->fd==-1),EXIT,"cannot open device %s",devicename);

        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        
        fmt.type = type;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
        if(img->height>0) fmt.fmt.pix.height= img->height;
        if(img->width >0) fmt.fmt.pix.width = img->width ;
        ioctl(handle->fd,VIDIOC_S_FMT,&fmt);
        
        ioctl(handle->fd,VIDIOC_G_FMT,&fmt);
        mException(fmt.fmt.pix.pixelformat!=V4L2_PIX_FMT_YUYV,EXIT,"cannot support image format YUYV");

        height = fmt.fmt.pix.height;
        width  = fmt.fmt.pix.width ;
        mImageRedefine(img,3,height,width);
        printf("height=%d,width=%d\n",height,width);
            
        struct v4l2_requestbuffers req;
        req.count=2;
        req.type=type;
        req.memory=V4L2_MEMORY_MMAP;
        ioctl(handle->fd,VIDIOC_REQBUFS,&req);

        handle->buffer_size = height*width*2;
        handle->buf.type = type;
        handle->buf.memory = V4L2_MEMORY_MMAP;
        
        handle->buf.index = 0;
        ioctl (handle->fd,VIDIOC_QUERYBUF,&handle->buf);
        handle->buffer[0] = (unsigned char *)mmap(NULL,handle->buf.length,PROT_READ|PROT_WRITE, MAP_SHARED,handle->fd,handle->buf.m.offset);
        mException(handle->buffer[0]==MAP_FAILED,EXIT,"memory map error");
        ioctl(handle->fd, VIDIOC_QBUF, &handle->buf);
        
        handle->buf.index = 1;
        ioctl (handle->fd,VIDIOC_QUERYBUF,&handle->buf);
        handle->buffer[1] = (unsigned char *)mmap(NULL,handle->buf.length,PROT_READ|PROT_WRITE, MAP_SHARED,handle->fd,handle->buf.m.offset);
        mException(handle->buffer[1]==MAP_FAILED,EXIT,"memory map error");
        ioctl(handle->fd, VIDIOC_QBUF, &handle->buf);
        printf("handle->buf.length=%d,handle->buffer_size=%d\n",handle->buf.length,handle->buffer_size);

        ioctl(handle->fd, VIDIOC_STREAMON, &type);

        handle->index = 0;
        hdl->valid =1;
    }
    handle->buf.index = handle->index;
    ioctl(handle->fd,VIDIOC_DQBUF,&handle->buf);
    YUV422ToRGB(handle->buffer[handle->index],img);
    ioctl(handle->fd,VIDIOC_QBUF ,&handle->buf);
    handle->index = 1-handle->index;
}

#elif defined(_WIN64)||defined(_WIN32)

#include <dshow.h>

extern "C" { 
    extern GUID CLSID_SampleGrabber;
    extern GUID CLSID_NullRenderer; 
}

interface ISampleGrabber:public IUnknown
{
	virtual HRESULT STDMETHODCALLTYPE SetOneShot( BOOL OneShot ) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMediaType( const AM_MEDIA_TYPE *pType ) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( AM_MEDIA_TYPE *pType ) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( BOOL BufferThem ) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( long *pBufferSize, long *pBuffer ) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( IMediaSample **ppSample ) = 0;
	// virtual HRESULT STDMETHODCALLTYPE SetCallback( ISampleGrabberCB *pCallback, long WhichMethodToCallback ) = 0;
};

static const IID IID_ISampleGrabber = { 0x6B652FFF, 0x11FE, 0x4fce, { 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };

struct HandleUSBCamera
{
    char camera_name[128];
    int height;
    int width;
    long int image_size;
    
    IGraphBuilder  *m_pGraph;
    IMediaControl  *m_pMediaControl;
    IMediaEvent    *m_pMediaEvent;
    IBaseFilter    *m_pDeviceFilter;
    IBaseFilter    *m_pGrabber;
    ISampleGrabber *m_pSampleGrabber;
    IBaseFilter    *m_pNullFilter;
    IPin           *m_pCameraOutput;
    IPin           *m_pGrabberInput;
    IPin           *m_pGrabberOutput;
    IPin           *m_pNullInputPin;
    
    unsigned char  *image_data;
};

void CameraOpen(struct HandleUSBCamera *handle)
{
    CoInitialize(NULL);

    ICreateDevEnum *pDevEnum=NULL;
    mException(CoCreateInstance(CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC_SERVER,IID_ICreateDevEnum,(void**)(&pDevEnum)),EXIT,"cannot get IDeviceFilter");

    IEnumMoniker *pEnumMonike = NULL;
    mException(pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEnumMonike, 0),EXIT,"cannot get IDeviceFilter");
    pEnumMonike->Reset();
    IMoniker *pMoniker = NULL;
    IPropertyBag *pPropBag=NULL;
    while(pEnumMonike->Next(1,&pMoniker,NULL)==0)
    {
        mException(pMoniker->BindToStorage(0, 0, IID_IPropertyBag,(void**)(&pPropBag)),EXIT,"cannot get camera");
        VARIANT varName;VariantInit(&varName);

        char cameraname[256];
        pPropBag->Read(L"FriendlyName", &varName, 0);
        int i;for(i=0;varName.bstrVal[i]!=0;i++){cameraname[i]=varName.bstrVal[i];}cameraname[i]=0;
        printf("cameraname=%s\n",cameraname);
        if(handle->camera_name[0]==0) strcpy(handle->camera_name,cameraname);
        if(strcmp(cameraname,handle->camera_name)==0) break;
    }
    
    mException(pMoniker->BindToObject(NULL,NULL,IID_IBaseFilter,(void**)&(handle->m_pDeviceFilter)),EXIT,"cannot get IDeviceFilter");
    pPropBag->Release();
    // IPropertyBag_Release(pPropBag);
    pMoniker->Release();
    pEnumMonike->Release();
    pDevEnum->Release();
    
    mException(CoCreateInstance(CLSID_FilterGraph  ,NULL,CLSCTX_INPROC       ,IID_IGraphBuilder,(void **)&(handle->m_pGraph     )),EXIT,"cannot get IGraphBuilder" );
    mException(CoCreateInstance(CLSID_SampleGrabber,NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,  (void **)&(handle->m_pGrabber   )),EXIT,"cannot get ISampleGrabber");
    mException(CoCreateInstance(CLSID_NullRenderer ,NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,  (void **)&(handle->m_pNullFilter)),EXIT,"cannot get INullFilter"   );

    mException(handle->m_pGraph->QueryInterface(IID_IMediaControl,(void **)&(handle->m_pMediaControl)),EXIT,"cannot get IMediaControl");
    mException(handle->m_pGraph->QueryInterface(IID_IMediaEvent  ,(void **)&(handle->m_pMediaEvent  )),EXIT,"cannot get IMediaEvent"  );

    mException(handle->m_pGraph->AddFilter(handle->m_pDeviceFilter,NULL           ),EXIT,"error at Filter Graph");
    mException(handle->m_pGraph->AddFilter(handle->m_pGrabber     ,L"Grabber"     ),EXIT,"error at Filter Graph");
    mException(handle->m_pGraph->AddFilter(handle->m_pNullFilter  ,L"NullRenderer"),EXIT,"error at Filter Graph");

    IEnumPins *pEnumPins;
    
    pEnumPins=NULL;
    handle->m_pDeviceFilter->EnumPins(&pEnumPins);pEnumPins->Reset();
    mException(pEnumPins->Next(1, &(handle->m_pCameraOutput), NULL),EXIT,"cannot find Camera Output pin");
    IAMStreamConfig *m_iconfig=NULL; 
    mException(handle->m_pCameraOutput->QueryInterface(IID_IAMStreamConfig,(void**)&m_iconfig),EXIT,"cannot config stream");
    AM_MEDIA_TYPE *pmt=NULL;
    mException(m_iconfig->GetFormat(&pmt)       ,EXIT,"cannot get MEDIA TYPE");
    mException(pmt->formattype!=FORMAT_VideoInfo,EXIT,"cannot get MEDIA TYPE");
    VIDEOINFOHEADER *phead=(VIDEOINFOHEADER*)(pmt->pbFormat);
    // printf("phead->bmiHeader.biWidth=%d\n",phead->bmiHeader.biWidth);
    // printf("phead->bmiHeader.biHeight=%d\n",phead->bmiHeader.biHeight);
    if(handle->height<=0) handle->height=phead->bmiHeader.biHeight;
    if(handle->width <=0) handle->width =phead->bmiHeader.biWidth ;
    if((handle->height!=phead->bmiHeader.biHeight)||(handle->width !=phead->bmiHeader.biWidth ))
    {
        phead->bmiHeader.biWidth = handle->width ;
        phead->bmiHeader.biHeight= handle->height;
        mException(m_iconfig->SetFormat(pmt),EXIT,"error at set image size");
    }
    m_iconfig->Release();

    mException(handle->m_pGrabber->QueryInterface(IID_ISampleGrabber,(void**)&(handle->m_pSampleGrabber)),EXIT,"cannot get ISampleGrabber");
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt,sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB24;
    mt.formattype = FORMAT_VideoInfo; 
    mException(handle->m_pSampleGrabber->SetMediaType(&mt),EXIT,"cannot SetMediaType");
    pEnumPins = NULL; 
    handle->m_pGrabber->EnumPins(&pEnumPins);pEnumPins->Reset();
    mException(pEnumPins->Next(1,&(handle->m_pGrabberInput ),NULL),EXIT,"cannot find Grabber Input pin");
    mException(pEnumPins->Next(1,&(handle->m_pGrabberOutput),NULL),EXIT,"cannot find Grabber Output pin");

    pEnumPins=NULL;
    handle->m_pNullFilter->EnumPins(&pEnumPins);pEnumPins->Reset();
    mException(pEnumPins->Next(1,&(handle->m_pNullInputPin),NULL),EXIT,"cannot find pNullInputPin");
    
    pEnumPins->Release();

    mException(handle->m_pGraph->Connect(handle->m_pCameraOutput ,handle->m_pGrabberInput),EXIT,"error at Filter Graph");
    mException(handle->m_pGraph->Connect(handle->m_pGrabberOutput,handle->m_pNullInputPin),EXIT,"error at Filter Graph");

    handle->m_pSampleGrabber->SetBufferSamples(TRUE);
    handle->m_pSampleGrabber->SetOneShot(TRUE);
    
    handle->m_pMediaControl->Run();
    long int evCode;
    handle->m_pMediaEvent->WaitForCompletion(1000, &evCode);
    // printf("evCode=%d,EC_COMPLETE=%d\t",evCode,EC_COMPLETE);

    handle->m_pSampleGrabber->GetCurrentBuffer(&(handle->image_size),NULL);
    // printf("handle->image_size=%d\n",handle->image_size);
    mException((handle->image_size!=handle->height*handle->width*3),EXIT,"invalid image size");
}

void CameraClose(struct HandleUSBCamera *handle)
{
    handle->m_pMediaControl->Stop();
    
    if(handle->m_pNullFilter   !=NULL) handle->m_pNullFilter   ->Release();
    if(handle->m_pSampleGrabber!=NULL) handle->m_pSampleGrabber->Release();
    if(handle->m_pGrabber      !=NULL) handle->m_pGrabber      ->Release();
    if(handle->m_pDeviceFilter !=NULL) handle->m_pDeviceFilter ->Release();
    if(handle->m_pMediaEvent   !=NULL) handle->m_pMediaEvent   ->Release();
    if(handle->m_pMediaControl !=NULL) handle->m_pMediaControl ->Release();
    if(handle->m_pGraph        !=NULL) handle->m_pGraph        ->Release();
    if(handle->m_pCameraOutput !=NULL) handle->m_pCameraOutput ->Release();
    if(handle->m_pGrabberInput !=NULL) handle->m_pGrabberInput ->Release();
    if(handle->m_pGrabberOutput!=NULL) handle->m_pGrabberOutput->Release();
    if(handle->m_pNullInputPin !=NULL) handle->m_pNullInputPin ->Release();

    CoUninitialize();
}

void endUSBCamera(void *info)
{
    struct HandleUSBCamera *handle = (struct HandleUSBCamera *)info;
    CameraClose(handle);
    if(handle->image_data!=NULL) mFree(handle->image_data);
}
void mUSBCamera(MImage *img,const char *camera_name)
{
    mException(INVALID_POINTER(img),EXIT,"invalid input");

    mTimerBegin();
    MHandle *hdl = mHandle(img,USBCamera);
    struct HandleUSBCamera *handle = (struct HandleUSBCamera *)(hdl->handle);
    sprintf(handle->camera_name,camera_name);
    if(hdl->valid == 0)
    {
        handle->height= img->height;handle->width= img->width;
        CameraOpen(handle);

        handle->image_data = (unsigned char *)mMalloc(handle->image_size);
        hdl->valid =1;
    }
    mImageRedefine(img,3,handle->height,handle->width);
    mTimerEnd();
    
    mTimerBegin();
    handle->m_pSampleGrabber->GetCurrentBuffer(&(handle->image_size),(long *)(handle->image_data));
    mTimerEnd();
    // printf("size=%d\n",handle->image_size);
    mTimerBegin();
    mImageDataInputU8(img,handle->image_data,DFLT,NULL,NULL);
    mTimerEnd();
}

#endif
#endif

// int main()
// {
//     MImage *img = mImageCreate();
//     for(int i=0;i<10;i++)
//     {
//         USBCamera(img,NULL);
//         mImageSave(img,"./img/test51_%d.jpg",i);
//     }
//     mImageRelease(img);
// }

// int main()
// {
//     MImage *img = mImageCreate(3,720,1280);
//     for(int i=0;i<10;i++)
//     {
//         printf("aaaaaaaaaaaaa\n");
//         // mTimerBegin();
//         USBCamera(img,NULL);//"BudeBuai-6Plus");
//         // mTimerEnd();
//         mImageSave(img,"./test41_%d.png",i);
//     }
//     mImageRelease(img);
//     return 0;
// }
