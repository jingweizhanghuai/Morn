/*
Copyright (C) 2019  Jing Lee
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
