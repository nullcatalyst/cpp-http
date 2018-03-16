#pragma once

#include <uv.h>

#include "./status.h"
#include "./message.h"

namespace http {
    class Response : public Message {
        Status status;
        icu::UnicodeString body;

    public:
        Response() : status(Status::OK) {}

        Status getStatus() const { return status; }
        void setStatus(Status status) { this->status = status; }

        const icu::UnicodeString & getBody() const { return body; }
        void setBody(const icu::UnicodeString & body) { this->body = body; }
        void write(const icu::UnicodeString & content) { this->body += content; }
        void clear() { this->body = icu::UnicodeString(); }

        /**
         * Parse the buffer representing an HTTP response.
         */
        bool parse(const uv_buf_t * buffer, ssize_t nread);

        /**
         * The caller is responsible for freeing the returned buffer.
         */
        uv_buf_t end() const;
    };
}
