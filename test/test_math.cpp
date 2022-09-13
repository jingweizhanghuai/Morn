#include "morn_math.h"
#include <iostream> 
#include <cmath>
#include "apfloat.h"
#include "apcplx.h"

using namespace std; 
// int main(void) 
// { 
//  apfloat x = 2; 
//  cout << pretty << x << endl; 
//  x.prec(10);
//  apfloat b=sin(x);
//  cout << pretty << b << endl; 
//  return 0;
// }

int main()
{
    double a=-7.7036216894;
    apcomplex x=a;
    x.prec(20);
    apcomplex b=sin(x);
    cout << pretty << b.re << endl;
    
    printf("%.10f\n",sin(a));

    float c=a*180.0/MORN_PI;
    printf("%.10f\n",mSin(c));
    
    return 0;
}
    