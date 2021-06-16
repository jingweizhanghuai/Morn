// gcc -O2 -fopenmp test_cl2.c -o test_cl2.exe -DMORN_USE_CL -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -lopenblas -ljpeg -lpng -lz -lclblas -lOpenCL -lstdc++
#include "morn_image.h"
#include "morn_tensor.h"

char *tensor_to_mat0 = mString(
__kernel void tensor_to_mat(__global const float* tns,__global float* mat,const int cn,const int h,const int w,const int height,const int width)
{
    const int mheight= get_global_size(0);
    const int mwidth = get_global_size(1);
    const int msize  = mheight*mwidth;
    const int sy=height/mheight;
    const int sx=width /mwidth ;
    const int size= height*width;
    const int y = get_global_id(0);
    const int x = get_global_id(1);
    const int idx = (y*mwidth+x);
    const int y0=y*sy-h/2;
    const int x0=x*sx-w/2;
    int n=0;
    for(int c=0;c<cn;c++)for(int j=y0;j<y0+h;j++)for(int i=x0;i<x0+w;i++)
    {
        if((j<0)||(j>=height)||(i<0)||(i>=width))
            mat[idx+n]=0;
        else
            mat[idx+n]=tns[c*size+j*width+i];
        n+=msize;
    }
    mat[idx+n]=1.0;
});

char *mat_to_tensor0 = mString(
__kernel void mat_to_tensor0(__global const float* mat,__global float* tns,const int cn,const int h,const int w,const int height,const int width,const int l)
{
    const int mheight= get_global_size(0);
    const int mwidth = get_global_size(1);
    const int msize  = mheight*mwidth;
    const int sy=height/mheight;
    const int sx=width /mwidth ;
    const int size= height*width;
    const int y = get_global_id(0);
    const int x = get_global_id(1);
    const int idx = (y*mwidth+x);
    const int j=y*sy-h/2+(l/w);
    const int i=x*sx-w/2+(l%w);
    int n=l*msize;
    if((j>=0)&&(j<height)&&(i>=0)&&(i<width))
    {
        for(int c=0;c<cn;c++)
        {
            tns[c*size+j*width+i]+=mat[idx+n];
            n+=(w*h*msize);
        }
    }
});

void ConvTensorToMatData(MTensor *tns,int bc,MMemoryBlock *mat,int knl_height,int knl_width,int y_stride,int x_stride);
void ConvMatDataToTensor(MMemoryBlock *mat,MTensor *tns,int bc,int knl_height,int knl_width,int y_stride,int x_stride);

int main()
{
    MImage *img = mImageCreate();
    mImageLoad(img,"./test_cl2.png");
    int height=img->height;
    int width =img->width;
    int cn    =img->channel;
    int w=3,h=3;
    int sx=1,sy=1;
    int ksize=w*h*cn+1;
    int size  =(height/sy)*(width/sx);
    
    MTensor *tns = mTensorCreate(1,cn,height,width,NULL,MORN_CL_GPU(0));
    
    for(int c=0;c<cn;c++)for(int j=0;j<height;j++)for(int i=0;i<width;i++) tns->data[0][c*height*width+j*width+i]=img->data[c][j][i];
    mImageRelease(img);
    
    MMemoryBlock *mat1 = mMemoryBlockCreate(size*ksize*sizeof(float),MORN_CL_GPU(0));
    float *m1=mat1->data;
    MMemoryBlock *mat2 = mMemoryBlockCreate(size*ksize*sizeof(float),MORN_HOST);
    float *m2=mat2->data;
    
    for(int n=0;n<1;n++)
    {
        mTimerBegin("gpu");
        mCLFunction(tensor_to_mat0,CLSIZE(height/sy,width/sx),CLIN(mTensorMemory(tns,0)),CLOUT(mat1),CLPARA(&cn,sizeof(int)),CLPARA(&h,sizeof(int)),CLPARA(&w,sizeof(int)),CLPARA(&height,sizeof(int)),CLPARA(&width,sizeof(int)));
        mTimerEnd("gpu");
        mMemoryBlockRead(mat1);
        printf("\n  0: ");for(int i=  0;i<ksize*size;i+=size) printf("%d,",(int)m1[i]);
        printf("\n423: ");for(int i=423;i<ksize*size;i+=200) printf("%d,",(int)m1[i]);
        printf("\n");

        mTimerBegin("cpu");
        ConvTensorToMatData(tns,0,mat2,h,w,sy,sx);
        mTimerEnd("cpu");
        printf("\n  0: ");for(int i=  0;i<ksize*size;i+=size) printf("%d,",(int)m2[i]);
        printf("\n423: ");for(int i=423;i<ksize*size;i+=200) printf("%d,",(int)m2[i]);
        printf("\n");

        int j=10;
        for(int c=0;c<cn;c++)for(int j=0;j<height;j++)for(int i=0;i<width;i++) tns->data[0][c*height*width+j*width+i]=0;
        mTimerBegin("cpu");
        ConvMatDataToTensor(mat2,tns,0,h,w,sy,sx);
        mTimerEnd("cpu");
        for(int i=0;i<width;i++) printf("%f,%f,%f\n",tns->data[0][j*height+i],tns->data[0][width*height+j*height+i],tns->data[0][width*height*2+j*height+i]);
        printf("\n");
        
        for(int c=0;c<cn;c++)for(int j=0;j<height;j++)for(int i=0;i<width;i++) tns->data[0][c*height*width+j*width+i]=0;
        MMemoryBlock *m_tns = mTensorMemory(tns,0);
        m_tns->flag = MORN_HOST;
        mMemoryBlockWrite(m_tns);
        mTimerBegin("gpu");
        for(int l=0;l<h*w;l++)
            mCLFunction(mat_to_tensor0,CLSIZE(height/sy,width/sx),CLIN(mat1),CLOUT(m_tns),CLPARA(&cn,sizeof(int)),CLPARA(&h,sizeof(int)),CLPARA(&w,sizeof(int)),CLPARA(&height,sizeof(int)),CLPARA(&width,sizeof(int)),CLPARA(&l,sizeof(int)));
        mTimerEnd("gpu");
        mMemoryBlockRead(m_tns);
        for(int i=0;i<width;i++) printf("%f,%f,%f\n",tns->data[0][j*height+i],tns->data[0][width*height+j*height+i],tns->data[0][width*height*2+j*height+i]);
    }

    mMemoryBlockRelease(mat1);
    mMemoryBlockRelease(mat2);
}