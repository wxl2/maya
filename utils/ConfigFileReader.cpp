//
// Created by wxl on 2020/11/29.
//

#include "ConfigFileReader.h"
#include <stdio.h>
#include <string.h>

ConfigFileReader::ConfigFileReader(const std::string filename)
:configFile_(filename)
{
    loadFile();
}

ConfigFileReader::~ConfigFileReader()
{
}

std::string ConfigFileReader::getConfigValue(const std::string &key)
{
    if(!loadOk_)
        return "";
    std::map<std::string,std::string>::iterator iter=configMap_.find(key);
    if(iter!=configMap_.end())
        return iter->second;
    return "";
}

int ConfigFileReader::setConfigValue(const std::string &key, const std::string &value)
{
    if(!loadOk_)
        return -1;
    std::map<std::string,std::string>::iterator iter=configMap_.find(key);
    if(iter!=configMap_.end())
    {
        iter->second=value;
    }
    else
    {
        configMap_.insert(std::make_pair(key,value));
    }
    return writeFile();
}

void ConfigFileReader::loadFile()
{
    FILE* fp=fopen(configFile_.c_str(),"r");
    if(!fp)
        return;
    char buf[256];
    for(;;)
    {
        char *p = fgets(buf,256,fp);
        if(!p)
            break;
        size_t len=strlen(buf);
        if(buf[len-1]=='\n')
            buf[len-1]=0;

        //skip the comment
        char* ch=strchr(buf,'#');
        if(ch)
            continue;

        //skip the empty line
        if(strlen(buf)==0)
            continue;
        parseLine(buf);
    }
    fclose(fp);
    loadOk_= true;
}

int ConfigFileReader::writeFile()
{
    FILE *fp = fopen(configFile_.c_str(),"w");

    if(fp==NULL)
        return -1;

    char szPaire[128];
    std::map<std::string,std::string>::iterator it=configMap_.begin();
    for(it;it!=configMap_.end();++it)
    {
        memset(szPaire,0,128);
        snprintf(szPaire,128,"%s=%s\n",it->first.c_str(),it->second.c_str());
        size_t ret=fwrite(szPaire,strlen(szPaire),1,fp);
        if(ret!=1)
        {
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return 0;
}

void ConfigFileReader::parseLine(char *line)
{
    char* p=strchr(line,'=');
    if(p==NULL)
        return;
    *p=0;
    char* key=trimSpace(line);
    char* value=trimSpace(p+1);
    if(key&&value)
    {
        configMap_.insert(std::make_pair(key,value));
    }
}

char *ConfigFileReader::trimSpace(char *name)
{
    char* startPos=name;
    while((*startPos==' ')||(*startPos=='\t')||(*startPos=='\r'))
        startPos++;

    if(strlen(startPos)==0)
        return NULL;

    char* endPos=name+strlen(name)-1;
    while ((*endPos==' ')||(*endPos=='\t')||(*endPos=='\r'))
    {
        *endPos=0;
        endPos++;
    }
    int len=(int)(endPos-startPos)+1;
    if(len<=0)
        return NULL;
    return startPos;
}

