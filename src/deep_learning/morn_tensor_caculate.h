/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
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

