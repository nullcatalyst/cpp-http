#pragma once

#include <uv.h>

#include "./method.h"
#include "./connection.h"

namespace http {
    class Request : public Connection {
        Method method;
        std::string url;
        std::string body;

    public:
        Request() = default;

        Method getMethod() const { return method; }
        void setMethod(Method method) { this->method = method; }

        const std::string & getUrl() const { return url; }
        void setUrl(const std::string & url) { this->url = url; }

        const std::string & getBody() const { return body; }
        void setBody(const std::string & body) { this->body = body; }

        bool parse(const uv_buf_t * buffer, ssize_t nread);
    };
}
