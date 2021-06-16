//build_x64_mingw: gcc -O2 -fopenmp test_geometry3.c -o test_geometry3.exe -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -ljpeg -lpng -lz
#include "morn_image.h"

void mPoissonDiskPoint(MList *list,float r,float x1,float x2,float y1,float y2);
int main()
{
    MImage *img = mImageCreate(3,400,400);
    MList *point = mListCreate();
    mPoissonDiskPoint(point,16,0,400,0,400);
    printf("point->num=%d\n",point->num);
    int n=0;
    for(int i=0;i<point->num;i++)
    {
        MImagePoint *pt = point->data[i];
        if((pt->x-200)*(pt->x-200)+(pt->y-200)*(pt->y-200)<200*200)
        {
            point->data[n]=pt;n++;
            mImageDrawPoint(img,pt);
        }
    }
    point->num = n;
    mImageSave(img,"./test_geimetry1.png");

    MList *polygon = mListCreate();
    mConvexHull(point,polygon);
    printf("polygon->num=%d\n",polygon->num);

    unsigned char color[3]={0,0,255};
    mImageDrawShape(img,polygon,color);
    mImageSave(img,"./test_geimetry2.png");
    
    mImageRelease(img);
    mListRelease(point);
    mListRelease(polygon);
}