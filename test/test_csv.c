// gcc -O2 -fopenmp test_csv.c -o test_csv.exe -lmorn
#include "morn_util.h"

int main()
{
//     MFile *file = mFileCreate("D:/data/Women_Shoes_Prices/data/data/7210_1.csv");
//     MFile *file = mFileCreate("D:/data/Denver_Police/data/police_pedestrian_stops_and_vehicle_stops.csv");
    MFile *file = mFileCreate("../data/test.csv");
    mTimerBegin();
    mCSVLoad(file);
    mTimerEnd();
    int row=0;mPropertyRead(file,"csv_row",&row,NULL);
    int col=0;mPropertyRead(file,"csv_col",&col,NULL);
    printf("row=%d,col=%d\n",row,col);

    char *p;
    p=mCSVRead(file,5,2);
    printf("p=%s\n",p);

    p=mCSVRead(file,5,"PROBLEM");
    printf("p=%s\n",p);

    p=mCSVRead(file,"DPD-15-0647708",2);
    printf("p=%s\n",p);

    p=mCSVRead(file,"DPD-15-0647708","PROBLEM");
    printf("p=%s\n",p);

    mPropertyWrite(file,"key","TIME_PHONEPICKUP");
    p=mCSVRead(file,"2015-11-05 00:37:02","PROBLEM");
    printf("p=%s\n",p);
    

    MList *list = mCSVCol(file,"MASTER_INCIDENT_NUMBER");
    
    printf("list->num=%d\n",list->num);
    for(int i=0;i<list->num;i++) printf("%s\n",(char *)(list->data[i]));
    
    mPropertyWrite(file,"key","MASTER_INCIDENT_NUMBER");
    for(int i=0;i<100;i++)
    {
        char *p=list->data[mRand(1,list->num)];
        printf("(%s) at %s: %s\n",p,mCSVRead(file,p,"TIME_PHONEPICKUP"),mCSVRead(file,p,"CALL_DISPOSITION"));
    }

    mFileRelease(file);
    return 0;
}


