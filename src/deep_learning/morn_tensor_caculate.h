/*
Copyright (C) 2019  Jing Lee
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef _MORN_TENSOR_CACULATE_H_
#define _MORN_TENSOR_CACULATE_H_

struct TensorOutputPara
{
    MLayer *prev;
   
    int height;
    int width;
    int channel;
    
    float (*loss)(MLayer *,MLayer *,float *);
    void (*dloss)(MLayer *,MLayer *);
    
    char argv[4][128];
    
    float rate;
    float decay;
    float momentum;
};

#endif

