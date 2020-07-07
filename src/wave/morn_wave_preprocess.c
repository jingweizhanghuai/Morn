/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_wave.h"

void mWaveHanningWin(MWave *win)
{
    int i;
    mException((INVALID_WAVE(win)),EXIT,"invalid input");
    int size = win->size;
    
    for(i=0;i<size/2;i++)
    {
        float data = 0.5f-0.5f*mCos(((float)(360*i))/((float)size));
        win->data[0][i]= data;
        win->data[0][size-1-i] = data;
    }
    win->data[0][i] = 0.5f-0.5f*mCos(((float)(360*i))/((float)size));
}

void mWaveHammingWin(MWave *win)
{
    int i;
    mException((INVALID_WAVE(win)),EXIT,"invalid input");
    int size = win->size;
    
    for(i=0;i<size/2;i++)
    {
        float data = 0.54f-0.46f*mCos(((float)(360*i))/((float)size));
        win->data[0][i]= data;
        win->data[0][size-1-i] = data;
    }
    win->data[0][i] = 0.54f-0.46f*mCos(((float)(360*i))/((float)size));
}

void mWaveFejerWin(MWave *win)
{
    int i;
    mException((INVALID_WAVE(win)),EXIT,"invalid input");
    int size = win->size;
    
    float data = 0.0f;
    float step = 2.0f/((float)size);
    for(i=0;i<size/2;i++)
    {
        win->data[0][i]= data;
        win->data[0][size-1-i] = data;
        data = data + step;
    }
    if(size%2) win->data[0][i] = data;
}

void mWaveBlackmanWin(MWave *win)
{
    int i;
    float data;
    int size;

    mException((INVALID_WAVE(win)),EXIT,"invalid input");
    size = win->size;
    
    for(i=0;i<size/2;i++)
    {
        data = 0.42f-0.5f*mCos(((float)(360*i))/((float)size))
               + 0.08f*mCos(720.0f*((float)i)/((float)size));
        win->data[0][i]= data;
        win->data[0][size-1-i] = data;
    }
    win->data[0][i] = 0.42f-0.5f*mCos(((float)(360*i))/((float)size))
                    + 0.08f*mCos(720.0f*((float)i)/((float)size));
}

#define HASH_WaveWithWindow 0x61c83602
struct HandleWaveWithWindow {
    void (*win_func)(MWave *);
    MWave *win;
}HandleWaveWithWindow;

void endWaveWithWindow(void *info)
{
    struct HandleWaveWithWindow *handle =(struct HandleWaveWithWindow *)info;
    if(handle->win != NULL) mWaveRelease(handle->win);
}
void mWaveWindow(MWave *src,MWave *dst,void (*win_func)(MWave *))
{
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    
    MHandle *hdl=mHandle(src,WaveWithWindow);
    struct HandleWaveWithWindow *handle = (struct HandleWaveWithWindow *)(hdl->handle);
    if((hdl->valid == 0)||(win_func != handle->win_func))
    {
        if(handle->win == NULL) handle->win = mWaveCreate(1,src->size,NULL);
        else                    mWaveRedefine(handle->win,1,src->size,NULL);
        
        if(win_func ==NULL) win_func = mWaveHammingWin;
        win_func(handle->win);
        
        hdl->valid = 1;
    }

    mWaveMul(src,handle->win,dst);
}
