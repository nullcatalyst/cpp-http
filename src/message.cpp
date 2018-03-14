#include "message.h"

namespace http {
    icu::UnicodeString Message::getHeader(const icu::UnicodeString & key) {
        auto it = headers.find(key);
        if (it != headers.end()) {
            return it->second;
        } else {
            return icu::UnicodeString();
        }
    }

    void Message::addHeader(const icu::UnicodeString & key, const icu::UnicodeString & value) {
        headers.emplace(key, value);
    }

    void Message::removeHeader(const icu::UnicodeString & key) {
        auto it = headers.find(key);
        if (it != headers.end()) {
            headers.erase(it);
        }
    }
}
