/*
Copyright (C) 2019-2020 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

#ifndef _MORN_IMAGE_CACULATE_H_
#define _MORN_IMAGE_CACULATE_H_

#define ToU8(A) ((A<0)?0:((A>255)?255:A))

#define REPEAT16(Expr) do{Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;}while(0)
#define REPEAT(N,Expr) do{\
    if(N== 1) {Expr;}\
    if(N== 2) {Expr;Expr;}\
    if(N== 3) {Expr;Expr;Expr;}\
    if(N== 4) {Expr;Expr;Expr;Expr;}\
    if(N== 5) {Expr;Expr;Expr;Expr;Expr;}\
    if(N== 6) {Expr;Expr;Expr;Expr;Expr;Expr;}\
    if(N== 7) {Expr;Expr;Expr;Expr;Expr;Expr;Expr;}\
    if(N== 8) {Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;}\
    if(N== 9) {Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;}\
    if(N==10) {Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;}\
    if(N==11) {Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;}\
    if(N==12) {Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;}\
    if(N==13) {Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;}\
    if(N==14) {Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;}\
    if(N==15) {Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;Expr;}\
}while(0)

#define DO_REPEAT16(Expr,I) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);Expr(I,6);Expr(I,7);Expr(I,8);Expr(I,9);Expr(I,10);Expr(I,11);Expr(I,12);Expr(I,13);Expr(I,14);Expr(I,15);}
#define DO_REPEAT(N,Expr,I) {\
         if(N== 0) NULL;\
    else if(N== 1) {Expr(I,0);}\
    else if(N== 2) {Expr(I,0);Expr(I,1);}\
    else if(N== 3) {Expr(I,0);Expr(I,1);Expr(I,2);}\
    else if(N== 4) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);}\
    else if(N== 5) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);}\
    else if(N== 6) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);}\
    else if(N== 7) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);Expr(I,6);}\
    else if(N== 8) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);Expr(I,6);Expr(I,7);}\
    else if(N== 9) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);Expr(I,6);Expr(I,7);Expr(I,8);}\
    else if(N==10) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);Expr(I,6);Expr(I,7);Expr(I,8);Expr(I,9);}\
    else if(N==11) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);Expr(I,6);Expr(I,7);Expr(I,8);Expr(I,9);Expr(I,10);}\
    else if(N==12) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);Expr(I,6);Expr(I,7);Expr(I,8);Expr(I,9);Expr(I,10);Expr(I,11);}\
    else if(N==13) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);Expr(I,6);Expr(I,7);Expr(I,8);Expr(I,9);Expr(I,10);Expr(I,11);Expr(I,12);}\
    else if(N==14) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);Expr(I,6);Expr(I,7);Expr(I,8);Expr(I,9);Expr(I,10);Expr(I,11);Expr(I,12);Expr(I,13);}\
    else if(N==15) {Expr(I,0);Expr(I,1);Expr(I,2);Expr(I,3);Expr(I,4);Expr(I,5);Expr(I,6);Expr(I,7);Expr(I,8);Expr(I,9);Expr(I,10);Expr(I,11);Expr(I,12);Expr(I,13);Expr(I,14);}\
}
#define RowCalculate(Expr,Width) do{\
    int I=0;for(I=0;I<Width-15;I+=16) DO_REPEAT16(Expr,I)\
    DO_REPEAT(Width-I,Expr,I)\
}while(0)





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
