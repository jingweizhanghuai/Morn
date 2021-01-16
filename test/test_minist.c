/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
//build: gcc -O2 -fopenmp test_minist.c -o test_minist.exe -I ..\include\ -L ..\lib\x64_mingw\ -lmorn -lopenblas -lclblas -lOpenCL -lstdc++
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "morn_Image.h"
#include "morn_Tensor.h"

void test_data()
{
    FILE *f_label= fopen("D:/download/minist/train-labels.idx1-ubyte","rb");
    FILE *f_data = fopen("D:/download/minist/train-images.idx3-ubyte","rb");
    
    MTensor *tns[2];
    tns[0] = mTensorCreate(1,1,28,28,NULL);
    tns[1] = mTensorCreate(1,1, 1,10,NULL);

    unsigned char label;
    unsigned char data[28*28];
    fseek(f_label,8,SEEK_SET);
    fseek(f_data,16,SEEK_SET);
    char mornname[256];
    for(int n=0;n<60000;n++)
    {
        fread(&label,1,1,f_label);
        memset(tns[1]->data[0],0,10*sizeof(float));tns[1]->data[0][label]=1;
        
        fread(data,1,28*28,f_data);
        for(int i=0;i<28*28;i++) tns[0]->data[0][i]=(float)data[i]/256;
        
        sprintf(mornname,"D:/download/minist/train%05d.morn",n);
        MFile *morn = mFileCreate(mornname);
        mMORNWrite(morn,mHash( "input",DFLT),(void **)(tns[0]->data),1,28*28*sizeof(float));
        mMORNWrite(morn,mHash("output",DFLT),(void **)(tns[1]->data),1,   10*sizeof(float));
        mFileRelease(morn);
    }
    mTensorRelease(tns[0]);
    mTensorRelease(tns[1]);
    fclose(f_label);
    fclose(f_data);
    return;
}

FILE *g_f_label=NULL;
FILE *g_f_data =NULL;
void MinistData(MVector **vec,char **name,int number,char *dir)
{
    mException((number!=2),EXIT,"invalid input");
    
    if(g_f_label==NULL)g_f_label= fopen("D:/download/minist/train-labels.idx1-ubyte","rb");
    if(g_f_data ==NULL)g_f_data = fopen("D:/download/minist/train-images.idx3-ubyte","rb");
    
    MVector *in,*out;
    if(strcmp(name[0],"input")==0){in=vec[0];out=vec[1];}
    else                          {in=vec[1];out=vec[0];}

    int n=mRand(0,6000);
    fseek(g_f_label,8+n*sizeof(unsigned char),SEEK_SET);
    fseek(g_f_data,16+n*28*28*sizeof(unsigned char),SEEK_SET);
    unsigned char label;
    unsigned char data[28*28];
    fread(&label,1,1,g_f_label);
    fread(data,1,28*28,g_f_data);
    for(int i=0;i<28*28;i++) in->data[i]=(float)data[i]/256;
    // printf("in->data[28*28]=%f\n",in->data[28*28]);
    memset(out->data,0,10*sizeof(float));out->data[label]=1;
}

void test_train()
{
    mDataFuncRegister(MinistData);
    mDeeplearningTrain("./test_minist.ini");
    if(g_f_label!=NULL) fclose(g_f_label);
    if(g_f_data !=NULL) fclose(g_f_data );
    return;
}

int test_predict()
{
    MFile *ini = mFileCreate("./test_minist.ini");
    
    FILE *f_label= fopen("D:/download/minist/t10k-labels.idx1-ubyte","rb");
    FILE *f_data = fopen("D:/download/minist/t10k-images.idx3-ubyte","rb");
    
    MTensor *tns[2];
    tns[0] = mTensorCreate(1,1,28,28,NULL);
    tns[1] = mTensorCreate(1,1, 1,10,NULL);
    
    char *name[2];name[0]="input";name[1]="output";
    
    unsigned char label;
    unsigned char predict;
    unsigned char data[28*28];
    fseek(f_label,8,SEEK_SET);
    fseek(f_data,16,SEEK_SET);
    int correct=0;
    for(int n=0;n<10000;n++)
    {
        // printf("n=%d\n",n);
        fread(data,1,28*28,f_data);
        for(int i=0;i<28*28;i++) tns[0]->data[0][i]=(float)data[i]/256;
        
        mNetworkPredict(ini,name,tns);
        float max=0;for(int i=0;i<10;i++) if(tns[1]->data[0][i]>max) {max=tns[1]->data[0][i];predict=i;}
        
        fread(&label,1,1,f_label);
        if(label==predict) correct+=1;
    }
    printf("total 10000,correct %d\n",correct);
    mTensorRelease(tns[0]);
    mTensorRelease(tns[1]);
    
    fclose(f_label);
    fclose(f_data);
    
    mFileRelease(ini);
    return 0;
}

int main(int argc,char **argv)
{
    mException(argc!=2,EXIT,"invalid operate");
         if(strcmp(argv[1],"data"   )==0) {test_data()   ;return 0;}
    else if(strcmp(argv[1],"train"  )==0) {test_train()  ;return 0;}
    else if(strcmp(argv[1],"predict")==0) {test_predict();return 0;}
}
