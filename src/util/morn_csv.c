#include "morn_util.h"

struct HandleCSV
{
    MArray *array;

    MMap **row_map;
    MChain *col_map;

    int col;
    int row;

    char key[256];
    int key_idx;

    int idx0;

    MList **col_list;

    MList *list;

    uint8_t *file;
};
void endCSV(struct HandleCSV *handle)
{
    if(handle->row_map!=NULL)
    {
        for(int i=0;i<handle->col;i++) if(handle->row_map[i]!=NULL) mMapRelease(handle->row_map[i]);
        mFree(handle->row_map);
    }
    if(handle->col_list!=NULL)
    {
        for(int i=0;i<handle->col;i++) if(handle->col_list[i]!=NULL) mListRelease(handle->col_list[i]);
        mFree(handle->col_list);
    }

    if(handle->array  !=NULL) mArrayRelease(handle->array  );
    if(handle->col_map!=NULL) mChainRelease(handle->col_map);
    if(handle->list   !=NULL)  mListRelease(handle->list   );
    if(handle->file   !=NULL)         mFree(handle->file   );
}
#define HASH_CSV 0x1e62f363

void CSVKey(char *key,struct HandleCSV *handle)
{
    if(handle->col_map==NULL) return;
    int key_idx=0;
    mornMapRead(handle->col_map,key,DFLT,&key_idx,NULL);
    handle->key_idx = key_idx;
    if(handle->row_map[key_idx]!=NULL) return;

    MMap *map = mMapCreate();
    int overwrite=0; mPropertyWrite(map,"overwrite",&overwrite,sizeof(int));
    for(int i=0;i<handle->row;i++)
    {
        int idx = handle->array->dataS32[i*handle->col+key_idx];
        char *p;
        if(idx>=0) p=(char *)(handle->file)+idx;
        else p=(char *)(handle->list->data[-2-idx])+1;
        mMapWrite(map,p,DFLT,&i,sizeof(int));
    }
    handle->row_map[key_idx] = map;
}

MFile *morn_csv_file = NULL;
struct HandleCSV *morn_csv_handle = NULL;

void mCSVLoad(MFile *csv)
{
    MHandle *hdl = mHandle(csv,CSV);
    struct HandleCSV *handle = hdl->handle;
    if(hdl->valid ==0)
    {
        mPropertyVariate(csv,"key",handle->key,DFLT);
        mPropertyFunction(csv,"key",CSVKey,handle);
        if(handle->array  ==NULL) handle->array  = mArrayCreate(1024,sizeof(int));
        if(handle->col_map==NULL) handle->col_map= mChainCreate();
        if(handle->list   ==NULL) handle->list   = mListCreate();

        hdl->valid = 1;
    }

    FILE *f = fopen(csv->filename,"rb");
    int size = fsize(f);
    if(handle->file != NULL) mFree(handle->file);
    handle->file = mMalloc(size+3);handle->file[0]=0;
    fread(handle->file+1,1,size,f);
    fclose(f);
    handle->file[size+1]='\n';handle->file[size+2]=' ';
//     printf("size=%d\n",size);

    int flag=0;
    uint8_t *p = handle->file+1;while(*p>=128) p++;
    int *idx = handle->array->dataS32;idx[0]=1;
    int n=1;int n0=0;
    while(p<handle->file+size+1)
    {
        if(((*p==',')||(*p<32))&&(flag%2==0))
        {
            if(flag==2) {idx[n-1]++;for(int i=-1;;i--) {if(p[i]=='"') {p[i]=0;break;}}}
            else if(flag>2)
            {
                idx[n-1]++;
                uint8_t *p0 = handle->file+idx[n-1];
                for(uint8_t *p1=p0;p1<p;p1++) {if(*p1=='"') {p1++;} *p0=*p1;p0++;}
                p0[-1]=0;
            }
            flag = 0;
            if(*p<32)
            {
//                 printf("nv=%d,idx[n]=%d,%s\n",n-1,idx[n-1],handle->file+idx[n-1]);
                *p=0;if(p[1]<32) {p++;*p=0;}
                idx[n]=p-handle->file+1;
                p++;n++;break;
            }
            else {*p=0;idx[n]=p-handle->file+1;n++;}
        }
        if(*p=='"') {flag++;} p++;
    }
    handle->col=n-1;

//     printf("handle->col=%d\n",handle->col);
//     return;

    handle->row_map = mMalloc(handle->col*sizeof(MMap * ));memset(handle->row_map ,0,handle->col*sizeof(MMap * ));
    handle->col_list= mMalloc(handle->col*sizeof(MList *));memset(handle->col_list,0,handle->col*sizeof(MList *));

    int key_idx=0;
    if(handle->key[0]==0) key_idx=0;
//     printf("handle->key=%s\n",handle->key);
//     printf("handle->key[0]=%d\n",handle->key[0]);
    for(int i=0;i<handle->col;i++)
    {
        uint8_t *p=handle->file+idx[i];
        mornMapWrite(handle->col_map,p,DFLT,&i,sizeof(int));
        if(handle->key[0]!=0) {if(strcmp(handle->key,(char *)p)==0) key_idx=i;}
    }
//     printf("key_idx=%d\n",key_idx);
//     exit(0);
    handle->key_idx = key_idx;
    handle->row_map[key_idx] = mMapCreate();
    MMap *row_map = handle->row_map[key_idx];

    mMapWrite(row_map,handle->file+idx[key_idx],DFLT,&(handle->row),sizeof(int));

    n0=n-1;
    handle->row=1;

    while(p<=handle->file+size+1)
    {
        if(((*p==',')||(*p<32))&&(flag%2==0))
        {
            if(flag==2) {idx[n-1]++;for(int i=-1;;i--) {if(p[i]=='"') {p[i]=0;break;}}}
            else if(flag>2)
            {
                idx[n-1]++;
                uint8_t *p0 = handle->file+idx[n-1];
                for(uint8_t *p1=p0;p1<p;p1++) {if(*p1=='"') {p1++;} *p0=*p1;p0++;}
                p0[-1]=0;
            }
            flag=0;

            if(*p<32)
            {
//                 printf("nnnnnnn=%d,n0=%d,key_idx=%d\n",n,n0,key_idx);
                if(n%handle->col!=0) break;
                *p=0;
//                 printf("key=%s\n",handle->file+idx[n0+key_idx]);
                mMapWrite(row_map,handle->file+idx[n0+key_idx],DFLT,&(handle->row),sizeof(int));
                if(p[1]<32) {p++;*p=0;}
                idx[n]=p-handle->file+1;n0=n;n++;
                handle->row+=1;
                if(n+handle->col>=handle->array->num) {mArrayAppend(handle->array,handle->array->num+handle->col*handle->row);idx=handle->array->dataS32;}
            }
            else
            {
                *p=0;
//                 if(n>=340000) printf("n=%d,idx[n]=%d,%s\n",n-1,idx[n-1],handle->file+idx[n-1]);
                idx[n]=p-handle->file+1;n++;
            }
        }
        if(*p=='"') {flag++;} p++;
    }
    handle->array->num=n0;
//     printf("handle->array->num=%d,n0=%d\n",handle->array->num,n0);
    mPropertyWrite(csv,"csv_row",&(handle->row),sizeof(int));
    mPropertyWrite(csv,"csv_col",&(handle->col),sizeof(int));

    morn_csv_handle = handle;
    morn_csv_file = csv;
}

struct HandleCSV *CSVHandle(MFile *csv)
{
    struct HandleCSV *handle=NULL;
    if(csv==morn_csv_file) handle=morn_csv_handle;
    else
    {
        MHandle *hdl = mHandle(csv,CSV);
        if(hdl->valid==0) mCSVLoad(csv);
        handle=hdl->handle;
    }
    return handle;
}

char *m_CSVRead(MFile *csv,intptr_t row,intptr_t col,const char *format,...)
{
    struct HandleCSV *handle=CSVHandle(csv);
    int row_idx,col_idx;char *p;

    if(row>handle->row)
    {
        char *name = (char *)row;
        p=mMapRead(handle->row_map[handle->key_idx],name,DFLT,&row_idx,NULL);
        if(p==NULL) return NULL;
    }
    else row_idx=row;

    if(col>handle->col)
    {
        char *name = (char *)col;
        p=mornMapRead(handle->col_map,name,DFLT,&col_idx,NULL);
        if(p==NULL) return NULL;
    }
    else col_idx=col;

//     printf("row_idx=%d,col_idx=%d\n",row_idx,col_idx);

    int idx=row_idx*handle->col+col_idx;
    idx = handle->array->dataS32[idx];

    if(idx>=0) p=(char *)(handle->file)+idx;
    else p=(char *)(handle->list->data[-2-idx])+1;

    if(!INVALID_POINTER(format))
    {
        va_list inipara;
        va_start(inipara,format);
        vsscanf(p,format,inipara);
        va_end(inipara);
    }

    return p;
}

MList *m_CSVCol(MFile *csv,intptr_t col)
{
    struct HandleCSV *handle=CSVHandle(csv);
    int col_idx;char *p;

    if(col>handle->col)
    {
        char *name = (char *)col;
        printf("name=%s\n",name);
        p=mornMapRead(handle->col_map,name,DFLT,&col_idx,NULL);
        printf("pppppppppppppppppp=%p\n",p);
        if(p==NULL) return NULL;
    }
    else col_idx=col;

    printf("col_idx=%d\n",col_idx);
    if(handle->col_list[col_idx]==NULL) handle->col_list[col_idx]=mListCreate();
    MList *col_list = handle->col_list[col_idx];
    printf("aaaaaaaaaaaaaaa\n");
    mListClear(col_list);
    for(int i=0;i<handle->row;i++)
    {
        int idx=i*handle->col+col_idx;
        idx = handle->array->dataS32[idx];

        if(idx>=0) p=(char *)(handle->file)+idx;
        else p=(char *)(handle->list->data[-2-idx])+1;

        mListWrite(col_list,DFLT,p,DFLT);
    }

    return col_list;
}

void mCSVColAppend(MFile *csv,int col)
{
    struct HandleCSV *handle = CSVHandle(csv);
    if(col<0) col=handle->col;
    MArray *array = mArrayCreate((handle->col+1)*handle->row,sizeof(int));

    int *p0 = handle->array->dataS32;int col0 = handle->col;
    int *p1 =         array->dataS32;int col1 = col0 + 1;

    memcpy(p1,p0,col);p1[col]=handle->idx0;p1+=col+1;p0+=col;
    for(int i=1;i<handle->row;i++)
    {
        memcpy(p1,p0,col0);p1[col0]=handle->idx0;
        p1+=col1;p0+=col0;
    }
    memcpy(p1,p0,col0-col);
    handle->col = col1;

    MMap **row_map = mMalloc(col1*sizeof(MMap *));
    memcpy(row_map,handle->row_map,col*sizeof(MMap *));
    row_map[col]=NULL;
    memcpy(row_map+col+1,handle->row_map+col,(col0-col)*sizeof(MMap *));
    mFree(handle->row_map);handle->row_map = row_map;

    MList **col_list = mMalloc(col1*sizeof(MList *));
    memcpy(col_list,handle->col_list,col*sizeof(MList *));
    col_list[col]=NULL;
    memcpy(col_list+col+1,handle->col_list+col,(col0-col)*sizeof(MList *));
    mFree(handle->col_list);handle->col_list = col_list;

    mArrayRelease(handle->array);
    handle->array = array;

    for(int i=col+1;i<col1;i++)
    {
        int idx = p1[i];char *p;
        if(idx>=0) p=(char *)(handle->file)+idx;
        else p=(char *)(handle->list->data[-2-idx])+1;

        mornMapWrite(handle->col_map,p,DFLT,&i,sizeof(int));
    }
}

void mCSVColDelete(MFile *csv,int col)
{
    struct HandleCSV *handle = CSVHandle(csv);
    if(col<0) col=handle->col-1;

    int *p0 = handle->array->dataS32;int col0 = handle->col;
    int *p1 = handle->array->dataS32;int col1 = col0 - 1;

    memmove(p1,p0,col);p1+=col;p0+=col+1;
    for(int i=1;i<handle->row;i++)
    {
        memmove(p1,p0,col1);
        p1+=col1;p0+=col0;
    }
    memmove(p1,p0,col1-col);
    handle->col = col1;

    memmove(handle->row_map +col,handle->row_map +col+1,(col1-col)*sizeof( MMap *));
    memmove(handle->col_list+col,handle->col_list+col+1,(col1-col)*sizeof(MList *));

    for(int i=col;i<col1;i++)
    {
        int idx = p1[i];char *p;
        if(idx>=0) p=(char *)(handle->file)+idx;
        else p=(char *)(handle->list->data[-2-idx])+1;

        mornMapWrite(handle->col_map,p,DFLT,&i,sizeof(int));
    }
}

void mCSVRowAppend(MFile *csv,intptr_t row)
{
    struct HandleCSV *handle = CSVHandle(csv);
    mArrayAppend(handle->array,handle->array->num+handle->col);
    if(row<0) row=handle->row;
    int *idx = handle->array->dataS32;
    if(row<handle->row)
        memmove(idx+(row+1)*handle->col,idx+row*handle->col,(handle->row-row)*handle->col*sizeof(int));
    memset(idx+row*handle->col,0,handle->col*sizeof(int));
    handle->row+=1;
}

void mCSVRowDelete(MFile *csv,intptr_t row)
{
    struct HandleCSV *handle = CSVHandle(csv);
    char *p;
    int row_idx;

    if(row>handle->row)
    {
        char *name = (char *)row;
        p=mMapRead(handle->row_map[handle->key_idx],name,DFLT,&row_idx,NULL);
        if(p==NULL) return;
    }
    else row_idx=row;
    int idx=row_idx*handle->col;

    for(int i=0;i<handle->col;i++)
    {
        if(handle->row_map[i]==NULL) continue;
        int n = handle->array->dataS32[idx+i];
        if(n>=0) p=(char *)(handle->file)+n;
        else p=(char *)(handle->list->data[-2-idx])+1;
        mMapNodeDelete(handle->row_map[i],p,DFLT);
    }

    int *pidx = handle->array->dataS32;
    memmove(pidx+row*handle->col,pidx+(row+1)*handle->col,(handle->row-1-row)*handle->col*sizeof(int));
    handle->row-=1;
}

char *m_CSVWrite(MFile *csv,intptr_t row,intptr_t col,const char *format,...)
{
    struct HandleCSV *handle = CSVHandle(csv);
    int row_idx,col_idx;
    char *p;

    if(row<0) {row_idx=handle->row;mCSVRowAppend(csv,row_idx);}
    else if(row>handle->row)
    {
        char *name = (char *)row;
        p=mMapRead(handle->row_map[handle->key_idx],name,DFLT,&row_idx,NULL);
        if(p==NULL) return NULL;
    }
    else row_idx=row;

    if(col<0) {col_idx=handle->col;mCSVColAppend(csv,col_idx);}
    else if(col>handle->col)
    {
        char *name = (char *)col;
        p=mornMapRead(handle->col_map,name,DFLT,&col_idx,NULL);
        if(p==NULL) return NULL;
    }
    else col_idx=col;

    char data[1024];
    va_list inipara;
    va_start(inipara,format);
    vsprintf(data+1,format,inipara);
    va_end(inipara);

    int size = strlen(data+1);
    int flag0=0;int flag1=0;
    for(int i=0;i<=size;i++)
    {
        if(data[i]==',') flag0 =1;
        if(data[i]=='"') flag1+=1;
    }
    if(flag1) data[0]=flag1+2;
    else      data[0]=flag0;

    int idx=row_idx*handle->col+col_idx;
    if(handle->row_map[col_idx]!=NULL)
    {
        int k = handle->array->dataS32[idx];
        if(k>=0) p=(char *)(handle->file)+k;
        else p=(char *)(handle->list->data[-2-idx])+1;
        mMapNodeDelete(handle->row_map[col_idx],p,DFLT);
        mMapWrite(handle->row_map[col_idx],data,DFLT,&row_idx,sizeof(int));
    }

    int n=handle->list->num;
    handle->array->dataS32[idx] = -2-n;
    return (char *)(mListWrite(handle->list,n,data,size+1))+1;
}

// void mCSVSave(MFile *csv)
// {
//     struct HandleCSV *handle = CSVHandle(csv);
//     int *array = handle->array->dataS32;
//
//     char buff[65536];
//
//     for(int j=0;j<handle->row;j++)
//     {
//         for(int i=0;i<handle->col;i++)
//         {
//             int idx = array[j*handle->col+i];
//             if(idx>=0) p=(char *)(handle->file)+idx;
//             else p=(char *)(handle->list->data[-2-idx])+1;
//         }
//     }
// }







