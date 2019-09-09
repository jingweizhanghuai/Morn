#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_Wave.h"

/////////////////////////////////////////////////////////
// 接口功能:
//  生成Hanning窗
//
// 参数：
//  (O)win(NO) - 输出的窗波形
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mWaveHanningWin(MWave *win)
{
    int i;
    float data;
    int size;

    mException((INVALID_WAVE(win)),EXIT,"invalid input");
    size = win->size;
    
    for(i=0;i<size/2;i++)
    {
        data = 0.5f-0.5f*mCos(((float)(360*i))/((float)size));
        win->data[0][i]= data;
        win->data[0][size-1-i] = data;
    }
    win->data[0][i] = 0.5f-0.5f*mCos(((float)(360*i))/((float)size));
}

/////////////////////////////////////////////////////////
// 函数功能:
//  生成Hamming窗
//
// 参数：
//  (O)win(NO) - 输出的窗波形
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mWaveHammingWin(MWave *win)
{
    int i;
    float data;
    int size;

    mException((INVALID_WAVE(win)),EXIT,"invalid input");
    size = win->size;
    
    for(i=0;i<size/2;i++)
    {
        // printf("mCos((float)(360*i))/((float)size)) is %f\n",mCos((float)(360*i))/((float)size));
        data = 0.54f-0.46f*mCos(((float)(360*i))/((float)size));
        win->data[0][i]= data;
        win->data[0][size-1-i] = data;
    }
    win->data[0][i] = 0.54f-0.46f*mCos(((float)(360*i))/((float)size));
    // printf("%d:%f,%f,%f,%f\n",win->size,win->data[0][0],win->data[0][1],win->data[0][8],win->data[0][63]);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  生成三角窗
//
// 参数：
//  (O)win(NO) - 输出的窗波形
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mWaveFejerWin(MWave *win)
{
    int i;
    float data;
    float step;
    int size;

    mException((INVALID_WAVE(win)),EXIT,"invalid input");
    size = win->size;
    
    data = 0.0f;
    step = 2.0f/((float)size);
    for(i=0;i<size/2;i++)
    {
        win->data[0][i]= data;
        win->data[0][size-1-i] = data;
        data = data + step;
    }
    
    if(size%2)
        win->data[0][i] = data;
}

/////////////////////////////////////////////////////////
// 接口功能:
//  生成布莱克曼窗
//
// 参数：
//  (O)win(NO) - 输出的窗波形
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
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

void mWaveWin(MWave *win,int winSelect)
{
    if(winSelect == MORN_WIN_HANNING)
        mWaveHanningWin(win);
    else if(winSelect == MORN_WIN_HAMMING)
        mWaveHammingWin(win);
    else if(winSelect == MORN_WIN_FEJER)
        mWaveFejerWin(win);
    else if(winSelect == MORN_WIN_BLACKMAN)
        mWaveBlackmanWin(win);
    else
        mException(1,EXIT,"invalid windows select");
}


/////////////////////////////////////////////////////////
// 接口功能:
//  对波形加窗
//
// 参数：
//  (I)src(NO) - 输出的待处理波形
//  (I)win(NO) - 输出的窗波形
//  (O)dst(src) - 输出的加窗后的波形
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
// void WaveWindow(MWave *src,MWave *win,MWave *dst)
// {    
    // int cn;
    // int i;
    
    // mException((INVALID_WAVE(src)),"invalid input",EXIT);
    // mException((INVALID_WAVE(win)),"invalid input",EXIT);
    // mException((win->size != src->size)
               // ||(win->channel != 1),"invalid windows",EXIT);
        
    // if(INVALID_POINTER(dst))
        // dst = src;
    // else
    // {
        // mWaveRedefine(dst,src->channel,src->size);
        // dst->info = src->info;
    // }

    // for(cn = 0;cn<src->channel;cn++)
        // for(i=0;i<src->size;i++)
            // dst->data[cn][i] = src->data[cn][i] * win->data[cn][i];
// }
    

#define HASH_WaveWithWindow 0x61c83602
struct HandleWaveWithWindow {
    int winSelect;
    MWave *win;
}HandleWaveWithWindow;

void endWaveWithWindow(void *handle)
{
    struct HandleWaveWithWindow *info;
    info = (struct HandleWaveWithWindow *)handle;
    
    if(info->win != NULL)
        mWaveRelease(info->win);
    
    mFree(info);
}

/////////////////////////////////////////////////////////
// 接口功能:
//  对波形加窗
//
// 参数：
//  (I)src(NO) - 输出的待处理波形
//  (O)dst(src) - 输出的加窗后的波形
//  (I)winSelect(MORN_WIN_HAMMING) - 窗类型的选择
//
// 返回值：
//  无
/////////////////////////////////////////////////////////
void mWaveWindow(MWave *src,MWave *dst,int winSelect)
{
    mException((INVALID_WAVE(src)),EXIT,"invalid input");
    
    MHandle *hdl; ObjectHandle(src,WaveWithWindow,hdl);
    struct HandleWaveWithWindow *handle = hdl->handle;
    if(hdl->valid == 0)
    {
        if(handle->win == NULL) handle->win = mWaveCreate(1,src->size,NULL);
        else                    mWaveRedefine(handle->win,1,src->size,NULL);
        
        hdl->valid = 1;
    }
    
    if(winSelect <=0)
        winSelect = MORN_WIN_HAMMING;
    if(handle->winSelect != winSelect)
    {
        if(winSelect == MORN_WIN_HANNING)
            mWaveHanningWin(handle->win);
        else if(winSelect == MORN_WIN_HAMMING)
            mWaveHammingWin(handle->win);
        else if(winSelect == MORN_WIN_FEJER)
            mWaveFejerWin(handle->win);
        else if(winSelect == MORN_WIN_BLACKMAN)
            mWaveBlackmanWin(handle->win);
        else
            mException(1,EXIT,"invalid window select");
        
        handle->winSelect = winSelect;
    }

    mWaveMul(src,handle->win,dst);
}
