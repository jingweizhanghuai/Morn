
// gcc test_argument.c -o test_argument.exe -lmorn
#include "morn_util.h"

int main(int argc,char *argv[])
{
    char *help_info = "Options:\
    \n--help          Display this information\
    \n--name          Set national name\
    \n--area          Set national area\
    \n--population    Set national population\
    \n--date          month-day,Set national day\
    \n";
    if(mStringArgument(argc,argv,"-help")) {printf(help_info);return 0;}
    
    char *name = mStringArgument(argc,argv,"-name");
    int month,day;
    mStringArgument(argc,argv,"-date","%d-%d",&month,&day);
    float area;
    mStringArgument(argc,argv,"-area","%f",&area);
    float population=atof(mStringArgument(argc,argv,"-population","%f",&population));

    printf("Here is %s, with area %f million people, and %f thousand square kilometers of land, %d-%d is our national-day. You are welcome.",name,population,area,month,day);

    return 0;
}