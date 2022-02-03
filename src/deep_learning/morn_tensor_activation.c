/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
#include "morn_tensor.h"

struct ActvRegister
{
    char name[32];
    float (*func)(float,float *);
    float (*dfunc)(float,float *);
    char *source;
};
struct ActvRegister morn_actv_register[64];
int morn_actv_register_num = 0;

void ActivationRegister(const char *name,float (*func)(float,float *),float (*dfunc)(float,float *),char *source)
{
    int n = morn_actv_register_num;
    morn_actv_register_num = n+1;
    strcpy(morn_actv_register[n].name,name);
    morn_actv_register[n].func  = func;
    morn_actv_register[n].dfunc = dfunc;
    morn_actv_register[n].source= source;
}


/*
float DC_sigmoid_data[256] = {
0.5000000,0.5050473,0.5100933,0.5151372,0.5201780,0.5252145,0.5302459,0.5352711,0.5402891,0.5452991,
0.5502999,0.5552908,0.5602706,0.5652384,0.5701933,0.5751345,0.5800608,0.5849716,0.5898657,0.5947425,
0.5996009,0.6044402,0.6092595,0.6140580,0.6188349,0.6235893,0.6283205,0.6330277,0.6377102,0.6423673,
0.6469983,0.6516023,0.6561789,0.6607273,0.6652469,0.6697371,0.6741972,0.6786268,0.6830252,0.6873919,
0.6917265,0.6960283,0.7002970,0.7045320,0.7087330,0.7128995,0.7170312,0.7211276,0.7251884,0.7292133,
0.7332020,0.7371541,0.7410695,0.7449477,0.7487887,0.7525922,0.7563580,0.7600859,0.7637758,0.7674275,
0.7710410,0.7746160,0.7781525,0.7816505,0.7851098,0.7885306,0.7919126,0.7952560,0.7985607,0.8018267,
0.8050542,0.8082431,0.8113935,0.8145056,0.8175793,0.8206149,0.8236123,0.8265718,0.8294935,0.8323776,
0.8352242,0.8380334,0.8408056,0.8435408,0.8462393,0.8489013,0.8515270,0.8541166,0.8566704,0.8591887,
0.8616716,0.8641194,0.8665325,0.8689110,0.8712553,0.8735656,0.8758422,0.8780855,0.8802957,0.8824730,
0.8846179,0.8867307,0.8888115,0.8908609,0.8928790,0.8948662,0.8968228,0.8987491,0.9006456,0.9025124,
0.9043499,0.9061586,0.9079386,0.9096903,0.9114141,0.9131103,0.9147792,0.9164211,0.9180365,0.9196256,
0.9211887,0.9227262,0.9242384,0.9257257,0.9271883,0.9286267,0.9300410,0.9314318,0.9327992,0.9341436,
0.9354653,0.9367646,0.9380419,0.9392974,0.9405315,0.9417445,0.9429366,0.9441083,0.9452597,0.9463913,
0.9475032,0.9485957,0.9496693,0.9507241,0.9517604,0.9527786,0.9537789,0.9547615,0.9557268,0.9566751,
0.9576065,0.9585214,0.9594200,0.9603025,0.9611693,0.9620206,0.9628567,0.9636777,0.9644839,0.9652756,
0.9660530,0.9668164,0.9675659,0.9683018,0.9690244,0.9697338,0.9704302,0.9711140,0.9717852,0.9724442,
0.9730910,0.9737260,0.9743493,0.9749611,0.9755617,0.9761511,0.9767297,0.9772975,0.9778548,0.9784018,
0.9789386,0.9794654,0.9799824,0.9804897,0.9809876,0.9814761,0.9819555,0.9824259,0.9828875,0.9833404,
0.9837848,0.9842208,0.9846486,0.9850683,0.9854800,0.9858840,0.9862803,0.9866691,0.9870505,0.9874247,
0.9877917,0.9881517,0.9885049,0.9888513,0.9891911,0.9895243,0.9898512,0.9901718,0.9904862,0.9907946,
0.9910971,0.9913937,0.9916845,0.9919698,0.9922495,0.9925238,0.9927927,0.9930565,0.9933151,0.9935686,
0.9938172,0.9940609,0.9942999,0.9945342,0.9947638,0.9949890,0.9952097,0.9954260,0.9956381,0.9958460,
0.9960498,0.9962495,0.9964452,0.9966370,0.9968250,0.9970092,0.9971898,0.9973667,0.9975400,0.9977098,
0.9978763,0.9980393,0.9981990,0.9983555,0.9985088,0.9986589,0.9988060,0.9989501,0.9990911,0.9992293,
0.9993646,0.9994971,0.9996269,0.9997539,0.9998782,1.0000000};
float DCSigmoid(float in,float *argv)
{
    int i=(int)(in*50+0.5); 
    if(i<-255) return 0.0f;
    else if(i<0) return (1.0f-DC_sigmoid_data[0-i]);
    else if(i<255) return DC_sigmoid_data[i];
    else return 1.0f;
}
float DDCSigmoid(float in,float *argv)
{float out=DCSigmoid(in,argv);return (out*(1.0-out));}
*/



float Sigmoid(float in,float *argv)
{return 1.0/(1.0+exp(0.0-in));}
float DSigmoid(float in,float *argv)
{float out=Sigmoid(in,argv);return (out*(1.0-out));}




/*
float DC_tanh_data[256] = {
0.0000000,0.0100946,0.0201867,0.0302745,0.0403560,0.0504290,0.0604918,0.0705422,0.0805783,0.0905982,
0.1006000,0.1105816,0.1205412,0.1304768,0.1403867,0.1502690,0.1601217,0.1699432,0.1797315,0.1894850,
0.1992019,0.2088805,0.2185191,0.2281161,0.2376698,0.2471786,0.2566410,0.2660555,0.2754206,0.2847347,
0.2939966,0.3032047,0.3123579,0.3214547,0.3304939,0.3394742,0.3483945,0.3572536,0.3660505,0.3747839,
0.3834530,0.3920567,0.4005940,0.4090641,0.4174661,0.4257991,0.4340624,0.4422552,0.4503769,0.4584267,
0.4664040,0.4743083,0.4821390,0.4898955,0.4975775,0.5051845,0.5127161,0.5201719,0.5275517,0.5348551,
0.5420820,0.5492320,0.5563050,0.5633010,0.5702197,0.5770611,0.5838252,0.5905120,0.5971214,0.6036535,
0.6101084,0.6164862,0.6227871,0.6290112,0.6351587,0.6412297,0.6472247,0.6531437,0.6589871,0.6647552,
0.6704484,0.6760669,0.6816112,0.6870817,0.6924786,0.6978026,0.7030540,0.7082333,0.7133409,0.7183774,
0.7233432,0.7282389,0.7330650,0.7378221,0.7425106,0.7471312,0.7516845,0.7561710,0.7605914,0.7649461,
0.7692359,0.7734614,0.7776231,0.7817217,0.7857579,0.7897323,0.7936456,0.7974983,0.8012911,0.8050248,
0.8086999,0.8123172,0.8158772,0.8193806,0.8228282,0.8262206,0.8295584,0.8328423,0.8360730,0.8392511,
0.8423774,0.8454524,0.8484768,0.8514514,0.8543767,0.8572534,0.8600821,0.8628636,0.8655983,0.8682871,
0.8709305,0.8735292,0.8760837,0.8785948,0.8810630,0.8834890,0.8858733,0.8882166,0.8905195,0.8927825,
0.8950063,0.8971915,0.8993386,0.9014482,0.9035209,0.9055572,0.9075578,0.9095231,0.9114537,0.9133501,
0.9152130,0.9170427,0.9188399,0.9206051,0.9223387,0.9240413,0.9257133,0.9273553,0.9289678,0.9305512,
0.9321060,0.9336327,0.9351318,0.9366036,0.9380487,0.9394675,0.9408604,0.9422279,0.9435704,0.9448883,
0.9461821,0.9474520,0.9486986,0.9499223,0.9511234,0.9523023,0.9534594,0.9545951,0.9557097,0.9568036,
0.9578772,0.9589308,0.9599648,0.9609795,0.9619752,0.9629523,0.9639111,0.9648519,0.9657750,0.9666808,
0.9675696,0.9684416,0.9692972,0.9701365,0.9709601,0.9717680,0.9725606,0.9733382,0.9741010,0.9748493,
0.9755834,0.9763034,0.9770097,0.9777025,0.9783821,0.9790486,0.9797024,0.9803436,0.9809725,0.9815892,
0.9821941,0.9827873,0.9833691,0.9839395,0.9844990,0.9850476,0.9855855,0.9861129,0.9866301,0.9871372,
0.9876344,0.9881219,0.9885998,0.9890683,0.9895277,0.9899780,0.9904194,0.9908521,0.9912763,0.9916920,
0.9920995,0.9924989,0.9928904,0.9932740,0.9936500,0.9940184,0.9943795,0.9947333,0.9950800,0.9954197,
0.9957525,0.9960786,0.9963981,0.9967110,0.9970176,0.9973179,0.9976120,0.9979001,0.9981823,0.9984586,
0.9987292,0.9989942,0.9992537,0.9995078,0.9997565,1.0000000};
float DCTanh(float in,float *argv)
{
    int i=(int)(in*100+0.5); 
    if(i<-255) return -1.0f;
    else if(i<0) return (0.0f-DC_tanh_data[0-i]);
    else if(i<255) return DC_tanh_data[i];
    else return 1.0f;
}
float DDCTanh(float in,float *argv)
{float out = DCTanh(in,argv);return (1.0-out*out);}
*/

float Tanh(float in,float *argv)
{return (1.0-2.0/(exp(in+in)+1.0));}
float DTanh(float in,float *argv)
{float out = Tanh(in,argv);return (1.0-out*out);}

float ReLu(float in,float *argv)
{return ((in>0.0)?in:0.0);}
float DReLu(float in,float *argv)
{return ((in>0.0)?1.0:0.0);}

float PReLu(float in,float *argv)
{if(argv[0]<0) argv[0]=0; return ((in>in*argv[0])?in:in*argv[0]);}
float DPReLu(float in,float *argv)
{return (((argv[0]>1.0)==(in>0.0))?argv[0]:1.0);}

float ELu(float in,float *argv)
{return (argv[0]*((in>0.0)?in:(exp(in)-1.0)));}
float DELu(float in,float *argv)
{return (argv[0]*((in>0.0)?1.0:in+1.0));}

float Softplus(float in,float *argv)
{return (log(1.0+exp(in)));}
float DSoftplus(float in,float *argv)
{return (1.0/(1.0+exp(0.0-in)));}

float Abs(float in,float *argv)
{return ((in>0.0)?in:(0.0-in));}
float DAbs(float in,float *argv)
{return ((in>0.0)?1.0:-1.0);}

#ifdef MORN_USE_CL
char *CL_Sigmoid =mString(
float activation(float in,float *argv)
{return (1.0/(1.0+exp(0.0-in)));}
float dactivation(float in,float *argv)
{float out=(1.0/(1.0+exp(0.0-in)));return (out*(1.0-out));}
);

char *CL_Tanh =mString(
float activation(float in,float *argv)
{return (1.0-2.0/(exp(in+in)+1.0));}
float dactivation(float in,float *argv)
{float out = Tanh(in,argv);return (1.0-out*out);}
);

char *CL_ReLu =mString(
float activation(float in,float *argv)
{return (in>0.0)?in:0.0;}
float dactivation(float in,float *argv)
{return ((in>0.0)?1.0:0.0);}
);

char *CL_PReLu =mString(
float activation(float in,float *argv)
{if(argv[0]<0) argv[0]=0; return ((in>in*argv[0])?in:in*argv[0]);}
float dactivation(float in,float *argv)
{return (((argv[0]>1.0)==(in>0.0))?argv[0]:1.0);}
);

char *CL_ELu =mString(
float activation(float in,float *argv)
{return (argv[0]*((in>0.0)?in:(exp(in)-1.0)));}
float dactivation(float in,float *argv)
{return (argv[0]*((in>0.0)?1.0:in+1.0));}
);

char *CL_Softplus =mString(
float activation(float in,float *argv)
{return (log(1.0+exp(in)));}
float dactivation(float in,float *argv)
{return (1.0/(1.0+exp(0.0-in)));}
);

char *CL_Abs =mString(
float Abs(float in,float *argv)
{return ((in>0.0)?in:(0.0-in));}
float DAbs(float in,float *argv)
{return ((in>0.0)?1.0:-1.0);}
);

char *activation_cl = mString(
__kernel void activation_gpu(__global const float* in,__global float* out,float argv[4])
{
    const int i = get_global_id(0);
    out[i]=activation(in[i],argv);
});
char *dactivation_cl = mString(
__kernel void dactivation_gpu(__global const float* in,__global const float* out,__global float *res,float argv[4],int flag)
{
    const int i = get_global_id(0);
    float data = out[i]*dactivation(in[i],argv);
    res[i] = (flag)?data:(res[i]+data);
});
#endif

void mActivationRegisterAll()
{
    mActivationRegister(Sigmoid );
    mActivationRegister(Tanh    );
    mActivationRegister(ReLu    );
    mActivationRegister(PReLu   );
    mActivationRegister(ELu     );
    mActivationRegister(Softplus);
    mActivationRegister(Abs     );
}

struct TensorActivationPara
{
    MLayer *prev;
    
    int res_valid;
    struct ActvRegister *actv_register;

    // float (*func)(float,float *);
    // float (*dfunc)(float,float *);
    float argv[4];
};
void *mTensorActivationPara(MSheet *ini,char *name)
{
    struct TensorActivationPara *para = (struct TensorActivationPara *)mMalloc(sizeof(struct TensorActivationPara));
   
    para->prev = mNetworkLayer(ini,mINIRead(ini,name,"prev"));
    mException((para->prev == NULL),EXIT,"invalid prev");
    para->res_valid = (strcmp("Input",mLayerType(para->prev))!=0);
    
    char *value = mINIRead(ini,name,"actv_func");
    if(value != NULL) value = "ReLu";
    int i;for(i=0;i<morn_actv_register_num;i++)
    {
        if(strcmp(morn_actv_register[i].name,value)==0)
        {
            para->actv_register = &(morn_actv_register[i]);
            break;
        }
    }
    mException((i==morn_actv_register_num),EXIT,"invalid activation function");
    
    para->argv[0]=DFLT; mINIRead(ini,name,"argv0","%f",&(para->argv[0]));
    para->argv[1]=DFLT; mINIRead(ini,name,"argv1","%f",&(para->argv[1]));
    para->argv[2]=DFLT; mINIRead(ini,name,"argv2","%f",&(para->argv[2]));
    para->argv[3]=DFLT; mINIRead(ini,name,"argv3","%f",&(para->argv[3]));
    
    return para;
}
struct HandleTensorActivation
{
    char *source;
};
void endTensorActivation(struct HandleTensorActivation *handle)
{
    if(handle->source != NULL) free(handle->source);
}
#define HASH_TensorActivation 0xbd8effc8
struct HandleTensorActivation *TensorActivationSet(MLayer *layer)
{
    struct TensorActivationPara *para = (struct TensorActivationPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out=layer->tns;

    struct HandleTensorActivation *handle=NULL;
    #ifdef MORN_USE_CL
    MHandle *hdl=mHandle(out,TensorActivation);
    handle = (struct HandleTensorActivation *)(hdl->handle);
    
    int size = strlen(activation_cl)+strlen(dactivation_cl)+strlen(para->actv_register->source)+8;
    handle->source = malloc(size);
    sprintf("%s\n%s\n%s\n",para->actv_register->source,activation_cl,dactivation_cl);
    #endif
    
    if(layer->state != DFLT) return handle;
    mTensorRedefine(out,in->batch,in->channel,in->height,in->width,NULL);
    if(morn_network_flag == MORN_TRAIN)
        mTensorRedefine(res,in->batch,in->channel,in->height,in->width,NULL);
        
    return handle;
}

void mTensorActivationForward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Activation",mLayerType(layer)),EXIT,"invalid layer type");
    
    struct TensorActivationPara *para = (struct TensorActivationPara *)(layer->para);
    MTensor *in = para->prev->tns;
    MTensor *out=layer->tns;
    struct ActvRegister *actv_register = para->actv_register;
    
    struct HandleTensorActivation *handle = TensorActivationSet(layer);
    int size = in->channel*in->height*in->width;

    #ifdef MORN_USE_CL
    char *activation_gpu = handle->source;
    for(int b=0;b<in->batch;b++)
    {
        mCLFunction(activation_gpu,CLSIZE(size),CLIN(mTensorMemory(in,b)),CLOUT(mTensorMemory(out,b)),CLPARA(para->argv,4*sizeof(float)));
    }
    #else
    for(int b=0;b<in->batch;b++)
    {
        float *in_data = in->data[b];
        float *out_data=out->data[b];
        for(int i=0;i<size;i++)
            out_data[i] = (actv_register->func)(in_data[i],para->argv);
    }
    #endif
    layer->state = MORN_FORWARD;
}

void mTensorActivationBackward(MLayer *layer)
{
    mException(INVALID_POINTER(layer),EXIT,"invalid input");
    mException(strcmp("Activation",mLayerType(layer)),EXIT,"invalid layer type");
    struct TensorActivationPara *para = (struct TensorActivationPara *)(layer->para);
    if(para->res_valid==0) return;
    MTensor *in = para->prev->tns;
    MTensor *res= para->prev->res;
    MTensor *out= layer->res;
    struct ActvRegister *actv_register = para->actv_register;
    
    int size = in->channel*in->height*in->width;
    int flag = (para->prev->state==MORN_FORWARD);
    
    #ifdef MORN_USE_CL
    MHandle *hdl=mHandle(out,TensorActivation);
    struct HandleTensorActivation *handle = (struct HandleTensorActivation *)(hdl->handle);
    char *dactivation_gpu = handle->source;
    for(int b=0;b<in->batch;b++)
    {
        mCLFunction(dactivation_gpu,CLSIZE(size),CLIN(mTensorMemory(in,b)),CLIN(mTensorMemory(out,b)),CLOUT(mTensorMemory(res,b)),CLPARA(para->argv,4*sizeof(float)),CLPARA(&flag,sizeof(int)));
    }
    #else
    for(int bc=0;bc<in->batch;bc++)
    {
        float * in_data= in->data[bc];
        float *res_data=res->data[bc];
        float *out_data=out->data[bc];
        for(int i=0;i<size;i++)
        {
            float data = out_data[i]*((actv_register->dfunc)(in_data[i],para->argv));
            res_data[i] = (flag)?data:(res_data[i]+data);
        }
    }
    #endif
    
    para->prev->state = MORN_BACKWARD;
}


