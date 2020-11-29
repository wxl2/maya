//
// Created by wxl on 2020/11/29.
//

#ifndef MAYA_CONFIGFILEREADER_H
#define MAYA_CONFIGFILEREADER_H

#include "base/nocopyable.h"
#include <map>
#include <string>

/**
 * 配置文件读取工具类
 * 文件内容格式为 key=value
 */
class ConfigFileReader
{
public:
    explicit ConfigFileReader(const std::string filename);
    ~ConfigFileReader();

    std::string getConfigValue(const std::string& key);
    int setConfigValue(const std::string& key,const std::string& value);

private:
    void loadFile();
    int writeFile();
    void parseLine(char* line);
    char* trimSpace(char* name);

    std::map<std::string,std::string>   configMap_;
    bool                                loadOk_;
    std::string                         configFile_;
};


#endif //MAYA_CONFIGFILEREADER_H
