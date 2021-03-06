#pragma once

#include <unordered_map>

#include "./unicode-string.h"

namespace http {
    class Message {
    protected:
        std::unordered_map<icu::UnicodeString, icu::UnicodeString> headers;

        Message() = default;

    public:
        icu::UnicodeString getHeader(const icu::UnicodeString & key);
        void addHeader(const icu::UnicodeString & key, const icu::UnicodeString & value);
        void removeHeader(const icu::UnicodeString & key);
    };
}
