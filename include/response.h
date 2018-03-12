#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <uv.h>

#include "./method.h"
#include "./status.h"

namespace http {
    class Response {
        Status status;
        std::stringstream body;

    public:
        Response() = default;

        Status getStatus() const { return status; }
        void setStatus(Status status) { this->status = status; }

        uv_buf_t * end();
    };
}
