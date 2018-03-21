#include "address.h"

#include <cstring> // memcmp

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

    bool Address::operator == (const Address & other) const {
        if (sock.ss_family == other.sock.ss_family) {
            if (isIPv4()) {
                return memcmp(&((struct sockaddr_in *) &sock)->sin_addr, &((struct sockaddr_in *) &other.sock)->sin_addr, sizeof(struct in_addr)) == 0;
            } else if (isIPv6()) {
                return memcmp(&((struct sockaddr_in6 *) &sock)->sin6_addr, &((struct sockaddr_in6 *) &other.sock)->sin6_addr, sizeof(struct in6_addr)) == 0;
            }
        }

        return false;
    }

    bool Address::operator != (const Address & other) const {
        if (sock.ss_family == other.sock.ss_family) {
            if (isIPv4()) {
                return memcmp(&((struct sockaddr_in *) &sock)->sin_addr, &((struct sockaddr_in *) &other.sock)->sin_addr, sizeof(struct in_addr)) != 0;
            } else if (isIPv6()) {
                return memcmp(&((struct sockaddr_in6 *) &sock)->sin6_addr, &((struct sockaddr_in6 *) &other.sock)->sin6_addr, sizeof(struct in6_addr)) != 0;
            }
        }

        return true;
    }
}
