//
// Created by wxl on 2020/11/28.
//

#include "json/json.h"
#include <stdio.h>
#include <string>

/**
 * {
    "encoding" : "UTF-8",
    "plug-ins" : [
        "python",
        "c++",
        "ruby"
        ],
    "indent" : { "length" : 3, "use_space": true }
}
 * @return
 */

using namespace std;
string ConstructJsonString()
{
    Json::Value rootValue = Json::objectValue;
    rootValue["encoding"] = "UTF-8";
    rootValue["plug-ins"] = Json::arrayValue;
    rootValue["plug-ins"].append("python");
    rootValue["plug-ins"].append("c++");
    rootValue["plug-ins"].append("ruby");
    rootValue["indent"] = Json::objectValue;
    rootValue["indent"]["length"] = 3;
    rootValue["indent"]["use_space"] = true;

    return Json::FastWriter().write(rootValue);
}
void ParseJsonString(const string& document)
{
    Json::Reader reader;
    Json::Value rootValue;
    if (!reader.parse(document, rootValue))
    {
        return;
    }

    if (!rootValue.isObject())
    {
        return;
    }

    if (rootValue.isMember("encoding") && rootValue["encoding"].isString())
    {
        printf("encoding is %s \n", rootValue["encoding"].asString().c_str());
    }

    if (rootValue.isMember("plug-ins") && rootValue["plug-ins"].isArray())
    {
        for (Json::ArrayIndex i = 0; i < rootValue["plug-ins"].size(); ++i)
        {
            if (rootValue["plug-ins"][i].isString())
            {
                printf("plug-ins %d : %s \n", i, rootValue["plug-ins"][i].asString().c_str());
            }
        }
    }if (rootValue.isMember("indent") && rootValue["indent"].isObject())
    {
        if (rootValue["indent"].isMember("length") && rootValue["indent"]["length"].isInt())
        {
            printf("indent length is %d \n", rootValue["indent"]["length"].asInt());
        }

        if (rootValue["indent"].isMember("use_space") && rootValue["indent"]["use_space"].isBool())
        {
            printf("indent use_space is %s \n", rootValue["indent"]["use_space"].asBool() ? "true":"false");
        }
    }
}
int main()
{
    string document = ConstructJsonString();
    printf(document.c_str());
    printf("\n");
    ParseJsonString(document);
    return 0;
}
