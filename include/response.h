#pragma once

#include <sstream>
#include <uv.h>

#include "./status.h"
#include "./message.h"

namespace http {
    class Response : public Message {
        Status status;
        std::stringstream body;

        friend class Server;

    public:
        Response() : status(Status::OK) {}

        Status getStatus() const { return status; }
        void setStatus(Status status) { this->status = status; }

        std::stringstream & getBody() { return body; }
        void clear();

        /**
         * Parse the buffer representing an HTTP response.
         */
        bool parse(const uv_buf_t * buffer, ssize_t nread);

        /**
         * The caller is responsible for freeing the returned buffer.
         */
        uv_buf_t end();
    };
}
