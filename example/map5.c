#include "morn_util.h"

/*
 * this example shows the different between enable-overwrite and disable-overwrite when use Morn map.
 */
int main()
{
    MMap *map = mMapCreate();
    
    int n;
    n=0; mMapWrite(map,"test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=1; mMapWrite(map,"test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=2; mMapWrite(map,"test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=3; mMapWrite(map,"test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=4; mMapWrite(map,"test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=5; mMapWrite(map,"test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=6; mMapWrite(map,"test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=7; mMapWrite(map,"test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=8; mMapWrite(map,"test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=9; mMapWrite(map,"test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    
    printf("read %d from the map\n",*(int *)mMapRead(map,"test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"test")); 
    printf("read %d from the map\n",*(int *)mMapRead(map,"test")); 
    printf("read %d from the map\n",*(int *)mMapRead(map,"test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"test"));
    
    int flag=0; //0 means don't overwrite
    mPropertyWrite(map,"overwrite",&flag,sizeof(int));
    printf("\ndisable overwrite\n");
    
    n=0; mMapWrite(map,"overwrite_test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=1; mMapWrite(map,"overwrite_test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=2; mMapWrite(map,"overwrite_test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=3; mMapWrite(map,"overwrite_test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=4; mMapWrite(map,"overwrite_test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=5; mMapWrite(map,"overwrite_test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=6; mMapWrite(map,"overwrite_test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=7; mMapWrite(map,"overwrite_test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=8; mMapWrite(map,"overwrite_test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    n=9; mMapWrite(map,"overwrite_test",DFLT,&n,sizeof(int));printf("write %d to the map\n",n);
    
    printf("read %d from the map\n",*(int *)mMapRead(map,"overwrite_test"));    
    printf("read %d from the map\n",*(int *)mMapRead(map,"overwrite_test")); 
    printf("read %d from the map\n",*(int *)mMapRead(map,"overwrite_test")); 
    printf("read %d from the map\n",*(int *)mMapRead(map,"overwrite_test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"overwrite_test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"overwrite_test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"overwrite_test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"overwrite_test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"overwrite_test"));
    printf("read %d from the map\n",*(int *)mMapRead(map,"overwrite_test"));
    
    mMapRelease(map);
    return 0;
}
