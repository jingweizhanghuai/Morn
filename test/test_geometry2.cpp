//build:   g++ -O2 -fopenmp test_geometry2.cpp -o test_geometry2.exe -lmorn -lCGAL -lgmp
#include "morn_image.h"
#include "CGAL/Exact_predicates_inexact_constructions_kernel.h"
// #include <CGAL/Polygon_2_algorithms.h>
// #include <CGAL/draw_polygon_2.h>
#include "CGAL/convex_hull_2.h"

#include "geos/geom.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel::Point_2 CGALPoint;

// typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
// typedef K::Point_2 CGALPoint;
/*
using std::cout; 
using std::endl;

void check_inside(CGALPoint pt, CGALPoint *pgn_begin, CGALPoint *pgn_end, K traits)
{
    cout << "The point " << pt;
    switch(CGAL::bounded_side_2(pgn_begin, pgn_end,pt, traits)) {
        case CGAL::ON_BOUNDED_SIDE :
            cout << " is inside the polygon.\n";
            break;
        case CGAL::ON_BOUNDARY:
            cout << " is on the polygon boundary.\n";
            break;
        case CGAL::ON_UNBOUNDED_SIDE:
            cout << " is outside the polygon.\n";
            break;
    }
}

int main1()
{
    CGALPoint points[] = { CGALPoint(0,0), CGALPoint(5,0), CGALPoint(5,5), CGALPoint(0,5)};
// CGAL::draw(points);

    cout << "The polygon is "
        << (CGAL::is_simple_2(points, points+4, K()) ? "" : "not ")
        << "simple." << endl;

    check_inside(CGALPoint(0.5, 0.5), points, points+4, K());
    check_inside(CGALPoint(1.5, 25), points, points+4, K());
    check_inside(CGALPoint(2.5, 0), points, points+4, K());

    double a=polygon_area_2(points,points+4,K());
    printf("a=%f\n",a);

    return 0;
}


int main2()
{
    CGALPoint points[5] = {CGALPoint(0,0), CGALPoint(10,0), CGALPoint(0,10), CGALPoint(6,5), CGALPoint(4,1) };
    CGALPoint result[5];
    CGALPoint *ptr = CGAL::convex_hull_2( points, points+5, result );
    std::cout <<    ptr - result << " points on the convex hull:" << std::endl;
    for(int i = 0; i < ptr - result; i++){std::cout << result[i] << std::endl;}

    MImagePoint p[5];mPoint(p+0,0,0);mPoint(p+1,10,0);mPoint(p+2,0,10);mPoint(p+3,6,5);mPoint(p+4,4,1);
    MList *list = mListCreate(); mListPlace(list,p,5,sizeof(MImagePoint));
    MList *polygon = mListCreate();
    mConvexHull(list,polygon);
    printf("polygon->num=%d\n",polygon->num);
    for(int i=0;i<polygon->num;i++) 
    {
        MImagePoint *pt = (MImagePoint *)(polygon->data[i]);
        printf("pt=(%f,%f)\n",pt->x,pt->y);
    }
    mListRelease(list);
    mListRelease(polygon);
    
    return 0;
}

void test1()
{
    #define N 10000000
    
    MList *morn_polygon= mListCreate(4);
    CGALPoint cgal_polygon[4];
    
    MImagePoint *pt = (MImagePoint *)malloc((N+4)*sizeof(MImagePoint));
    for(int i=0;i<(N+4);i+=4)
    {
        pt[i+0].x=mRand(   0, 5050)/101.0;pt[i+0].y=mRand(   0, 5050)/101.0;
        pt[i+1].x=mRand(   0, 5050)/101.0;pt[i+1].y=mRand(5050,10100)/101.0;
        pt[i+2].x=mRand(5050,10100)/101.0;pt[i+2].y=mRand(5050,10100)/101.0;
        pt[i+3].x=mRand(5050,10100)/101.0;pt[i+3].y=mRand(   0, 5050)/101.0;
    }

    int *flag = (int*)malloc(N*sizeof(int));
    mTimerBegin("CGAL");
    for(int i=0;i<N;i++)
    {
        cgal_polygon[0]=CGALPoint(pt[i  ].x,pt[i  ].y);
        cgal_polygon[1]=CGALPoint(pt[i+1].x,pt[i+1].y);
        cgal_polygon[2]=CGALPoint(pt[i+2].x,pt[i+2].y);
        cgal_polygon[3]=CGALPoint(pt[i+3].x,pt[i+3].y);
        CGALPoint test =CGALPoint(pt[N-i].x,pt[N-i].y);
        flag[i] = (CGAL::bounded_side_2(cgal_polygon,cgal_polygon+4,test,K())==CGAL::ON_BOUNDED_SIDE);
        // printf("%d",flag[i]);
    }
    mTimerEnd("CGAL");
    
    mTimerBegin("Morn");
    for(int i=0;i<N;i++)
    {
        morn_polygon->data[0]=pt+i;
        morn_polygon->data[1]=pt+i+1;
        morn_polygon->data[2]=pt+i+2;
        morn_polygon->data[3]=pt+i+3;
        MImagePoint *test = pt+N-i;
        flag[i] = mPointInPolygon(test,morn_polygon);
        // printf("%d",flag[i]);
    }
    mTimerEnd("Morn");
    free(flag);

    float *area = (float *)malloc(N*sizeof(float));
    mTimerBegin("CGAL");
    for(int i=0;i<N;i++)
    {
        cgal_polygon[0]=CGALPoint(pt[i  ].x,pt[i  ].y);
        cgal_polygon[1]=CGALPoint(pt[i+1].x,pt[i+1].y);
        cgal_polygon[2]=CGALPoint(pt[i+2].x,pt[i+2].y);
        cgal_polygon[3]=CGALPoint(pt[i+3].x,pt[i+3].y);
        area[i] = ABS(polygon_area_2(cgal_polygon,cgal_polygon+4,K()));
        // printf("%f ",ABS(area));
    }
    mTimerEnd("CGAL");
    
    mTimerBegin("Morn");
    for(int i=0;i<N;i++)
    {
        morn_polygon->data[0]=pt+i;
        morn_polygon->data[1]=pt+i+1;
        morn_polygon->data[2]=pt+i+2;
        morn_polygon->data[3]=pt+i+3;
        area[i] = mPolygonArea(morn_polygon);
        // printf("%f ",area);
    }
    mTimerEnd("Morn");
    free(area);
    
    mListRelease(morn_polygon);
    free(pt);
}
*/

// int main()
// {
//     MImage *img = mImageCreate(1,200,200);
//     MImagePoint pt;
//     int n=0;
//     while(n<1000)
//     {
//         pt.x=mNormalRand(100,50);if((pt.x<0)||(pt.x>200)) continue;
//         pt.y=mNormalRand(100,50);if((pt.y<0)||(pt.y>200)) continue;
//         n++;
//         mImageDrawPoint(img,&pt);
//     }
//     mImageSave(img,"./test_geometry.png");
//     
//     mImageRelease(img);
// }




int main()
{
    #define N 10000
    
    CGALPoint cgal_point[N];
    MList *list = mListCreate();
    std::vector<Coordinate> points;
    
    MImagePoint pt;
    int num=0;
    while(num<N)
    {
        pt.x=mRand(-135700,135700)/1357.0;
        pt.y=mRand(-135700,135700)/1357.0;
        if(pt.x*pt.x+pt.y*pt.y>100*100) continue;
        mListWrite(list,DFLT,&pt,sizeof(MImagePoint));
        cgal_point[num]=CGALPoint(pt.x,pt.y);
        points.push_back(Coordinate(pt.x,pt.y));
        num++;
    }

    CGALPoint cgal_result[N];
    CGALPoint *ptr;
    mTimerBegin("CGAL");
    for(int i=0;i<1000;i++)
        ptr = CGAL::convex_hull_2(cgal_point,cgal_point+N,cgal_result);
    mTimerEnd("CGAL");
    printf("cgal result num = %ld\n",ptr-cgal_result);
//     for(int i=0;i<ptr-cgal_result;i++)
//     {
//         printf("cgal_result=(%f,%f)\n",cgal_result[i][0],cgal_result[i][1]);
//     }
    
    const GeometryFactory* factory = GeometryFactory::getDefaultInstance();
    MultiPoint* mp = factory->createMultiPoint(points);
    Geometry* geos_rst;
    mTimerBegin("geos");
    for(int i=0;i<1000;i++)
        geos_rst=mp->convexHull();
    mTimerEnd("geos");
    printf("geos result num = %ld\n",geos_rst->getNumPoints()-1);
//     CoordinateSequence* cs=geos_rst->getCoordinates();
//     for(int i=0;i<geos_rst->getNumPoints()-1;i++)
//     {
//         Coordinate co=cs->getAt(i);
//         printf("geos result=(%f,%f)\n",co.x,co.y);
//     }

    MList *morn_result = mListCreate();
    mTimerBegin("Morn");
    for(int i=0;i<1000;i++)
        mConvexHull(list,morn_result);
    mTimerEnd("Morn");
    printf("morn result num = %d\n",morn_result->num);
//     for(int i=0;i<morn_result->num;i++) 
//     {
//         MImagePoint *pt = (MImagePoint *)(morn_result->data[i]);
//         printf("morn_result=(%f,%f)\n",pt->x,pt->y);
//     }

    mListRelease(morn_result);
    mListRelease(list);
}
