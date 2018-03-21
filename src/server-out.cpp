#include "server.h"

#include <cstdlib> // malloc, free
#include <cstring> // memcpy, memset
#include <utility> // std::move

#include "error.h"
#include "request.h"
#include "response.h"

namespace {
    struct Connection {
        http::Server * server;
        http::Request req;
        http::Response res;

        uv_tcp_t tcp;
        uv_buf_t buffer;

        union {
            uv_connect_t connect;
            uv_write_t write;
            uv_shutdown_t shutdown;
        };

        http::Server::HttpResponseCallback callback;

        Connection(uv_loop_t * loop, http::Server * server) : server(server) {
            if (int err = uv_tcp_init(loop, &tcp)) {
                ERROR("%s", uv_strerror(err));
            }

            tcp.data = this;
        }

        ~Connection() {
            free(buffer.base);
        }
    };
}


namespace http {
    void Server::makeRequest(Request && req, const HttpResponseCallback & callback) {
        Connection * conn = new Connection(&loop, this);
        conn->req = std::move(req);
        conn->callback = callback;

        memset(&conn->connect, 0, sizeof(uv_connect_t));
        conn->connect.data = conn;

        if (int err = uv_tcp_connect(&conn->connect, &conn->tcp, (const struct sockaddr *) &req.address.sock, Server::out_onConnect)) {
            ERROR("%s", uv_strerror(err));
        }
    }
}


namespace http {
    void Server::out_onConnect(uv_connect_t * connect, int status) {
        if (status) {
            ERROR("%s", uv_strerror(status));
            return;
        }

        Connection * conn = (Connection *) connect->data;
        if (int err = uv_read_start((uv_stream_t *) &conn->tcp, Server::allocBuffer, Server::out_onRead)) {
            ERROR("%s", uv_strerror(err));
            return;
        }
        conn->buffer = conn->req.end();

        memset(&conn->write, 0, sizeof(uv_write_t));
        conn->write.data = conn;

        if (int err = uv_write(&conn->write, (uv_stream_t *) &conn->tcp, &conn->buffer, 1, Server::out_onWrite)) {
            ERROR("%s", uv_strerror(err));
            return;
        }
    }

    void Server::out_onWrite(uv_write_t * write, int status) {
        Connection * conn = (Connection *) write->data;
        free(conn->buffer.base);
        conn->buffer.base = nullptr;
        conn->buffer.len = 0;
    }

    void Server::out_onRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buffer) {
        Connection * conn = (Connection *) stream->data;

        if (nread >= 0) {
            if (conn->res.parse(buffer, nread)) {
                conn->callback(conn->req, conn->res);
            }
        } else {
            if (nread == UV_EOF) {
                // Do nothing
            } else {
                ERROR("%s", uv_strerror(nread));
            }
        }

        memset(&conn->shutdown, 0, sizeof(uv_shutdown_t));
        if (int err = uv_shutdown(&conn->shutdown, stream, Server::out_onShutdown)) {
            ERROR("%s", uv_strerror(err));
        }

        // Deallocate the buffer memory
        free(buffer->base);
    }

    void Server::out_onShutdown(uv_shutdown_t * shutdown, int status) {
        uv_close((uv_handle_t *) shutdown->handle, Server::out_onClose);
    }

    void Server::out_onClose(uv_handle_t * handle) {
        Connection * conn = (Connection *) handle->data;
        free(conn->buffer.base);
        delete conn;
    }
}
