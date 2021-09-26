/*
Copyright (C) 2019-2022 JingWeiZhangHuai <jingweizhanghuai@163.com>
Licensed under the Apache License, Version 2.0; you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License.
*/

char *morn_version = "00.00.01";

char *help_infomation = "Options:\
\n--help              Display this information\
\n-v                  Display version information\
";

char *help_crypt = "Options:\
\n--help              Display this information\
\n-v                  Display version information\
\n-e <file>           Set input file for encrypt\
\n-d <file>           Set input file for decrypt\
\n-o <file>           Set output file\
\n-k <key>            Set encrypt/decrypt key\
";

char *help_imageformat = "Options:\
\n--help              Display this information\
\n-v                  Display version information\
\n-i <file input>     Set input file for transform\
\n-o <file output>    Set output file\
\n-di <dir input>     Set input directory\
\n-do <dir output>    Set output directory\
\n-ti <type input>    Set input image type\
\n-to <type output>   Set output image type\
";

#define ToolHelp(Name) do{\
    if(mStringArgument(argc,argv,"-help")!=NULL) {printf("%s",help_##Name);return 0;}\
    if(mStringArgument(argc,argv,"v"    )!=NULL) {printf("version %s\n",morn_version);return 0;}\
}while(0)
