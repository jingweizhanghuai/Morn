/*
Copyright (C) 2019  JingWeiZhangHuai
This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
                Border = ImageX1(Src,J); if(Border<X1) X1=Border; else if(Border>X2) X2=Border;\
                Border = ImageX2(Src,J); if(Border<X3) X3=Border; else if(Border>X4) X4=Border;\
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