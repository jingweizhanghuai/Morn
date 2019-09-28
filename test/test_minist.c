// gcc -O2 -fopenmp test44.c -I ..\include\ -L ..\lib\mingw\ -lmorn -lopenblas -o test44.exe
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "morn_Image.h"
#include "morn_Tensor.h"

int main()
{
    MFile *ini = mFileCreate("./test44.ini");
    NetworkTrain(ini);
    mFileRelease(ini);
    return 0;
}

int main1()
{
    MList *filelist = mListCreate(DFLT,NULL);
    mFileList(filelist,"E:\\minist\\test","*.morn");
    
    char *name[2];name[0]="input_0";name[1]="output_0";
    MTensor *tns[2];
    tns[0] = mTensorCreate(1,1,28,28,NULL);
    tns[1] = mTensorCreate(1,1, 1,10,NULL);
    
    float *out = tns[1]->data[0];
    float true[10];float *ref = true;
    
    MFile *ini = mFileCreate("./test44.ini");
    
    int num=0;
    for(int i=0;i<filelist->num;i++)
    {
        char filename[128];sprintf(filename,"E:\\minist\\test\\%s",filelist->data[i]);
        MFile *file = mFileCreate(filename);
        
        mMORNRead(file,name[0],(void **)(tns[0]->data),1,28*28*sizeof(float));
        mNetworkPredict(ini,name,tns);
        
        float max=out[0];int result=0;
        for(int j=1;j<10;j++) {if(out[j]>max) {max=out[j];result=j;}}
        
        mMORNRead(file,name[1],(void **)(&ref),1,10*sizeof(float));
        int reference;
        for(int j=0;j<10;j++) if(ref[j]==1.0f) reference=j;
        
        printf("filename is %s,result is %d,reference is %d\n",filename,result,reference);
        num+=(result==reference);
        
        mFileRelease(file);
    }
    printf("num is %d\n",num);
    
    mFileRelease(ini);
    return 0;
}

    
    
