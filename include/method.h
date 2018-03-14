#pragma once

#include <cstdint> // uint32_t

namespace http {
#define HTTP_METHOD_MAP(XX)         \
    XX(0,   Unknown,    UNKNOWN)    \
    XX(1,   Delete,     DELETE)     \
    XX(2,   Get,        GET)        \
    XX(3,   Head,       HEAD)       \
    XX(4,   Post,       POST)       \
    XX(5,   Put,        PUT)

    enum class Method : uint32_t {
#define XX(num, name, string) name = num,
        HTTP_METHOD_MAP(XX)
#undef XX
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
