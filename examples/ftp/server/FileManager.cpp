//
// Created by wxl on 2020/11/30.
//

#include "FileManager.h"
#include "base/Logging.h"
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

using namespace maya;
using namespace maya::net;

FileManager::FileManager()
{
}

FileManager::~FileManager()
{
}

bool FileManager::init(const char *basepath)
{
    m_basePath=basepath;
    DIR *dp=opendir(basepath);
    if(dp==NULL)
    {
        int savaError=errno;
        LOG_ERROR<<"open base dir error, error: "<<savaError<<" "<<strerror_tl(savaError);
        if(mkdir(basepath,S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH))
            return true;
        savaError=errno;
        LOG_ERROR<<"mkdir base dir error, errno: "<<savaError<<" "<<strerror_tl(savaError);
    }

    struct dirent* dirp;
    while ((dirp=readdir(dp))!=NULL)
    {
        if((strcmp(dirp->d_name,".")==0)||(strcmp(dirp->d_name,"..")==0))
            continue;
        m_listFiles.emplace_back(std::make_pair(dirp->d_name,""));
        LOG_INFO<<"filename: "<<dirp->d_name;
    }

    closedir(dp);
    return true;
}

bool FileManager::isFileExist(const char *filename)
{
    std::lock_guard<std::mutex> lock(m_mtFile);

    //先查看缓存
    for(const auto& iter:m_listFiles)
    {
        if(iter.first==filename)
            return true;
    }

    //再查看系统目录
    std::string filepath=m_basePath;
    filepath+=filename;
    FILE * fp=fopen(filepath.c_str(),"r");
    if(fp!=NULL)
    {
        fclose(fp);
        m_listFiles.emplace_back(std::make_pair(filename,""));
        return true;
    }
    return false;
}

void FileManager::addFile(const char *filename)
{
    std::lock_guard<std::mutex> lock(m_mtFile);
    m_listFiles.emplace_back(std::make_pair(filename,""));
}
