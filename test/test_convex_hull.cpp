//build: g++ -O2 -fopenmp test_convex_hull.cpp -o test_convex_hull.exe -lmorn -lCGAL -lgmp -lgeos -lopencv_core -lopencv_imgproc
#include "morn_image.h"

#include "CGAL/Exact_predicates_inexact_constructions_kernel.h"
#include "CGAL/convex_hull_2.h"

#include "geos/geom.h"

#include "opencv2/opencv.hpp"

// #define N 10000

void PointInRectangle(float *data,int n)
{
    for(int i=0;i<n*2;i+=2)
    {
        data[i+0]=mRand(-135700,135700)/1357.0;
        data[i+1]=mRand(-135700,135700)/1357.0;
    }
}

void PointInCircle(float *data,int n)
{
    int num=0;
    while(num<n)
    {
        data[0]=mRand(-135700,135700)/1357.0;
        data[1]=mRand(-135700,135700)/1357.0;
        if(data[0]*data[0]+data[1]*data[1]>100*100) continue;
        num+=1;
        data+=2;
    }
}

void PointWithGaussian(float *data,int n)
{
    for(int i=0;i<n*2;i+=2)
    {
        data[i+0]=mNormalRand(0,20);
        data[i+1]=mNormalRand(0,20);
    }
}

typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2 CGALPoint;
int test_CGAL(float *data,int n)
{
    CGALPoint *cgal_point=(CGALPoint *)malloc(n*sizeof(CGALPoint));
    for(int i=0;i<n;i++)
        cgal_point[i]=CGALPoint(data[i+i],data[i+i+1]);

    CGALPoint *cgal_result=(CGALPoint *)malloc(n*sizeof(CGALPoint));
    CGALPoint *ptr;
    mTimerBegin("CGAL");
    for(int i=0;i<10000000/n;i++)
        ptr = CGAL::convex_hull_2(cgal_point,cgal_point+n,cgal_result);
    mTimerEnd("CGAL");
    int rst_num=ptr-cgal_result;
//     printf("CGAL result num = %ld\n",ptr-cgal_result);
//     for(int i=0;i<ptr-cgal_result;i++)
//     {
//         printf("cgal_result=(%f,%f)\n",cgal_result[i][0],cgal_result[i][1]);
//     }
    free(cgal_point);
    free(cgal_result);
    return rst_num;
}

int test_GEOS(float *data,int n)
{
    std::vector<Coordinate> geos_point;
    for(int i=0;i<n;i++)
        geos_point.push_back(Coordinate(data[i+i],data[i+i+1]));
    
    const GeometryFactory* factory = GeometryFactory::getDefaultInstance();
    MultiPoint* mp = factory->createMultiPoint(geos_point);
    Geometry* geos_rst;
    mTimerBegin("GEOS");
    for(int i=0;i<10000000/n;i++)
        geos_rst=mp->convexHull();
    mTimerEnd("GEOS");
//     printf("GEOS result num = %ld\n",geos_rst->getNumPoints()-1);
//     CoordinateSequence* cs=geos_rst->getCoordinates();
//     for(int i=0;i<geos_rst->getNumPoints()-1;i++)
//     {
//         Coordinate co=cs->getAt(i);
//         printf("geos result=(%f,%f)\n",co.x,co.y);
//     }
    return geos_rst->getNumPoints()-1;
}

int test_OpenCV(float *data,int n)
{
    std::vector<cv::Point2f> opencv_point;
    for(int i=0;i<n;i++)
        opencv_point.push_back(cv::Point2f(data[i+i],data[i+i+1]));

    std::vector<cv::Point2f> opencv_result;
    mTimerBegin("OpenCV");
    for(int i=0;i<10000000/n;i++)
        cv::convexHull(opencv_point,opencv_result);
    mTimerEnd("OpenCV");
//     printf("OpenCV result num = %ld\n",opencv_result.size());
//     for(int i=0;i<opencv_result.size();i++)
//     {
//         printf("opencv_result=(%f,%f)\n",opencv_result[i].x,opencv_result[i].y);
//     }
    return opencv_result.size();
}

int test_Morn(float *data,int n)
{
    MList *morn_point=mListCreate();
    mListPlace(morn_point,data,n,sizeof(MImagePoint));
    
    MList *morn_result = mListCreate();
    mTimerBegin("Morn");
    for(int i=0;i<10000000/n;i++)
        mConvexHull(morn_point,morn_result);
    mTimerEnd("Morn");
    int rst_num=morn_result->num;
//     printf("Morn result num = %d\n",morn_result->num);
//     for(int i=0;i<morn_result->num;i++) 
//     {
//         MImagePoint *pt = (MImagePoint *)(morn_result->data[i]);
//         printf("morn_result=(%f,%f)\n",pt->x,pt->y);
//     }
    mListRelease(morn_result);
    mListRelease(morn_point);
    return rst_num;
}

int main()
{
    float *data=(float *)malloc(100000*2*sizeof(float));
    int n1,n2,n3,n4;
    
    printf("\n%d data in rectangle, for %d times:\n",1000,10000000/1000);
    PointInRectangle(data,1000);
    n1=test_CGAL(data,1000);
    n2=test_GEOS(data,1000);
    n3=test_OpenCV(data,1000);
    n4=test_Morn(data,1000);
    mException((n1!=n2)||(n2!=n3)||(n3!=n4),EXIT,"result error with CGAL is %d, GEOS is %d, OpenCV is %d and Morn is %d.",n1,n2,n3,n4);
    
    printf("\n%d data in circle, for %d times:\n",1000,10000000/1000);
    PointInCircle(data,1000);
    n1=test_CGAL(data,1000);
    n2=test_GEOS(data,1000);
    n3=test_OpenCV(data,1000);
    n4=test_Morn(data,1000);
    mException((n1!=n2)||(n2!=n3)||(n3!=n4),EXIT,"result error with CGAL is %d, GEOS is %d, OpenCV is %d and Morn is %d.",n1,n2,n3,n4);

    printf("\n%d data with gaussian, for %d times:\n",1000,10000000/1000);
    PointWithGaussian(data,1000);
    n1=test_CGAL(data,1000);
    n2=test_GEOS(data,1000);
    n3=test_OpenCV(data,1000);
    n4=test_Morn(data,1000);
    mException((n1!=n2)||(n2!=n3)||(n3!=n4),EXIT,"result error with CGAL is %d, GEOS is %d, OpenCV is %d and Morn is %d.",n1,n2,n3,n4);

    printf("\n%d data in rectangle, for %d times:\n",10000,10000000/10000);
    PointInRectangle(data,10000);
    n1=test_CGAL(data,10000);
    n2=test_GEOS(data,10000);
    n3=test_OpenCV(data,10000);
    n4=test_Morn(data,10000);
    mException((n1!=n2)||(n2!=n3)||(n3!=n4),EXIT,"result error with CGAL is %d, GEOS is %d, OpenCV is %d and Morn is %d.",n1,n2,n3,n4);

    printf("\n%d data in circle, for %d times:\n",10000,10000000/10000);
    PointInCircle(data,10000);
    n1=test_CGAL(data,10000);
    n2=test_GEOS(data,10000);
    n3=test_OpenCV(data,10000);
    n4=test_Morn(data,10000);
    mException((n1!=n2)||(n2!=n3)||(n3!=n4),EXIT,"result error with CGAL is %d, GEOS is %d, OpenCV is %d and Morn is %d.",n1,n2,n3,n4);

    printf("\n%d data with gaussian, for %d times:\n",10000,10000000/10000);
    PointWithGaussian(data,10000);
    n1=test_CGAL(data,10000);
    n2=test_GEOS(data,10000);
    n3=test_OpenCV(data,10000);
    n4=test_Morn(data,10000);
    mException((n1!=n2)||(n2!=n3)||(n3!=n4),EXIT,"result error with CGAL is %d, GEOS is %d, OpenCV is %d and Morn is %d.",n1,n2,n3,n4);

    printf("\n%d data in rectangle, for %d times:\n",100000,10000000/100000);
    PointInRectangle(data,100000);
    n1=test_CGAL(data,100000);
    n2=test_GEOS(data,100000);
    n3=test_OpenCV(data,100000);
    n4=test_Morn(data,100000);
    mException((n1!=n2)||(n2!=n3)||(n3!=n4),EXIT,"result error with CGAL is %d, GEOS is %d, OpenCV is %d and Morn is %d.",n1,n2,n3,n4);

    printf("\n%d data in circle, for %d times:\n",100000,10000000/100000);
    PointInCircle(data,100000);
    n1=test_CGAL(data,100000);
    n2=test_GEOS(data,100000);
    n3=test_OpenCV(data,100000);
    n4=test_Morn(data,100000);
    mException((n1!=n2)||(n2!=n3)||(n3!=n4),EXIT,"result error with CGAL is %d, GEOS is %d, OpenCV is %d and Morn is %d.",n1,n2,n3,n4);

    printf("\n%d data with gaussian, for %d times:\n",100000,10000000/100000);
    PointWithGaussian(data,100000);
    n1=test_CGAL(data,100000);
    n2=test_GEOS(data,100000);
    n3=test_OpenCV(data,100000);
    n4=test_Morn(data,100000);
    mException((n1!=n2)||(n2!=n3)||(n3!=n4),EXIT,"result error with CGAL is %d, GEOS is %d, OpenCV is %d and Morn is %d.",n1,n2,n3,n4);

    free(data);
}
