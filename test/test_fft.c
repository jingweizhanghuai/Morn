/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
// 编译： gcc -fopenmp test_fft.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_fft.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "morn_wave.h"

int main()
{
    MWave *src = mWaveCreate(1,30,NULL);
    MWave *fft = mWaveCreate(DFLT,DFLT,NULL);
    MWave *dst = mWaveCreate(DFLT,DFLT,NULL);
    
    printf("src->size is %d.\n",src->size);
    for(int i=0;i<src->size;i++) {src->data[0][i]=((float)mRand(-100,100))/100.0f; printf("%f,",src->data[0][i]);}
    
    mWaveFFT(src,fft);
    
    printf("\n\nfft->size is %d.\n",fft->size);
    for(int i=0;i<fft->size;i++) printf("%f+%fi,",fft->data[0][i],fft->data[1][i]);
    
    mWaveIFFT(fft,dst);
    
    printf("\n\ndst->size is %d.\n",dst->size);
    for(int i=0;i<dst->size;i++) printf("%f,",dst->data[0][i]);
    
    mWaveRelease(src);
    mWaveRelease(fft);
    mWaveRelease(dst);
}
