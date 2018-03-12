#include "server.h"

#include <cstdlib> // malloc, free

#include "request.h"
#include "response.h"

namespace {
    constexpr const char * Host = "0.0.0.0";
    constexpr int Backlog = 128;

    struct Client {
        uv_tcp_t tcp;
        http::Request req;
        http::Response res;

        Client(uv_loop_t * loop) {
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

    Server::~Server() {

    }

    void Server::listen(uint16_t port) {
        if (uv_tcp_init(loop, &tcp)) {
            // Error
            return;
        }

        struct sockaddr_in address;
        if (uv_ip4_addr(Host, port, &address)) {
            // Error
            return;
        }

        if (uv_tcp_bind(&tcp, (struct sockaddr *) &address, 0)) {
            // Error
            return;
        }

        if (uv_listen((uv_stream_t *) &tcp, Backlog, onConnection)) {
            // Error
            return;
        }

        if (uv_run(loop, UV_RUN_DEFAULT)) {
            // Error
            return;
        }
    }


    void Server::onConnection(uv_stream_t * server, int status) {
        if (status) {
            // Error
            return;
        }

        Client * client = new Client(server->loop);
        uv_stream_t * stream = (uv_stream_t *) &client->tcp;

        if (uv_accept(server, stream)) {
            uv_shutdown_t * shutdownReq = (uv_shutdown_t *) malloc(sizeof(uv_shutdown_t));
            if (uv_shutdown(shutdownReq, stream, onShutdown)) {
                // Error
                return;
            }
        }

        if (uv_read_start(stream, onAllocBuffer, onRead)) {
            // Error
            return;
        }
    }

    void Server::onShutdown(uv_shutdown_t * shutdownReq, int status) {
        uv_close((uv_handle_t *) shutdownReq->handle, onClose);
        free(shutdownReq);
    }

    void Server::onClose(uv_handle_t * handle) {
        Client * client = (Client *) handle->data;
        delete client;
    }

    void Server::onAllocBuffer(uv_handle_t * handle, size_t suggestedSize, uv_buf_t * buffer) {
        buffer->base = (char *) malloc(suggestedSize);
        buffer->len = suggestedSize;
    }

    void Server::onRead(uv_stream_t * handle, ssize_t nread, const uv_buf_t * buffer) {
        if (nread >= 0) {
            Client * client = (Client *) handle->data;
            if (client->req.parse(buffer, nread)) {
                httpRequestHandler(&client->req, &client->res);
            }
        } else {
            if (nread == UV_EOF) {
                // do nothing
            } else {
                // LOGF("read: %s\n", uv_strerror(nread));
            }

            uv_shutdown_t * shutdown_req = (uv_shutdown_t *) malloc(sizeof(uv_shutdown_t));
            if (uv_shutdown(shutdown_req, handle, onShutdown)) {
                // Error
            }
        }

        // Deallocate the buffer memory
        free(buffer->base);
    }

    void Server::onWrite(uv_write_t * writeReq, int status) {
        uv_close((uv_handle_t *) writeReq->handle, onClose);
        free(writeReq);
    }
}
