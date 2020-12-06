//
// Created by wxl on 2020/11/30.
//

#ifndef MAYA_FILEMANAGER_H
#define MAYA_FILEMANAGER_H

#include <list>
#include <mutex>
#include <string>
#include "base/nocopyable.h"

namespace maya{
namespace net{

    class FileManager final :nocopyable
    {
    public:
        typedef std::list<std::string> FileList;
        FileManager();
        ~FileManager();

        bool init(const char* basepath);
        bool isFileExist(const char* filename);
        void addFile(const char* filename);
        const FileList& getFileList() const;
    private:
        FileList                                        m_listFiles;
        std::mutex                                      m_mtFile;
        std::string                                     m_basePath;
    };
}//namespace net
}//namespace maya


#endif //MAYA_FILEMANAGER_H
