#include "connection.h"

namespace http {
    icu::UnicodeString Connection::getHeader(const icu::UnicodeString & key) {
        auto it = headers.find(key);
        if (it != headers.end()) {
            return it->second;
        } else {
            return icu::UnicodeString();
        }
    }

    void Connection::addHeader(const icu::UnicodeString & key, const icu::UnicodeString & value) {
        headers.emplace(key, value);
    }

    void Connection::removeHeader(const icu::UnicodeString & key) {
        auto it = headers.find(key);
        if (it != headers.end()) {
            headers.erase(it);
        }
    }
}
