
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
    printf("1+2*3/4            = %f\n",mCalculate("1+2*3/4"));
    printf("pi                 = %f\n",mCalculate("pi"));
    printf("sin(pi/2)          = %f\n",mCalculate("sin(pi/2)"));
    printf("ln(e)              = %f\n",mCalculate("ln(e)"));
    printf("sqrt(3^2+pow(4,2)) = %f\n",mCalculate("sqrt(3^2+pow(4,2))"));
    
    mCalculateFunction(a2r);
    printf("sin(a2r(30))       = %f\n",mCalculate("sin(a2r(30))"));
    
    mCalculateFunction(r2a);
    printf("r2a(atan(1))       =%f\n",mCalculate("r2a(atan(1))"));

    mCalculateFunction("mid",get_mid_value);
    printf("mid(5,1,2)         = %f\n",mCalculate("mid(5,1,2)"));

    return 0;
}
