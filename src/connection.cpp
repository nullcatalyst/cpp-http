#include "connection.h"

namespace http {
    std::string Connection::getHeader(const std::string & key) {
        auto it = headers.find(key);
        if (it != headers.end()) {
            return it->second;
        } else {
            return std::string();
        }
    }

    void Connection::addHeader(const std::string & key, const std::string & value) {
        headers.emplace(key, value);
    }

    void Connection::removeHeader(const std::string & key) {
        auto it = headers.find(key);
        if (it != headers.end()) {
            headers.erase(it);
        }
    }
}
