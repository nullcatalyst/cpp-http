#include "server.h"

#include <cstdlib> // malloc, free
#include <cstring> // memcpy, memset

#include "error.h"
#include "request.h"
#include "response.h"

namespace {
    struct DNSLookup {
        uv_getaddrinfo_t getaddrinfo;
        http::Server * server;
        http::Address address;
        http::Server::DNSLookupCallback callback;

        DNSLookup(http::Server * server, const http::Server::DNSLookupCallback & callback) : server(server), callback(callback) {
            getaddrinfo.data = this;
        }
    };
}


namespace http {
    Server::Server() {
        uv_loop_init(&loop);
    }

    Server::~Server() {
        close();
        uv_loop_close(&loop);
    }

    void Server::makeDNSLookup(const std::string & name, const DNSLookupCallback & callback) {
        struct addrinfo hints;
        hints.ai_family = PF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = 0;

        DNSLookup * dns = new DNSLookup(this, callback);
        if (int err = uv_getaddrinfo(&loop, &dns->getaddrinfo, Server::onAddress, name.c_str(), nullptr, &hints)) {
            ERROR("%s", uv_strerror(err));
        }
    }

    bool Server::listen(uint16_t port) {
        if (int err = uv_tcp_init(&loop, &tcp)) {
            ERROR("%s", uv_strerror(err));
            return false;
        }
        tcp.data = this;

        struct sockaddr_in address;
        if (int err = uv_ip4_addr("0.0.0.0", port, &address)) {
            ERROR("%s", uv_strerror(err));
            return false;
        }

        if (int err = uv_tcp_bind(&tcp, (const struct sockaddr *) &address, 0)) {
            ERROR("%s", uv_strerror(err));
            return false;
        }

        if (int err = uv_listen((uv_stream_t *) &tcp, 128, Server::in_onConnect)) {
            ERROR("%s", uv_strerror(err));
            return false;
        }

        return true;
    }

    bool Server::reuseSocket(const Server & other) {
        if (int err = uv_tcp_init(&loop, &tcp)) {
            ERROR("%s", uv_strerror(err));
            return false;
        }
        tcp.data = this;

        uv_os_sock_t sock = 0;
        if (int err = uv_fileno((const uv_handle_t *) &other.tcp, (uv_os_fd_t *) &sock)) {
            ERROR("%s", uv_strerror(err));
            return false;
        }

        if (int err = uv_tcp_open(&tcp, sock)) {
            ERROR("%s", uv_strerror(err));
            return false;
        }

        if (int err = uv_listen((uv_stream_t *) &tcp, 128, Server::in_onConnect)) {
            ERROR("%s", uv_strerror(err));
            return false;
        }

        return true;
    }

    void Server::run() {
        running = true;
        if (int err = uv_run(&loop, UV_RUN_DEFAULT)) {
            if (err != 1) {
                ERROR("%s", uv_strerror(err));
            }
        }

        running = false;
    }

    void Server::close() {
        if (running) {
            uv_async_t * async = (uv_async_t *) malloc(sizeof(uv_async_t));
            async->data = this;

            uv_async_init(&loop, async, [] (uv_async_t * async) {
                Server * server = (Server *) async->data;

                uv_read_stop((uv_stream_t *) &server->tcp);
                uv_close((uv_handle_t *) &server->tcp, nullptr);
                uv_stop(&server->loop);

                free(async);
            });

            uv_async_send(async);
        }
    }
}

namespace http {
    void Server::onAddress(uv_getaddrinfo_t * getaddrinfo, int status, struct addrinfo * addrinfo) {
        if (status) {
            // Error
            return;
        }

        DNSLookup * dns = (DNSLookup *) getaddrinfo->data;

        if (addrinfo != nullptr) {
            if (addrinfo->ai_addr->sa_family == PF_INET) {
                memcpy(&dns->address.sock, addrinfo->ai_addr, sizeof(struct sockaddr_in));
            } else if (addrinfo->ai_addr->sa_family == PF_INET6) {
                memcpy(&dns->address.sock, addrinfo->ai_addr, sizeof(struct sockaddr_in6));
            } else {
                memset(&dns->address.sock, 0, sizeof(Address));
            }
        } else {
            memset(&dns->address.sock, 0, sizeof(Address));
        }

        dns->callback(dns->address);
        delete dns;
    }

    void Server::allocBuffer(uv_handle_t * handle, size_t suggestedSize, uv_buf_t * buffer) {
        *buffer = uv_buf_init((char *) malloc(suggestedSize), suggestedSize);
    }
}
