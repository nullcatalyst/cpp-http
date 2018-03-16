#pragma once

#include <cstdint> // uint32_t

namespace http {
#define HTTP_METHOD_MAP(XX)         \
    XX(-1,  Unknown,    UNKNOWN)    \
    XX(0,   Get,        GET)        \
    XX(1,   Post,       POST)       \
    XX(2,   Delete,     DELETE)     \
    XX(3,   Put,        PUT)        \
    XX(4,   Head,       HEAD)

    enum class Method {
#define XX(num, name, string) name = num,
        HTTP_METHOD_MAP(XX)
#undef XX
        Count,
    };

    inline const char * getMethodString(Method method) {
        switch (method) {
#define XX(num, name, string) case Method:: name : return #string ;
            HTTP_METHOD_MAP(XX)
#undef XX
            default: return "";
        }
    }

#undef HTTP_METHOD_MAP
}
