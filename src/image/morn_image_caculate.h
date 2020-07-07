/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#ifndef _MORN_IMAGE_CACULATE_H_
#define _MORN_IMAGE_CACULATE_H_

#if defined _MSC_VER
#define mPragma(InFo) __pragma(InFo)
#elif defined __GNUC__
#define mPragma(InFo) _Pragma(#InFo)
#else
#define mPragma(InFo)
#endif

#define mImageRegion(Src,R,Process) do{\
    int K;\
    mPragma(omp parallel for)\
    for(K=ImageY1(Src);K<ImageY2(Src);K=K+(16-R-R))\
    {\
        int X1=Src->width;int X2=0;int X3=Src->width;int X4=0;\
        if(Src->border !=NULL)\
            for(int J=K;J<MIN(K+(16-R-R),ImageY2(Src));J++)\
            {\
                int Border;\
                Border = ImageX1(Src,J); if(Border<X1) X1=Border; if(Border>X2) X2=Border;\
                Border = ImageX2(Src,J); if(Border<X3) X3=Border; if(Border>X4) X4=Border;\
            }\
        \
        for(int I=X1;I<X2;I++)\
            for(int J=K;J<MIN(K+(16-R-R),ImageY2(Src));J++)\
                if(I>ImageX1(Src,J))\
                    Process(I,J)\
        \
        for(int I=X2;I<X3;I++)\
            for(int J=K;J<MIN(K+(16-R-R),ImageY2(Src));J++)\
                Process(I,J)\
        \
        for(int I=X3;I<X4;I++)\
            for(int J=K;J<MIN(K+(16-R-R),ImageY2(Src));J++)\
                if(I<ImageX2(Src,J))\
                    Process(I,J)\
    }\
}while(0)

#endif