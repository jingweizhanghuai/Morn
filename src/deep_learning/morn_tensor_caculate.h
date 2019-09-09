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

