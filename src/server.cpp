#include "server.h"

#include <cstdlib> // malloc, free

#include "request.h"
#include "response.h"

namespace {
    // struct Address : public uv_getaddrinfo_t {
    //     // http::Server::AddressHandler callback;
    // };

    // void onAddress(uv_getaddrinfo_t * _address, int status, struct addrinfo * res) {
    //     Address * address = (Address *) _address;
    //     printf("onAddress:%p\n", _address);
    // }

    struct Client {
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

        Client(uv_loop_t * loop, http::Server * server) : write{0}, server(server) {
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

    // void Server::getAddress(const std::string & name, const AddressHandler & callback) {
    //     struct addrinfo hints;
    //     hints.ai_family = PF_INET;
    //     hints.ai_socktype = SOCK_STREAM;
    //     hints.ai_protocol = IPPROTO_TCP;
    //     hints.ai_flags = 0;

    //     Address * address = new Address();
    //     // address.callback = callback;

    //     printf("getAddress:%p\n", address);
    //     if (uv_getaddrinfo(loop, address, onAddress, name.c_str(), nullptr, &hints)) {
    //         // Error
    //     }
    // }

    void Server::makeRequest(sockaddr * address, const Request & req, const HttpResponseHandler & callback) {
        Client * client = new Client(loop, this);
        client->req = req;
        client->callback = callback;

        if (uv_ip4_addr("172.217.4.206", 80, (sockaddr_in *) address)) {
            // Error
            return;
        }

        client->connect.data = client;
        if (uv_tcp_connect(&client->connect, &client->tcp, address, Server::onConnect)) {
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
    void Server::allocBuffer(uv_handle_t * handle, size_t suggestedSize, uv_buf_t * buffer) {
        buffer->base = (char *) malloc(suggestedSize);
        buffer->len = suggestedSize;
    }

    void Server::close(uv_handle_t * handle) {
        Client * client = (Client *) handle->data;
        delete client;
    }

    void Server::shutdown(uv_shutdown_t * shutdown, int status) {
        uv_close((uv_handle_t *) shutdown->handle, Server::close);
    }

    void Server::onConnection(uv_stream_t * stream, int status) {
        if (status) {
            // Error
            return;
        }

        Client * client = new Client(stream->loop, (Server *) stream->data);

        if (uv_accept(stream, (uv_stream_t *) &client->tcp)) {
            if (uv_shutdown(&client->shutdown, (uv_stream_t *) &client->tcp, Server::shutdown)) {
                // Error
                return;
            }
        }

        if (uv_read_start((uv_stream_t *) &client->tcp, Server::allocBuffer, Server::onRequestRead)) {
            // Error
            return;
        }
    }

    void Server::onRequestRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buffer) {
        Client * client = (Client *) stream->data;

        if (nread >= 0) {
            if (client->req.parse(buffer, nread)) {
                client->server->httpRequestHandler(*client->server, client->req, client->res);

                uv_buf_t resBuffer = client->res.end();
                client->write.data = resBuffer.base;
                uv_write(&client->write, stream, &resBuffer, 1, Server::onResponseWrite);
            }
        } else {
            if (nread == UV_EOF) {
                // do nothing
            } else {
                // printf("read: %s\n", uv_strerror(nread));
            }

            if (uv_shutdown(&client->shutdown, stream, Server::shutdown)) {
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

        Client * client = (Client *) connect->data;
        if (uv_read_start((uv_stream_t *) &client->tcp, Server::allocBuffer, Server::onResponseRead)) {
            // Error
            return;
        }

        const uv_buf_t buffer = client->req.end();
        client->write.data = buffer.base;
        if (uv_write(&client->write, (uv_stream_t *) &client->tcp, &buffer, 1, Server::onRequestWrite)) {
            // Error
            return;
        }
    }

    void Server::onRequestWrite(uv_write_t * write, int status) {
        free(write->data);
    }

    void Server::onResponseRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buffer) {
        Client * client = (Client *) stream->data;

        if (nread >= 0) {
            if (client->res.parse(buffer, nread)) {
                client->callback(*client->server, client->req, client->res);
            }
        } else {
            if (nread == UV_EOF) {
                // do nothing
            } else {
                // printf("read: %s\n", uv_strerror(nread));
            }
        }

        if (uv_shutdown(&client->shutdown, stream, Server::shutdown)) {
            // Error
        }

        // Deallocate the buffer memory
        free(buffer->base);
    }
}
