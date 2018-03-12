#pragma once

#include <string>
#include <unordered_map>

namespace http {
    class Connection {
    protected:
        std::unordered_map<std::string, std::string> headers;

        Connection() = default;

    public:
        std::string getHeader(const std::string & key);
        void addHeader(const std::string & key, const std::string & value);
        void removeHeader(const std::string & key);
    };
}
