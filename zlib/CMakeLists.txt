set(zlib_SRCS
        zutil.c
        uncompr.c
        trees.c
        inftrees.c
        inflate.c
        inffast.c
        infback.c
        gzwrite.c
        gzread.c
        gzlib.c
        gzclose.c
        deflate.c
        crc32.c
        compress.c
        adler32.c
        ZlibUtil.cpp
        )

add_library(zlib ${zlib_SRCS})