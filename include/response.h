#pragma once

#include <uv.h>

#include "./status.h"
#include "./connection.h"

namespace http {
    class Response : public Connection {
        Status status;
        icu::UnicodeString body;

    public:
        Response() = default;

        Status getStatus() const { return status; }
        void setStatus(Status status) { this->status = status; }

        const icu::UnicodeString & getBody() const { return body; }
        void setBody(const icu::UnicodeString & body) { this->body = body; }

        void write(const icu::UnicodeString & content) { this->body += content; }
        uv_buf_t end();
    };
}
