#pragma once

#include <uv.h>

#include "./method.h"
#include "./address.h"
#include "./message.h"

namespace http {
    class Request : public Message {
        Method method;
        Address address;
        icu::UnicodeString url;
        icu::UnicodeString body;

        friend class Server;

    public:
        Request();

        Method getMethod() const { return method; }
        void setMethod(Method method) { this->method = method; }

        Address getAddress() const { return address; }
        void setAddress(const Address & address) { this->address = address; }

        const icu::UnicodeString & getUrl() const { return url; }
        void setUrl(const icu::UnicodeString & url) { this->url = url; }

        const icu::UnicodeString & getBody() const { return body; }
        void setBody(const icu::UnicodeString & body) { this->body = body; }
        void write(const icu::UnicodeString & content) { this->body += content; }
        void clear() { this->body = icu::UnicodeString(); }

        /**
         * Parse the buffer representing an HTTP request.
         */
        bool parse(const uv_buf_t * buffer, ssize_t nread);

        /**
         * The caller is responsible for freeing the returned buffer.
         */
        uv_buf_t end() const;
    };
}
