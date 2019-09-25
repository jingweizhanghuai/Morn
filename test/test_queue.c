/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//编译： gcc -O2 -fopenmp test_queue.c -I ..\include\ -L ..\lib\x64\mingw\ -lmorn -o test_queue.exe

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "morn_util.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
MList *my_queue;
void write(float *data)
{
    int n=0;
    while(n<100)
    {
        pthread_mutex_lock(&mutex);
        void *p = mQueueWrite(my_queue,data+n,sizeof(float));
        pthread_mutex_unlock(&mutex);
        if(p==NULL) continue;
        n=n+1;
        mSleep(mRand(0,100));
    }
}
void read()
{
    int n=0;
    while(n<100)
    {
        pthread_mutex_lock(&mutex);
        float *data = mQueueRead(my_queue,NULL,DFLT);
        pthread_mutex_unlock(&mutex);
        if(data==NULL) continue;
        n=n+1;
        int size = mQueueSize(my_queue);printf("size is %d\t",size);
        printf("data is %f\n",*data);
        mSleep(mRand(0,100));
    }
}
int main()
{
    float in[100];
    for(int i=0;i<100;i++) in[i]=(float)i;
    
    if(my_queue ==NULL) my_queue = mListCreate(10,NULL);
    
    mThread(2,write(in),read());
    
    mListRelease(my_queue);
    return 0;
}