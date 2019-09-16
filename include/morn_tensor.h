#ifndef _MORN_TENSOR_H_
#define _MORN_TENSOR_H_

#include "morn_Math.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MORN_PREDICT  0
#define MORN_TRAIN    1
#define MORN_FORWARD  0
#define MORN_BACKWARD 1

typedef struct MTensor{
    int batch;
    int channel;
    int height;
    int width;
    
    float **data;
    
    MList *handle;
    void *reserve;
}MTensor;
#define INVALID_TENSOR(Tns) ((((Tns) ==NULL)||((intptr_t)(Tns) == DFLT))?1:(((Tns)->data == NULL)||((intptr_t)((Tns)->data) == DFLT)\
                                                                      ||((Tns)->batch <= 0)||((Tns)->height <= 0)\
                                                                      ||((Tns)->width <= 0)||((Tns)->channel <= 0)\
                                                                      ||((Tns)->handle == NULL)))

MTensor *mTensorCreate(int batch,int channel,int height,int width,float **data);
void mTensorRedefine(MTensor *tns,int batch,int channel,int height,int width,float **data);
void mTensorRelease(MTensor *tns);

void mTensorCopy(MTensor *src,MTensor *dst);
// void *mTensorParaWrite(MList *net,void *para,int size);


typedef struct MLayer
{
    char name[32];
    int type_index;
    MTensor *tns;
    MTensor *res;
    void *para;
    int state;
}MLayer;
#define mLayerType(layer) (morn_tensor_register[layer->type_index].type)
MLayer *mNetworkLayer(MFile *ini,char *name);
// MTensor *mNetworkInput(MList *net,int index);
// MTensor *mNetworkOutput(MList *net,int index);

MList *mNetworkGenerate(MFile *ini);

struct TensorRegister
{
    char type[32];
    void *(*para)(MFile *,char *);
    void  (*forward)(MLayer *);
    void  (*backward)(MLayer *);
};
extern struct TensorRegister morn_tensor_register[256];
extern int morn_tensor_register_num;
void mTensorRegister(char *type,void *(*para)(MFile *,char *),void (*forward)(MLayer *),void (*backward)(MLayer *));
int mTensorRegisterIndex(char *type);
void mTensorRegisterAll();

struct ActvRegister
{
    char name[32];
    float (*func)(float,float *);
    float (*dfunc)(float,float *);
};
extern struct ActvRegister morn_actv_register[64];
extern int morn_actv_register_num;
void mActivationRegister(char *name,float (*func)(float,float *),float (*dfunc)(float,float *));
void mActivationRegisterAll();

struct LossRegister
{
    char name[32];
    float (*loss)(MLayer *,MLayer *,float *);
    void (*dloss)(MLayer *,MLayer *);
};
extern struct LossRegister morn_loss_register[64];
extern int morn_loss_register_num;
void mLossRegister(char *name,float (*loss)(MLayer *,MLayer *,float *),void (*dloss)(MLayer *,MLayer *));
void mLossRegisterAll();

// extern char *morn_network_train_data_dir;

extern char *morn_network_para_dir;
extern char morn_network_para_filename[256];
extern MFile *morn_network_parafile;
extern int morn_network_time;
extern int morn_network_time_max;
extern int morn_network_save;

extern float morn_network_error;
extern float morn_network_error_thresh;

// extern int morn_network_batch;
extern int morn_network_flag;



void mTrainDataGenerate(void *in ,void  (*in_func)(void *,MTensor **,void *),void *in_para ,int in_num,
                        void *out,void (*out_func)(void *,MTensor **,void *),void *out_para,int out_num,
                        char *filename);

void mTrainData(MFile *ini);
// void mPredictData(MFile *ini,char *name[],MTensor *tns[]);
void mNetworkTensor(MFile *ini,char *name[],MTensor *tns[]);

void NetworkTrain(MFile *ini);
void mNetworkTrain(MFile *ini,char *name[],MTensor *tns[]);
void mNetworkPredict(MFile *ini,char *name[],MTensor *tns[]);

void *mTensorInputPara(MFile *ini,char *name);
void mTensorInputForward(MLayer *layer);
void mTensorInputBackward(MLayer *layer);

void *mTensorOutputPara(MFile *ini,char *name);
void mTensorOutputForward(MLayer *layer);
void mTensorOutputBackward(MLayer *layer);

void *mTensorConnectPara(MFile *ini,char *name);
void mTensorConnectForward(MLayer *layer);
void mTensorConnectBackward(MLayer *layer);

void *mTensorConvPara(MFile *ini,char *name);
void mTensorConvForward(MLayer *layer);
void mTensorConvBackward(MLayer *layer);

void *mTensorPoolPara(MFile *ini,char *name);
void mTensorMaxPoolForward(MLayer *layer);
void mTensorMaxPoolBackward(MLayer *layer);

void mTensorAvgPoolForward(MLayer *layer);
void mTensorAvgPoolBackward(MLayer *layer);

void *mTensorActivationPara(MFile *ini,char *name);
void mTensorActivationForward(MLayer *layer);
void mTensorActivationBackward(MLayer *layer);

void *mTensorBatchNormPara(MFile *ini,char *name);
void mTensorBatchNormForward(MLayer *layer);
void mTensorBatchNormBackward(MLayer *layer);

void *mTensorMergePara(MFile *ini,char *name);
void mTensorMergeForward(MLayer *layer);
void mTensorMergeBackward(MLayer *layer);

void *mTensorResizePara(MFile *ini,char *name);
void mTensorResizeForward(MLayer *layer);
void mTensorResizeBackward(MLayer *layer);

void *mTensorMulPara(MFile *ini,char *name);
void mTensorMulForward(MLayer *layer);
void mTensorMulBackward(MLayer *layer);

#define PARA_SAVE (((morn_network_time%morn_network_save == 0)&&(morn_network_time>0))\
                   ||(morn_network_error <= morn_network_error_thresh)\
                   ||(morn_network_error == morn_network_time_max))

#define mNetworkParaWrite(Layer,Name,Data,Size) do{\
    if(PARA_SAVE)\
    {\
        char Para_name[64];sprintf(Para_name,"%s_%s",Layer->name,Name);\
        void *P_data = Data;\
        mMORNWrite(morn_network_parafile,Para_name,&P_data,1,Size);\
    }\
}while(0)
    
#define mNetworkParaRead(Layer,Name,Data,Size) do{\
    char Para_name[64];sprintf(Para_name,"%s_%s",Layer->name,Name);\
    void *P_data = Data;\
    mMORNRead(morn_network_parafile,Para_name,&P_data,1,Size);\
}while(0)

#ifdef __cplusplus
}
#endif

#endif