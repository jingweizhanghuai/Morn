
#include "morn_math.h"

double r2a(double r)
{
    return r*180.0/MORN_PI;
}

double a2r(double a)
{
    return a*MORN_PI/180.0;
}

double get_mid_value(double a,double b,double c)
{
    if((a>b)==(c>=a)) return a;
    if((a>b)==(b>=c)) return b;
    return c;
}

int main()
{
    printf("1+2*3/4            = %f\n",mCaculate("1+2*3/4"));
    printf("pi                 = %f\n",mCaculate("pi"));
    printf("sin(pi/2)          = %f\n",mCaculate("sin(pi/2)"));
    printf("ln(e)              = %f\n",mCaculate("ln(e)"));
    printf("sqrt(3^2+pow(4,2)) = %f\n",mCaculate("sqrt(3^2+pow(4,2))"));
    
    mCaculateFunction(a2r);
    printf("sin(a2r(30))       = %f\n",mCaculate("sin(a2r(30))"));
    
    mCaculateFunction(r2a);
    printf("r2a(atan(1))       =%f\n",mCaculate("r2a(atan(1))"));

    mCaculateFunction("mid",get_mid_value);
    printf("mid(5,1,2)         = %f\n",mCaculate("mid(5,1,2)"));

    return 0;
}
