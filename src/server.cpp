#include "server.h"

#include <cstdlib> // malloc, free

#include "request.h"
#include "response.h"

namespace {
    struct DNSLookup {
        uv_getaddrinfo_t getaddrinfo;
        http::Server * server;
        http::Address address;
        http::Server::AddressHandler callback;

        DNSLookup(http::Server * server, const http::Server::AddressHandler & callback) : server(server), callback(callback) {
            getaddrinfo.data = this;
        }
    };

    struct Connection {
        uv_tcp_t tcp;

        union {
            uv_write_t write;
            uv_connect_t connect;
            uv_shutdown_t shutdown;
        };

        http::Server * server;
        http::Request req;
        http::Response res;
        http::Server::HttpResponseHandler callback;

        Connection(uv_loop_t * loop, http::Server * server) : write{0}, server(server) {
            if (uv_tcp_init(loop, &tcp)) {
                // Error
            }

            tcp.data = this;
        }
    };
}



namespace http {
    Server::Server() : loop(uv_default_loop()) {}

    Server::Server(uv_loop_t * loop) : loop(loop) {}

    Server::~Server() {}

    void Server::makeDNSLookup(const std::string & name, const AddressHandler & callback) {
        struct addrinfo hints;
        hints.ai_family = PF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = 0;

        DNSLookup * dns = new DNSLookup(this, callback);
        if (uv_getaddrinfo(loop, &dns->getaddrinfo, Server::onAddress, name.c_str(), nullptr, &hints)) {
            // Error
        }
    }

    void Server::makeRequest(const Address & address, const Request & req, const HttpResponseHandler & callback) {
        Connection * conn = new Connection(loop, this);
        conn->req = req;
        conn->callback = callback;

        conn->connect.data = conn;

        if (uv_tcp_connect(&conn->connect, &conn->tcp, (const struct sockaddr *) &address.sock, Server::onConnect)) {
            // Error
        }
    }

    bool Server::listen(uint16_t port) {
        if (uv_tcp_init(loop, &tcp)) {
            // Error
            return false;
        }
        tcp.data = this;

        struct sockaddr_in address;
        if (uv_ip4_addr("0.0.0.0", port, &address)) {
            // Error
            return false;
        }
        this->port = address.sin_port;

        if (uv_tcp_bind(&tcp, (struct sockaddr *) &address, 0)) {
            // Error
            return false;
        }

        if (uv_listen((uv_stream_t *) &tcp, 128, Server::onConnection)) {
            // Error
            return false;
        }

        return true;
    }

    void Server::run() {
        if (uv_run(loop, UV_RUN_DEFAULT)) {
            // Error
        }

        running = false;
        cv.notify_all();
    }

    void Server::close() {
        if (running) {
            uv_stop(loop);

            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock, [this] { return !running; });
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

        dns->callback(*dns->server, dns->address);
        delete dns;
    }

    void Server::allocBuffer(uv_handle_t * handle, size_t suggestedSize, uv_buf_t * buffer) {
        buffer->base = (char *) malloc(suggestedSize);
        buffer->len = suggestedSize;
    }

    void Server::close(uv_handle_t * handle) {
        Connection * conn = (Connection *) handle->data;
        delete conn;
    }

    void Server::shutdown(uv_shutdown_t * shutdown, int status) {
        uv_close((uv_handle_t *) shutdown->handle, Server::close);
    }

    void Server::onConnection(uv_stream_t * stream, int status) {
        if (status) {
            // Error
            return;
        }

        Connection * conn = new Connection(stream->loop, (Server *) stream->data);

        if (uv_accept(stream, (uv_stream_t *) &conn->tcp)) {
            if (uv_shutdown(&conn->shutdown, (uv_stream_t *) &conn->tcp, Server::shutdown)) {
                // Error
                return;
            }
        }

        if (uv_read_start((uv_stream_t *) &conn->tcp, Server::allocBuffer, Server::onRequestRead)) {
            // Error
            return;
        }
    }

    void Server::onRequestRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buffer) {
        Connection * conn = (Connection *) stream->data;

        if (nread >= 0) {
            if (conn->req.parse(buffer, nread)) {
                conn->server->httpRequestHandler(*conn->server, conn->req, conn->res);

                uv_buf_t resBuffer = conn->res.end();
                conn->write.data = resBuffer.base;
                uv_write(&conn->write, stream, &resBuffer, 1, Server::onResponseWrite);
            }
        } else {
            if (nread == UV_EOF) {
                // do nothing
            } else {
                // printf("read: %s\n", uv_strerror(nread));
            }

            if (uv_shutdown(&conn->shutdown, stream, Server::shutdown)) {
                // Error
            }
        }

        // Deallocate the buffer memory
        free(buffer->base);
    }

    void Server::onResponseWrite(uv_write_t * write, int status) {
        uv_close((uv_handle_t *) write->handle, Server::close);
        free(write->data);
    }

    void Server::onConnect(uv_connect_t * connect, int status) {
        if (status) {
            // Error
            return;
        }

        Connection * conn = (Connection *) connect->data;
        if (uv_read_start((uv_stream_t *) &conn->tcp, Server::allocBuffer, Server::onResponseRead)) {
            // Error
            return;
        }

        const uv_buf_t buffer = conn->req.end();
        conn->write.data = buffer.base;
        if (uv_write(&conn->write, (uv_stream_t *) &conn->tcp, &buffer, 1, Server::onRequestWrite)) {
            // Error
            return;
        }
    }

    void Server::onRequestWrite(uv_write_t * write, int status) {
        free(write->data);
    }

    void Server::onResponseRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buffer) {
        Connection * conn = (Connection *) stream->data;

        if (nread >= 0) {
            if (conn->res.parse(buffer, nread)) {
                conn->callback(*conn->server, conn->req, conn->res);
            }
        } else {
            if (nread == UV_EOF) {
                // do nothing
            } else {
                // printf("read: %s\n", uv_strerror(nread));
            }
        }

        if (uv_shutdown(&conn->shutdown, stream, Server::shutdown)) {
            // Error
        }

        // Deallocate the buffer memory
        free(buffer->base);
    }
}
