#pragma once

#include <sstream>
#include <uv.h>

#include "./method.h"
#include "./address.h"
#include "./message.h"

namespace http {
    class Request : public Message {
        Method method;
        Address address;
        icu::UnicodeString url;
        std::stringstream body;

        friend class Server;

    public:
        Request();

        Method getMethod() const { return method; }
        void setMethod(Method method) { this->method = method; }

        Address getAddress() const { return address; }
        void setAddress(const Address & address) { this->address = address; }

        const icu::UnicodeString & getUrl() const { return url; }
        void setUrl(const icu::UnicodeString & url) { this->url = url; }

        std::stringstream & getBody() { return body; }
        void clear();

        /**
         * Parse the buffer representing an HTTP request.
         */
        bool parse(const uv_buf_t * buffer, ssize_t nread);

        /**
         * The caller is responsible for freeing the returned buffer.
         */
        uv_buf_t end();
    };
}
