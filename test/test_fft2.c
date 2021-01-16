/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
// build: gcc -O2 -fopenmp test_fft2.c -o test_fft2.exe -I ..\include\ -I C:\ProgramFiles\CPackage\fftw\include -L ..\lib\x64_mingw\ -L C:\ProgramFiles\CPackage\fftw\lib -lfftw3f -lmorn

#include "morn_wave.h"
#include "fftw3.h"

int main()
{
    MFile *file = mFileCreate("./test.wav");
    MWave *src = mWaveCreate(1,1024,NULL);
    MWave *dst = mWaveCreate(2,1024,NULL);
    mWAVRead(file,src);

    printf("fft with Morn:\n");
    mTimerBegin();
    for(int i=0;i<100000;i++)mWaveFFT(src,dst);
    mTimerEnd();
    
    // for(int i=0;i<80;i+=1) printf("%f+%fi,",dst->data[0][i],dst->data[1][i]);printf("\n\n");

    /*
    fftwf_complex *in  = fftwf_malloc(sizeof(fftwf_complex) * 1024);
    fftwf_complex *out = fftwf_malloc(sizeof(fftwf_complex) * 1024);
    for(int i=0;i<1024;i++){in[i][0]=src->data[0][i];in[i][1]=0;}

    printf("fft with fftw:\n");
    mTimerBegin();
    fftwf_plan plan = fftwf_plan_dft_1d(1024,in,out,FFTW_FORWARD,FFTW_ESTIMATE);
    for(int i=0;i<10000;i++) fftwf_execute(plan);
    fftwf_destroy_plan(plan);
    fftwf_cleanup();
    mTimerEnd();
    
    // for(int i=0;i<80;i+=1) printf("%f+%fi,",out[i][0],out[i][1]);printf("\n\n");
    fftwf_free(in);
    fftwf_free(out);
    */

    float *data = mMalloc(sizeof(float)*1024);
    for(int i=0;i<1024;i++){data[i]=src->data[0][i];}
    fftwf_complex *out = fftwf_malloc(sizeof(fftwf_complex) * 1024);
    
    printf("fft with fftw:\n");
    mTimerBegin();
    // fftwf_plan plan = fftwf_plan_dft_1d(1024,in,out,FFTW_FORWARD,FFTW_ESTIMATE);
    fftwf_plan plan = fftwf_plan_dft_r2c_1d(1024,data,out,FFTW_ESTIMATE);
    // fftwf_plan plan = fftwf_plan_dft_r2c_1d(1024,data,out,FFTW_MEASURE);
    for(int i=0;i<100000;i++)
    {
        fftwf_execute(plan);
        for(int j=513;j<1024;j++) {out[j][0]=out[1024-j][0];out[j][1]=out[1024-j][1];}
    }
    fftwf_destroy_plan(plan);
    fftwf_cleanup();
    mTimerEnd();
    
    mFree(data);
    fftwf_free(out);
    
    mFileRelease(file);
    mWaveRelease(src);
    mWaveRelease(dst);
}
