/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: g++ -O2 -fopenmp test_binary.cpp  -o test_binary.exe -I C:\ProgramFiles\OpenCV\include -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -ljpeg -lpng -lz -L C:\ProgramFiles\OpenCV\lib\mingw -lopencv_world349 -llibjpeg-turbo -llibjasper -lIlmImf -llibtiff  -llibwebp -llibpng -lzlib
#include "morn_image.h"
#include "opencv2/opencv.hpp"
using namespace cv;
using namespace std;

void operate(unsigned char *in,unsigned char *out,void *para)
{
    out[0]=(MIN(MIN(in[0],in[1]),in[2])<128)?255:0;
}
    
int main()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad(src,"../doc/test_binary.jpg");
    
    MImage *bin = mImageCreate(1,src->height,src->width,NULL);
    mImageOperate(src,bin,operate,NULL);
    mImageSave(bin,"../doc/test_binary_bin.png");
    
    Mat binary(bin->height,bin->width,CV_8UC1);
    mImageDataOutput(bin,binary.data,DFLT,NULL,NULL);
    
    vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	mTimerBegin("OpenCV");
	for(int i=0;i<100;i++)findContours(binary,contours,hierarchy,CV_RETR_LIST,CV_CHAIN_APPROX_NONE,Point());
	mTimerEnd("OpenCV");

    MSheet *sheet =mSheetCreate(DFLT,NULL,NULL);
    mTimerBegin("Morn");
    for(int i=0;i<100;i++) mImageBinaryEdge(bin,sheet,NULL);
    mTimerEnd("Morn");

    for(int h=0;h<bin->height;h++) memset(bin->data[0][h],0,bin->width*sizeof(unsigned char));
    for(int j=0;j<sheet->row;j++)for(int i=0;i<sheet->col[j];i++)
    {
        MImagePoint *p = (MImagePoint *)(sheet->data[j][i]);
        bin->data[0][(int)(p->y)][(int)(p->x)]=255;
    }
    mImageSave(bin,"../doc/test_binary_edge.png");

    mImageRelease(src);
    mImageRelease(bin);
    mSheetRelease(sheet);
    return 0;
}

extern "C" void mImageBinaryDenoise(MImage *src,MImage *dst,int num_thresh);
int main1()
{
    MImage *src = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad(src,"D:/Morn_video/data5/test1_bin.png");

    mTimerBegin();
    mImageBinaryDenoise(src,src,32);
    mTimerEnd();
    mImageSave(src,"./test_binary_denoise.png");

    mImageRelease(src);
    return 0;
}

extern "C" int mImageBinaryArea(MImage *src,MList *list,MImagePoint *point);
int main2()
{
    MImage *bin = mImageCreate(DFLT,DFLT,DFLT,NULL);
    mImageLoad(bin,"../doc/test_binary_bin.png");

    MImage *out = mImageCreate(1,bin->height,bin->width,NULL);

    MList *list = mListCreate(DFLT,NULL);
    for(int j=40;;j+=40)
    {
        if(j>255) j=40;
        
        mImageBinaryArea(bin,list,NULL);
        
        printf("list->num = %d\n",list->num);
        if(list->num==0) break;
        for(int i=0;i<list->num;i++)
        {
            MImagePoint *p=(MImagePoint *)(list->data[i]);
            out->data[0][(int)(p->y)][(int)(p->x)]=j;
        }
    }

    // MImagePoint point;mPoint(&point,500,500);
    // mImageBinaryArea(bin,list,&point);
    // printf("list->num = %d\n",list->num);
    // for(int i=0;i<list->num;i++)
    // {
    //     MImagePoint *p=(MImagePoint *)(list->data[i]);
    //     bin->data[0][(int)(p->y)][(int)(p->x)]=180;
    // }
    mImageSave(out,"../doc/test_binary_area.png");
    
    mImageRelease(bin);
    mImageRelease(out);
    mListRelease(list);
    return 0;
}
