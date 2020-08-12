// gcc -O2 -fopenmp test_image_binary.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -ljpeg -lpng -lz -o test_image_binary.exe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Image.h"

int main0()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad("../doc/test_binary.jpg",src);
    
    MImage *bin = mImageCreate(1,src->height,src->width,NULL);
    void operate(unsigned char *in,unsigned char *out,void *para)
    {out[0]=(MIN(MIN(in[0],in[1]),in[2])<128)?255:0;}
    mImageOperate(src,bin,operate,NULL);
    mImageSave(bin,"../doc/test_binary_bin.png");

    MSheet *sheet =mSheetCreate(DFLT,NULL,NULL);
    MList *list = mListCreate(DFLT,NULL);
    mImageBinaryEdge(bin,sheet,list);

    for(int h=0;h<bin->height;h++) memset(bin->data[0][h],0,bin->width*sizeof(unsigned char));
    for(int j=0;j<sheet->row;j++)for(int i=0;i<sheet->col[j];i++)
    {
        MImagePoint *p = (MImagePoint *)(sheet->data[j][i]);
        bin->data[0][(int)(p->y)][(int)(p->x)]=255;
    }
    mImageSave(bin,"../doc/test_binary_edge.png");

    for(int i=0;i<list->num;i++)
        mImageDrawRect(src,src,list->data[i],NULL,3);
    mImageSave(src,"../doc/test_binary_edge_rect.png");

    mImageRelease(src);
    mImageRelease(bin);
    mSheetRelease(sheet);
    mListRelease(list);
    return 0;
}

int main()
{
    int array[256]={0,0,1,1,0,0,1,1,1,1,0,1,1,1,0,1,\
                    1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,1,\
                    0,0,1,1,0,0,1,1,1,1,0,1,1,1,0,1,\
                    1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,1,\
                    1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,\
                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                    1,1,0,0,1,1,0,0,1,1,0,1,1,1,0,1,\
                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,\
                    0,0,1,1,0,0,1,1,1,1,0,1,1,1,0,1,\
                    1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,1,\
                    0,0,1,1,0,0,1,1,1,1,0,1,1,1,0,1,\
                    1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,\
                    1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,\
                    1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,\
                    1,1,0,0,1,1,0,0,1,1,0,1,1,1,0,0,\
                    1,1,0,0,1,1,1,0,1,1,0,0,1,0,0,0 };

    int data[256];
    for(int i=0;i<256;i++)
    {
        int d0=(i&0x80)==0;int d1=(i&0x40)==0;int d2=(i&0x20)==0;int d3=(i&0x10)==0;
        int d4=(i&0x08)==0;int d5=(i&0x04)==0;int d6=(i&0x02)==0;int d7=(i&0x01)==0;

        int rst = d0+(d1<<1)+(d2<<2)+(d3<<3)+(d4<<4)+(d5<<5)+(d6<<6)+(d7<<7);
        printf("%d,",array[rst]);
        data[i]=array[rst];
    }
    printf("\n");
    printf("data[0b00110000] is %d,array is %d\n",data[0b00110000],array[0b11001111]);
    
}

