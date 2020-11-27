//
// Created by wxl on 2020/11/25.
//

#ifndef MAYA_UUIDGENERATOR_H
#define MAYA_UUIDGENERATOR_H

#include "base/nocopyable.h"
#include <string>

namespace maya{
namespace detail{
    class UUIDGenerator final :nocopyable
    {
    private:
        UUIDGenerator()=delete;
        ~UUIDGenerator()=delete;

    public:
        static std::string generate();
    };
}
}


#endif //MAYA_UUIDGENERATOR_H
