/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// 编译： gcc -O2 -fopenmp test_fft2.c -I ..\include\ -I E:\fftw\api\ -L ..\lib\x64\mingw\ -L E:\fftw\ -lfftw3f -lmorn -o test_fft2.exe
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_wave.h"
#include "fftw3.h"

int main()
{
    MFile *file = mFileCreate("E:/test.wav");
    MWave *src = mWaveCreate(1,1024,NULL);
    MWave *dst = mWaveCreate(2,1024,NULL);
    mWAVRead(file,src);

    printf("fft with Morn:\n");
    mTimerBegin();
    for(int i=0;i<10000;i++)mWaveFFT(src,dst);
    mTimerEnd();
    
    // for(int i=0;i<80;i+=1) printf("%f+%fi,",dst->data[0][i],dst->data[1][i]);printf("\n\n");
    
    fftwf_complex *in  = fftwf_malloc(sizeof(fftwf_complex) * 1024);
    fftwf_complex *out = fftwf_malloc(sizeof(fftwf_complex) * 1024);
    for(int i=0;i<1024;i++){in[i][0]=src->data[0][i];in[i][1]=0;}

    printf("fft with Morn:\n");
    mTimerBegin();
    fftwf_plan plan = fftwf_plan_dft_1d(1024,in,out,FFTW_FORWARD,FFTW_ESTIMATE);
    for(int i=0;i<10000;i++) fftwf_execute(plan);
    fftwf_destroy_plan(plan);
    fftwf_cleanup();
    mTimerEnd();
    
    // for(int i=0;i<80;i+=1) printf("%f+%fi,",out[i][0],out[i][1]);printf("\n\n");
    
    fftwf_free(in);
    fftwf_free(out);
    
    mFileRelease(file);
    mWaveRelease(src);
    mWaveRelease(dst);
}
