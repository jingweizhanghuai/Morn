## 定义

### 数据结构

----

#### 波形

```c
typedef struct MWave {
    int cn;
    int size;
    
    float *data[MORN_MAX_WAVE_CN];
    
    MMemory *memory;
    MHandleSet *handle;
    
    struct
    {
        int frequency;
        int wave_type;
        float normalize_value;
    }info;
    
    void *reserve;
}MWave;
```

##### 成员

* cn：波形的通道数，最多支持32通道
* size：波形点的个数
* data：波形中各点的值（MORN_MAX_WAVE_CN为32）
* memory：波形内存（不建议外部使用）
* handle：波形操作集（不建议外部使用）
* info.frequency：波形的采样率
* info.wave_type：波形的类型（时域、频域等）
* info.normalize_value：波形的归一化值
* reserve：预留

----

### 函数

----

#### 创建波形

```c
MWave *mWaveCreate(int cn,int size,float **data)
```

##### 参数

* cn：输入，所创建波形的通道数，取值应在[0,MORN_MAX_WAVE_CN]之间
* size：输入，所创建波形的大小，值应大于等于0
* data：输入，初始化各通道波形点的值，传入为NULL，则不对波形初始化

#####返回值

所创建波形的指针

##### 备注

当不确定向量的维数时，可使用 

```c
mWaveCreate(DFLT,DFLT,NULL);
```

当只创建向量，不初始化时，可使用

```c
mWaveCreate(cn,size,NULL);
```

----

#### 波形释放

```c
void mWaveRelease(MWave *wave)
```

##### 参数

* wave：输入，所需释放的波形

##### 返回值

无

----

#### 波形重定义

```c
void mWaveRedefine(MWave *src,int cn,int size)
```

##### 参数

* src：输入，所需重定义的波形
* cn：输入，重定义后波形的通道数，传入为NULL则通道数不变
* size：输入，重定义后波形中点的个数，传入为NULL则波形点的个数不变

##### 返回值

无

----

#### 波形截取

```c
void mWaveCut(MWave *src,MWave *dst,int locate,int size)
```

##### 参数

* src：输入，所截取的源波形
* dst：输出，截取后的波形，若传入为NULL则源波形仅保留截取后的部分
* locate：输入，截取位置，若传入默认值（DFLT），则从位置0开始截取
* size：输入，截取的波形点的个数

##### 返回值

无

##### 备注

以下调用，含义如注释

```c
mWaveCut(src,dst,locate,size);	//从src的locate处截取长度为size的波形，存入dst
mWaveCut(src,NULL,locate,size);	//保留src的从locate处始，长度为size的
mWaveCut(src,dst,DFLT,size);	//从src的起始位置截取长度为size的波形，存入dst
mWaveCut(src,NULL,DFLT,size);	//保留src的从起始位置起，长度为size的
mWaveCut(src,dst,locate,DFLT);	//截取src从locate处始，长度为dst->size的波形，存入dst
mWaveCut(src,NULL,locate,DFLT);	//保留src从locate处始至末尾的波形
mWaveCut(src,dst,DFLT,DFLT);	//复制src到dst
```

----



## 波形状态

### 函数

----

#### 波形均值

```c
void mWavMean(MWave *src,float *mean)
```

##### 参数

* src：输入，待统计的源波形
* mean：输出，各通道波形的均值

##### 返回值

无

----

#### 波形绝对值均值

```c
void mWavABSMean(MWave *src,float *mean)
```

##### 参数

- src：输入，待统计的源波形
- mean：输出，各通道波形的绝对值均值

##### 返回值

无

----

#### 波形平方均值

```c
void mWavSquarMean(MWave *src,float *mean)
```

##### 参数

- src：输入，待统计的源波形
- mean：输出，各通道波形的平方均值

##### 返回值

无

----

## 波形运算

#### 函数

----

#### 波形加法

``` c
void mWaveAdd(MWave *src1,MWave *src2,MWave *dst)
```

##### 参数

* src1：输入，源波形
* src2：输入，源波形
* dst：输出，相加后波形，若传入为NULL，则输出波形存入src1

##### 返回值

无

----

#### 波形减法

```c
void mWaveSub(MWave *src1,MWave *src2,MWave *dst)
```

##### 参数

- src1：输入，源波形
- src2：输入，源波形
- dst：输出，相减（src1减src2）后波形，若传入为NULL，则输出波形存入src1

##### 返回值

无

----

#### 平均波形

``` c
void mWaveAverage(MWave *src1,MWave *src2,MWave *dst)
```

##### 参数

- src1：输入，源波形
- src2：输入，源波形
- dst：输出，src1和src2的平均波形，若传入为NULL，则输出波形存入src1

##### 返回值

无

----

#### 加权平均波形

``` c
void mWaveWeightedAverage(MWave *src1,MWave *src2,MWave *dst,float weight1,float weight2)
```

- src1：输入，源波形
- src2：输入，源波形
- dst：输出，src1和src2的加权平均波形，若传入为NULL，则输出波形存入src1
- weight1：输入，src1的权重
- weight2：输入，src2的权重

##### 返回值

无

----

####波形幅值缩放

``` c
void mWaveScale(MWave *src,MWave *dst,float k)
```

##### 参数

* src：输入，源波形
* dst：输出，缩放后的波形
* k：输入，缩放系数

##### 返回值

无

----

#### 波形乘法

```c
void mWaveMul(MWave *src1,MWave *src2,MWave *dst)
```

##### 参数

- src1：输入，源波形
- src2：输入，源波形
- dst：输出，相乘后波形，若传入为NULL，则输出波形存入src1

##### 返回值

无

------

#### 波形加窗

``` c
void mWaveWindow(MWave *src,MWave *dst,int winSelect)
```

##### 参数

* src：输入，源波形
* dst：输出，加窗后的波形，若传入为NULL，则输出波形存入src
* winselect：输入，所加窗的类型，详见备注

##### 返回值

无

##### 备注

所支持的加窗类型包括：Hanning窗（winselect==MORN_WIN_HANNING），HammingWin窗（winselect==MORN_WIN_HAMMING），三角窗（winselect==MORN_WIN_FEJER），Blackman窗（winselect==MORN_WIN_BLACKMAN）

----

#### 波形除法

```c
void mWaveDiv(MWave *src1,MWave *src2,MWave *dst)
```

##### 参数

- src1：输入，源波形
- src2：输入，源波形
- dst：输出，相乘后波形，若传入为NULL，则输出波形存入src1

##### 返回值

无

----

#### 波形阈值限定

``` c
void mWaveThreshold(MWave *src,MWave *dst,MThreshold *thresh)
```

##### 参数

* src：输入，源波形
* dst：输出，阈值限定后的波形，若传入为NULL，则输出波形存入src
* thresh：输入，阈值

##### 返回值

无

----













----

