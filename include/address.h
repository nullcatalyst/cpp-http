#pragma once

#include <cstdint> // uint16_t
#include <string>
#include <uv.h>

namespace http {
    class Address {
        struct sockaddr_storage sock;

        friend class Server;

    public:
        static Address fromIPv4(const std::string & ip4, uint16_t port);
        static Address fromIPv6(const std::string & ip6, uint16_t port);

        uint16_t getPort() const { return ntohs(((const struct sockaddr_in *) &sock)->sin_port); }
        void setPort(uint16_t port) { ((struct sockaddr_in *) &sock)->sin_port = ntohs(port); }

        bool isIPv4() const { return sock.ss_family == AF_INET; }
        bool isIPv6() const { return sock.ss_family == AF_INET6; }

        std::string toString() const;
    };
}
