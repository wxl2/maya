//
// Created by wxl on 2020/10/12.
//

#ifndef MAYA_TYPES_H
#define MAYA_TYPES_H

#include <stdint.h>
#include <string.h>  // memset
#include <string>

#ifndef NDEBUG
#include <assert.h>
#endif

namespace maya{

    using std::string;

    inline void memZero(void* p, size_t n)
    {
        memset(p, 0, n);
    }

    template<typename To, typename From>
    inline To implicit_cast(From const &f)
    {
        return f;
    }

    template<typename To, typename From>
    inline To down_cast(From* f)
    {
        if (false)//永远不可能执行
        {
            implicit_cast<From*, To>(0);
        }

#if !defined(NDEBUG) && !defined(GOOGLE_PROTOBUF_NO_RTTI)
        assert(f == NULL || dynamic_cast<To>(f) != NULL);
#endif
        return static_cast<To>(f);
    }

}//namespace maya
#endif //MAYA_TYPES_H
