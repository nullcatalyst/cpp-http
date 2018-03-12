#pragma once

#include <functional> // std::hash
#include <unicode/unistr.h> // icu::UnicodeString

// Add the ability to store a `icu::UnicodeString` in an `std::unordered_map`
namespace std {
    template<>
    class hash<icu::UnicodeString> {
    public:
        size_t operator()(const icu::UnicodeString & s) const {
            return (size_t) s.hashCode();
        }
    };
}

namespace http {
    inline icu::UnicodeString convertRawToUnicode(const char * at, size_t len) {
        return icu::UnicodeString::fromUTF8(icu::StringPiece(at, len));
    }

    inline std::string convertUnicodeToString(const icu::UnicodeString & u) {
        std::string s;
        u.toUTF8String(s);
        return s;
    } 
}
