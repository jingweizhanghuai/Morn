/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_wave.h"

#define COMPLEXADD(re0,im0,re1,im1,re_out,im_out) {re_out = re0+re1;im_out = im0+im1;}
#define COMPLEXSUB(re0,im0,re1,im1,re_out,im_out) {re_out = re0-re1;im_out = im0-im1;}
#define COMPLEXMUL(re0,im0,re1,im1,re_out,im_out) {re_out = re0*re1-im0*im1;im_out = im0*re1+re0*im1;}

#define FFTCACL(re0,im0,re1,im1) {\
    register float re_mul,im_mul;\
    COMPLEXMUL(Wre[k],Wim[k],re1,im1,re_mul,im_mul);\
    COMPLEXSUB(re0,im0,re_mul,im_mul,re1,im1);\
    COMPLEXADD(re0,im0,re_mul,im_mul,re0,im0);\
}

struct HandleWaveFFT {
    int size;
    float *Wre;
    float *Wim;
    int *order;
}HandleWaveFFT;
void endWaveFFT(void *info)
{
    struct HandleWaveFFT *handle=info;
    if(handle->Wre != NULL)mFree(handle->Wre);
    if(handle->Wim != NULL)mFree(handle->Wim);
    if(handle->order!=NULL)mFree(handle->order);
}
#define HASH_WaveFFT 0xf197b3ec

void mWaveFFT(MWave *src,MWave *fft)
{
    int i,j,k,n;
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    // mException((mInfoGet(&(src->info),"wave_type") != MORN_WAVE_TD),EXIT,"invalid input");

    int N;
    MHandle *hdl; ObjectHandle(src,WaveFFT,hdl);
    struct HandleWaveFFT *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        mException((src->size<4),EXIT,"invalid input");
        k=1;while(src->size>(2<<k))k=k+1; N=(2<<k);
        if(handle->size != N)
        {
            handle->size = N;
            if(handle->order!=NULL) mFree(handle->order);handle->order=mMalloc(N*sizeof(int));
            N=N>>1;handle->order[0]=0;j=1;
            for(k=N;k>0;k=k>>1) {for(i=0;i<j;i++) handle->order[i+j]=handle->order[i]+k; j=j+j;}
            
            if(handle->Wre!=NULL) mFree(handle->Wre);handle->Wre=mMalloc(N*sizeof(float));
            if(handle->Wim!=NULL) mFree(handle->Wim);handle->Wim=mMalloc(N*sizeof(float));
            
            double n_pi = MORN_PI/((double)N);double thta = n_pi;
            handle->Wre[0] = 1.0f; handle->Wim[0] = 0.0f;
            for(k=1;k<N;k++)
            {
                handle->Wre[k] = (float)cos(thta);
                handle->Wim[k] = 0.0f-(float)sin(thta);
                thta = thta + n_pi;
            }
        }
        hdl->valid = 1;
    }
    N = handle->size;
    float *Wre = handle->Wre; float *Wim = handle->Wim;

    MWave *p=fft;
    if((fft==NULL)||(fft == src)) fft = mWaveCreate(((src->channel)<<1),N,NULL);
    else                          mWaveRedefine(fft,((src->channel)<<1),N,fft->data);
    fft->info = src->info;
    mInfoSet(&(fft->info),"wave_type",MORN_WAVE_FD);
    mInfoSet(&(fft->info),"normalize",MORN_NOT_NORMALIZED);
    
    N=(N>>1);
    for(int cn=0;cn<src->channel;cn++)
    {
        float *FFTDataRe = fft->data[(cn<<1)];
        float *FFTDataIm = fft->data[(cn<<1)+1];
        float *src_data = src->data[cn];
        
        for(i=0;i<N+N;i+=4)
        {
            int n0=handle->order[i  ];int n1=handle->order[i+1];
            int n2=handle->order[i+2];int n3=handle->order[i+3];
            float d0=src_data[n0];float d1=(n1>=src->size)?0:src_data[n1];
            float d2=src_data[n2];float d3=(n3>=src->size)?0:src_data[n3];
            FFTDataRe[i  ]=d0+d1+d2+d3;FFTDataRe[i+1]=d0-d1;FFTDataRe[i+2]=d0+d1-d2-d3;FFTDataRe[i+3]=d0-d1;
            FFTDataIm[i  ]=0;          FFTDataIm[i+1]=d3-d2;FFTDataIm[i+2]=0;          FFTDataIm[i+3]=d2-d3;
        }
        
        for(n=4;n<=N;n=(n<<1))
            for(j=0;j<(N<<1);j=j+(n<<1))
                for(i=0,k=0;i<n;i++,k=k+N/n)
                    FFTCACL(FFTDataRe[j+i],FFTDataIm[j+i],FFTDataRe[j+i+n],FFTDataIm[j+i+n]);
    }
    
    if(p!=fft) {mWaveExchange(src,fft);mWaveRelease(fft);}
}

#define HandleWaveIFFT HandleWaveFFT
#define endWaveIFFT endWaveFFT
#define HASH_WaveIFFT 0x81f00b75
void mWaveIFFT(MWave *fft,MWave *dst)
{
    int i,j,k,n;
    mException((INVALID_WAVE(fft)),EXIT,"invalid input");
    mException((mInfoGet(&(fft->info),"wave_type") != MORN_WAVE_FD),EXIT,"invalid input");

    int N;
    MHandle *hdl; ObjectHandle(fft,WaveIFFT,hdl);
    struct HandleWaveIFFT *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        N=fft->size;mException((N<4)||((N&(N-1))!=0),EXIT,"invalid input");
        if(handle->size != N)
        {
            handle->size = N;
            if(handle->order!=NULL) mFree(handle->order);handle->order=mMalloc(N*sizeof(int));
            N=N>>1;handle->order[0]=0;j=1;
            for(k=N;k>0;k=k>>1) {for(i=0;i<j;i++) handle->order[i+j]=handle->order[i]+k; j=j+j;}
            
            if(handle->Wre!=NULL) mFree(handle->Wre);handle->Wre=mMalloc(N*sizeof(float));
            if(handle->Wim!=NULL) mFree(handle->Wim);handle->Wim=mMalloc(N*sizeof(float));
            
            double n_pi = MORN_PI/((double)N);double thta = n_pi;
            handle->Wre[0] = 1.0f; handle->Wim[0] = 0.0f;
            for(k=1;k<N;k++)
            {
                handle->Wre[k] = (float)cos(thta);
                handle->Wim[k] = (float)sin(thta);
                thta = thta + n_pi;
            }
        }
        hdl->valid = 1;
    }
    N = handle->size;
    float *Wre = handle->Wre; float *Wim = handle->Wim;

    MWave *p=dst;
    if((dst==NULL)||(dst==fft)) dst = mWaveCreate(fft->channel,N,NULL);
    else                        mWaveRedefine(dst,fft->channel,N,dst->data);
    dst->info = fft->info;
    mInfoSet(&(dst->info),"wave_type",MORN_WAVE_TD);
    mInfoSet(&(dst->info),"normalize",MORN_NOT_NORMALIZED);
    
    N=(N>>1);
    for(int cn=0;cn<dst->channel;cn+=2)
    {
        float *FFTDataRe = dst->data[cn];
        float *FFTDataIm = dst->data[cn+1];
        float *fft_data_re=fft->data[cn];
        float *fft_data_im=fft->data[cn+1];

        for(i=0;i<N+N;i++)
        {
            int n=handle->order[i];
            FFTDataRe[i]=fft_data_re[n];FFTDataIm[i]=fft_data_im[n];
        }
        
        for(n=1;n<=N;n=(n<<1))
            for(j=0;j<(N<<1);j=j+(n<<1))
                for(i=0,k=0;i<n;i++,k=k+N/n)
                    FFTCACL(FFTDataRe[j+i],FFTDataIm[j+i],FFTDataRe[j+i+n],FFTDataIm[j+i+n]);
       
        for(i=0;i<N+N;i++) dst->data[cn>>1][i] = FFTDataRe[i]/((float)(N+N));
    }
    dst->channel=dst->channel>>1;
    if(p!=dst) {mWaveExchange(fft,dst);mWaveRelease(dst);}
}


/*
void mWaveIFFT0(MWave *fft,MWave *dst)
{    
    int i,j,k,n;
    
    int cn;
    
    int wave_size;
    
    double *DstDataRe;
    double *DstDataIm;
   
    int N;
    
    double *Wre,*Wim;
    
    int out_valid;
    
    mException((INVALID_WAVE(fft)),EXIT,"invalid input");
    mException((mInfoGet(&(fft->info),"wave_type") != MORN_WAVE_FD),EXIT,"invalid input");
    wave_size = fft->size;
    
    N = wave_size;
    while((N&0x01)==0)
        N = N>>1;
    mException((N!=1),EXIT,"invalid input data");
    
    N = wave_size;
    
    if((INVALID_POINTER(dst))||(dst == fft))
    {
        out_valid = 0;
        dst = mWaveCreate(((fft->channel)>>1),N,NULL);
    }
    else
    {
        out_valid = 1;
        mWaveRedefine(dst,((fft->channel)>>1),N,dst->data);
    }
    dst->info = fft->info;
    mInfoSet(&(dst->info),"wave_type",MORN_WAVE_TD);
    
    DstDataRe = (double *)mMalloc(N*sizeof(double));
    DstDataIm = (double *)mMalloc(N*sizeof(double));
    
    N=(N>>1);
    
    Wre = (double *)mMalloc(N*sizeof(double));
    Wim = (double *)mMalloc(N*sizeof(double));
    
    for(k=0;k<N;k++)
    {
        Wre[k] = cos((((double)(k))/((double)(N)))*MORN_PI);
        Wim[k] = sin((((double)(k))/((double)(N)))*MORN_PI);
    }
    
    for(cn=0;cn<fft->channel;cn=cn+2)
    {
        DstDataRe[0] = fft->data[cn][0];
        DstDataRe[N] = fft->data[cn][1];
        DstDataIm[0] = fft->data[cn+1][0];
        DstDataIm[N] = fft->data[cn+1][1];
        for(i=1,j=N;i<N;i++)  
        {
            printf("i is %d,j is %d\n",i,j);
            printf("i is %d,j is %d\n",i+N,j+1);
            DstDataRe[i]   = fft->data[cn][j];
            DstDataRe[i+N] = fft->data[cn][j+1];
            DstDataIm[i]   = fft->data[cn+1][j];
            DstDataIm[i+N] = fft->data[cn+1][j+1];
            
            k=N;  
            while(k<=j)  
            {
                j=j-k;  
                k=k/2;  
            }
            j=j+k;  
        }
        for(i=0;i<N+N;i++)
            printf("data is %f+%fi\n",DstDataRe[i],DstDataIm[i]);
        
        for(n=1;n<=N;n=(n<<1))
        {
            //#pragma omp parallel for
            for(j=0;j<(N<<1);j=j+(n<<1))
                for(i=0,k=0;i<n;i++,k=k+N/n)
                    FFTCACL(DstDataRe[j+i],DstDataIm[j+i],DstDataRe[j+i+n],DstDataIm[j+i+n]);
        }
        
        for(i=0;i<wave_size;i++)
            dst->data[cn>>1][i] = (float)(DstDataRe[i]/((double)wave_size));
    }
    
    mFree(Wre);
    mFree(Wim);
    mFree(DstDataRe);
    mFree(DstDataIm);
    
    if(!out_valid)
    {
        mWaveExchange(fft,dst);
        mWaveRelease(dst);
    }
}
*/

void mWavePowerSpectrum(MWave *fft,MWave *ps,int mode)
{
    int wav_size;
    float *re,*im;
    float *ps_data;
    int i,j;
    
    if(mode == MORN_DEFAULT)
        mode = MORN_SQUAR_POWERS;
    mException(((mode<1)||(mode>3)),EXIT,"invalid input");
    mException((INVALID_WAVE(fft)),EXIT,"invalid input");
    mException((mInfoGet(&(fft->info),"wave_type") != MORN_WAVE_FD),EXIT,"invalid input");
    
    wav_size = (fft->size)>>1;   
    
    if(INVALID_POINTER(ps))
        ps = fft;
    
    mWaveRedefine(ps,((fft->channel)>>1),wav_size,ps->data);
    ps->info = fft->info;
    mInfoSet(&(ps->info),"wave_type",MORN_WAVE_PS);
    mInfoSet(&(ps->info),"normalize",MORN_NOT_NORMALIZED);
    
    if(mode == MORN_SQUAR_POWERS)
    {
        for(j=0;j<ps->channel;j++)
        {
            re = fft->data[(j<<1)];
            im = fft->data[(j<<1)+1];
            ps_data = ps->data[j];
            for(i=0;i<wav_size;i++)
                ps_data[i] = re[i]*re[i] + im[i]*im[i];
        }
    }
    else if(mode == MORN_POWERS)
    {
        for(j=0;j<ps->channel;j++)
        {
            re = fft->data[(j<<1)];
            im = fft->data[(j<<1)+1];
            ps_data = ps->data[j];
            for(i=0;i<wav_size;i++)
                ps_data[i] = (float)sqrt((double)(re[i]*re[i] + im[i]*im[i]));
        }
    }
    else if(mode == MORN_LOG_POWERS)
    {
        for(j=0;j<ps->channel;j++)
        {
            re = fft->data[(j<<1)];
            im = fft->data[(j<<1)+1];
            ps_data = ps->data[j];
            for(i=0;i<wav_size;i++)
                ps_data[i] = (log10((double)(re[i]*re[i] + im[i]*im[i])))/2.0;
        }
    }    
}

void mWaveFrequencyComponent(MWave *src,float frequency,float *component)
{
    float src_frequency = mInfoGet(&(src->info),"frequency");
    float c = (MORN_PI+MORN_PI)*frequency/src_frequency;
    for(int cn=0;cn<src->channel;cn++)
    {
        float *data = src->data[cn];
        
        float e = 0;
        float re = data[0];
        float im = 0.0f;
        for(int i=1;i<src->size;i++)
        {
            e = e+c;
            re = re + data[i]*cos(e);
            im = im - data[i]*sin(e);
        }
        
        component[cn] =re*re+im*im;
    }
}

struct HandleWaveFrequencyAnalyse
{
    int src_frequency;
    
    int num;
    float *frequency;
    
    MMatrix *re_mat;
    MMatrix *im_mat;
}HandleWaveFrequencyAnalyse;
#define HASH_WaveFrequencyAnalyse 0x77f2456d
void endWaveFrequencyAnalyse(void *info) 
{
    struct HandleWaveFrequencyAnalyse *handle = info;
    
    if(handle->frequency!=NULL) mFree(handle->frequency);
    if(handle->re_mat != NULL) mMatrixRelease(handle->re_mat);
    if(handle->im_mat != NULL) mMatrixRelease(handle->im_mat);
}
void mWaveFrequencyAnalyse(MWave *src,float *frequency,int num,float **component)
{
    int cn,i,j;
    MHandle *hdl; ObjectHandle(src,WaveFrequencyAnalyse,hdl);
    struct HandleWaveFrequencyAnalyse *handle = hdl->handle;
    if(hdl->valid ==1)
    {
        if(num <=0) num = handle->num;
        if(INVALID_POINTER(frequency)) frequency = handle->frequency;
        
        float src_frequency=mInfoGet(&(src->info),"frequency");
        if((handle->src_frequency != src_frequency)&&(src_frequency >0))
            hdl->valid = 0;
        else if(frequency != handle->frequency)
            if(memcmp(handle->frequency,frequency,num*sizeof(float))!=0)
                hdl->valid = 0;
    }
    if(hdl->valid == 0)
    {
        mException((num<=0)||(INVALID_POINTER(frequency)),EXIT,"invalid input");
        handle->src_frequency = mInfoGet(&(src->info),"frequency");
        mException((handle->src_frequency<=0),EXIT,"invalid input");
        
        if(num>handle->num) {mFree(handle->frequency);handle->frequency=NULL;}
        if(handle->frequency==NULL) handle->frequency=mMalloc(num*sizeof(float));
        handle->num = num;
    
        if(handle->re_mat == NULL) handle->re_mat = mMatrixCreate(num,src->size,NULL);
        else                       mMatrixRedefine(handle->re_mat,num,src->size,NULL);
        if(handle->im_mat == NULL) handle->im_mat = mMatrixCreate(num,src->size,NULL);
        else                       mMatrixRedefine(handle->im_mat,num,src->size,NULL);
        
        memcpy(handle->frequency,frequency,num*sizeof(float));
        
        for(j=0;j<num;j++)
        {
            double c = ((double)(MORN_PI+MORN_PI)*(handle->frequency[j]))/((double)(handle->src_frequency));
            double e = 0.0;
            handle->re_mat->data[j][0] = 1.0f;
            handle->im_mat->data[j][0] = 0.0f;
            for(i=1;i<src->size;i++)
            {
                e = e+c;
                handle->re_mat->data[j][i] = cos(e);
                handle->im_mat->data[j][i] = 0.0f - sin(e);
            }
        }
        
        hdl->valid = 1;
    }
    
    MMatrix *re_mat = handle->re_mat;
    MMatrix *im_mat = handle->im_mat;
    for(cn=0;cn<src->channel;cn++)
    {
        for(j=0;j<num;j++)
        {
            float re = 0.0f;
            float im = 0.0f;
            for(i=0;i<src->size;i++)
            {
                re = re + src->data[cn][i]*re_mat->data[j][i];
                im = im + src->data[cn][i]*im_mat->data[j][i];
            }
            component[cn][j] =re*re+im*im;
        }
    }
}

/*
void mWaveFrequencyAnalyse2(MWave *src,float *frequency,int num,float **component)
{
    float *e,*c;
    int i,j,cn;
    float *re,*im;
    
    float *data;
    
    e = (float *)mMalloc(num*sizeof(float));
    c = (float *)mMalloc(num*sizeof(float));
    re = (float *)mMalloc(num*sizeof(float));
    im = (float *)mMalloc(num*sizeof(float));

    for(cn=0;cn<src->channel;cn++)
    {
        data = src->data[cn];
        
        for(j=0;j<num;j++)
        {
            c[j] = (MORN_PI+MORN_PI)*frequency[j]/((float)(src->info.frequency));
            e[j] = 0;
            re[j] = data[0];
            im[j] = 0.0;
        }
        
        for(i=1;i<src->size;i++)
            for(j=0;j<num;j++)
            {
                e[j] = e[j]+c[j];
                re[j] = re[j] + data[i]*cos(e[j]);
                im[j] = im[j] - data[i]*sin(e[j]);
            }
        
        for(j=0;j<num;j++)
        {
            // printf("re[j] is %f,im[j] is %f\n",re[j],im[j]);
            component[cn][j] =re[j]*re[j]+im[j]*im[j];
        }
    }
    
    mFree(e);
    mFree(c);
    mFree(re);
    mFree(im);
}
*/



