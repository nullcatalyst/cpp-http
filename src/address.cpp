#include "address.h"

namespace http {
    Address Address::fromIPv4(const std::string & ip4, uint16_t port) {
        Address address;

        if (uv_ip4_addr(ip4.c_str(), port, (struct sockaddr_in *) &address.sock)) {
            // Error
        }

        return address;
    }

    Address Address::fromIPv6(const std::string & ip6, uint16_t port) {
        Address address;

        if (uv_ip6_addr(ip6.c_str(), port, (struct sockaddr_in6 *) &address.sock)) {
            // Error
        }

        return address;
    }

    std::string Address::toString() const {
        char buffer[INET6_ADDRSTRLEN];
        return inet_ntop(sock.ss_family, &((struct sockaddr_in *) &sock)->sin_addr, buffer, INET6_ADDRSTRLEN);
    }
}
