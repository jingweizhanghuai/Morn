/*
Copyright (C) 2019-2021 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this json except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/
//编译： g++ -O2 test_JSON_file2.cpp -o test_JSON_file2.exe -lcjson -ljsoncpp -lmorn

#include "morn_util.h"

#include <fstream>
#include "json/json.h"

#include "rapidjson/document.h"

#include "cJSON.h"

#include "nlohmann/json.hpp"



void test1()
{
    printf("\n\n");
    
    mTimerBegin("cjson");
    FILE *f1 = fopen("./citm_catalog.json","rb");
    int size1 = fsize(f1);
    char *cjson=(char *)malloc(size1);
    fread(cjson,size1,1,f1);
    fclose(f1);
    cJSON * cjson_root = cJSON_Parse(cjson); 
    free(cjson);
    mTimerEnd("cjson");

    std::ifstream ifs;
    Json::CharReaderBuilder reader;
    Json::Value jsoncpp_root;
    JSONCPP_STRING errs;
    mTimerBegin("jsoncpp");
    ifs.open("./citm_catalog.json");
    Json::parseFromStream(reader, ifs, &jsoncpp_root, &errs);
    mTimerEnd("jsoncpp");

    nlohmann::json nlohmannjson;
    std::ifstream jfile("./citm_catalog.json");
    mTimerBegin("nlohmann");
    jfile >> nlohmannjson;
    mTimerEnd("nlohmann");

    rapidjson::Document document;
    mTimerBegin("rapidjson");
    FILE *f2 = fopen("./citm_catalog.json","rb");
    int size2 = fsize(f2);
    char *rapidjson=(char *)malloc(size2);
    fread(rapidjson,size2,1,f2);
    fclose(f2);
    document.Parse(rapidjson);
    free(rapidjson);
    mTimerEnd("rapidjson");
    
    MArray *json=mArrayCreate();
    mTimerBegin("Morn");
    mJSONLoad(json,"./citm_catalog.json");
    mTimerEnd("Morn");
    mArrayRelease(json);
}

int cjson_test()
{
    FILE *f = fopen("./canada.json","rb");
    int size = fsize(f);
    char *json=(char *)malloc(size);
    fread(json,size,1,f);
    fclose(f);
    cJSON * root = cJSON_Parse(json); 
    free(json);

    int n=0;
    cJSON *features_array = cJSON_GetObjectItem(root,"features");
    cJSON *features       = cJSON_GetArrayItem(features_array,0);
    cJSON *geometry       = cJSON_GetObjectItem(features,"geometry");
    cJSON *coordinates0   = cJSON_GetObjectItem(geometry,"coordinates");
    for(int i=0;i<cJSON_GetArraySize(coordinates0);i++)
    {
        cJSON *coordinates1 = cJSON_GetArrayItem(coordinates0,i);
        for(int j=0;j<cJSON_GetArraySize(coordinates1);j++)
        {
            cJSON *coordinates2 = cJSON_GetArrayItem(coordinates1,j);
            double x=cJSON_GetArrayItem(coordinates2,0)->valuedouble;
            cJSON *y=cJSON_GetArrayItem(coordinates2,1);
            n++;
            // printf("x=%f,y=%f\n",x,y);
        }
    }
    
    cJSON_Delete(root);
    return n;
}

int jsoncpp_test()
{
    std::ifstream ifs;
    ifs.open("./canada.json");

    Json::CharReaderBuilder reader;
    Json::Value root;
    JSONCPP_STRING errs;
    Json::parseFromStream(reader, ifs, &root, &errs);

    int n=0;
    Json::Value coordinates0 = root["features"][0]["geometry"]["coordinates"];
    for(int j=0;j<coordinates0.size();j++)
    {
        Json::Value coordinates1 = coordinates0[j];
        for(int i=0;i<coordinates1.size();i++)
        {
            Json::Value coordinates2 = coordinates1[i];
            double x=coordinates2[0].asDouble();
            double y=coordinates2[1].asDouble();
            n++;
            // printf("x=%f,y=%f\n",x,y);
        }
    }
    return n;
}

int nlohmann_test()
{
    nlohmann::json nlohmannjson;
    std::ifstream jfile("./canada.json");
    jfile >> nlohmannjson;

    int n=0;
    nlohmann::json coordinates0 = nlohmannjson["features"][0]["geometry"]["coordinates"];
    for(int j=0;j<coordinates0.size();j++)
    {
        nlohmann::json coordinates1=coordinates0[j];
        for(int i=0;i<coordinates1.size();i++)
        {
            nlohmann::json coordinates2=coordinates1[i];
            double x=coordinates2[0];
            double y=coordinates2[1];
            n++;
            // printf("x=%f,y=%f\n",x,y);
        }
    }
    return n;
}

int rapidjson_test()
{
    rapidjson::Document doc;
    
    FILE *f = fopen("./canada.json","rb");
    int size = fsize(f);
    char *json=(char *)malloc(size);
    fread(json,size,1,f);
    fclose(f);
    doc.Parse(json);
    free(json);
    
    int n=0;
    const rapidjson::Value& coordinates0= doc["features"][0]["geometry"]["coordinates"];
    for(int i=0;i<coordinates0.Size();i++)
    {
        const rapidjson::Value& coordinates1= coordinates0[i];
        for(int j=0;j<coordinates1.Size();j++)
        {
            const rapidjson::Value& coordinates2= coordinates1[j];
            double x= coordinates2[0].GetDouble();
            double y= coordinates2[1].GetDouble();
            n++;
            // printf("x=%f,y=%f\n",x,y);
        }
    }
    return n;
}

int Morn_test()
{
    MArray *json=mArrayCreate();
    mJSONLoad(json,"./canada.json");
    
    MArray *coordinates0 = mArrayCreate();
    MArray *coordinates1 = mArrayCreate();
    MArray *coordinates2 = mArrayCreate();

    int n=0;
    mJSONArray(coordinates0,mJSONRead(json,"features[0].geometry.coordinates"));
    struct JSONNode *node0=(struct JSONNode *)coordinates0->data;
    for(int j=0;j<coordinates0->num;j++)
    {
        mJSONArray(coordinates1,node0+j);
        struct JSONNode *node1=(struct JSONNode *)coordinates1->data;
        for(int i=0;i<coordinates1->num;i++)
        {
            mJSONArray(coordinates2,node1+i);
            struct JSONNode *node2=(struct JSONNode *)coordinates2->data;
            double x=node2[0].value_f;
            double y=node2[1].value_f;
            n++;
            // printf("x=%f,y=%f\n",x,y);
        }
    }
    
    mArrayRelease(coordinates0);
    mArrayRelease(coordinates1);
    mArrayRelease(coordinates2);
    mArrayRelease(json);
    return n;
}

void test2()
{
    printf("\n\n");
    int n;
    mTimerBegin("cjson");
    n=cjson_test();
    mTimerEnd("cjson");
    printf("get %d coordinates\n\n",n);

    mTimerBegin("jsoncpp");
    n=jsoncpp_test();
    mTimerEnd("jsoncpp");
    printf("get %d coordinates\n\n",n);

    mTimerBegin("nlohmann");
    n=nlohmann_test();
    mTimerEnd("nlohmann");
    printf("get %d coordinates\n\n",n);
    
    mTimerBegin("rapidjson");
    n=rapidjson_test();
    mTimerEnd("rapidjson");
    printf("get %d coordinates\n\n",n);

    mTimerBegin("Morn");
    n=Morn_test();
    mTimerEnd("Morn");
    printf("get %d coordinates\n\n",n);
}

int main()
{
    test1();
    test2();
    return 0;
}