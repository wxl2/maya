//
// Created by wxl on 2020/10/30.
//

#ifndef MAYA_ENDIAN_H
#define MAYA_ENDIAN_H
#include <endian.h>
#include <stdint.h>
namespace maya
{
namespace net
{
namespace sockets
{
    inline uint64_t hostToNetwork64(uint64_t host64)
    {
        return htobe64(host64);
    }

    inline uint32_t hostToNetwork32(uint32_t host32)
    {
        return htobe32(host32);
    }

    inline uint16_t hostToNetwoek16(uint16_t host16)
    {
        return htobe16(host16);
    }

    inline uint64_t networkToHost64(uint64_t net64)
    {
        return be64toh(net64);
    }

    inline uint32_t networkToHost32(uint32_t net32)
    {
        return be32toh(net32);
    }

    inline uint16_t networkToHost16(uint16_t net16)
    {
        return be16toh(net16);
    }
}//namespace sockets
}//namespace net
}//namespace maya
#endif //MAYA_ENDIAN_H
