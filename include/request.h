#pragma once

#include <uv.h>

#include "./method.h"
#include "./connection.h"

namespace http {
    class Request : public Connection {
        Method method;
        icu::UnicodeString url;
        icu::UnicodeString body;

    public:
        Request() = default;

        Method getMethod() const { return method; }
        void setMethod(Method method) { this->method = method; }

        const icu::UnicodeString & getUrl() const { return url; }
        void setUrl(const icu::UnicodeString & url) { this->url = url; }

        const icu::UnicodeString & getBody() const { return body; }
        void setBody(const icu::UnicodeString & body) { this->body = body; }

        bool parse(const uv_buf_t * buffer, ssize_t nread);
    };
}
