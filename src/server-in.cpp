#include "server.h"

#include <cstdlib> // malloc, free
#include <cstring> // memcpy, memset

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
            uv_write_t write;
            uv_shutdown_t shutdown;
        };

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
    void Server::in_onConnect(uv_stream_t * stream, int status) {
        if (status) {
            ERROR("%s", uv_strerror(status));
            return;
        }

        Connection * conn = new Connection(stream->loop, (Server *) stream->data);

        // Accept the connection
        if (int err = uv_accept(stream, (uv_stream_t *) &conn->tcp)) {
            ERROR("%s", uv_strerror(err));

            memset(&conn->shutdown, 0, sizeof(uv_shutdown_t));
            if (int err = uv_shutdown(&conn->shutdown, (uv_stream_t *) &conn->tcp, Server::in_onShutdown)) {
                ERROR("%s", uv_strerror(err));
            }

            return;
        }

        // Get the client's address
        int socklen = sizeof(struct sockaddr_storage);
        if (int err = uv_tcp_getpeername((const uv_tcp_t *) &conn->tcp, (struct sockaddr *) &conn->req.address.sock, &socklen)) {
            ERROR("%s", uv_strerror(err));
        }

        // Start reading
        if (int err = uv_read_start((uv_stream_t *) &conn->tcp, Server::allocBuffer, Server::in_onRead)) {
            ERROR("%s", uv_strerror(err));
            return;
        }
    }

    void Server::in_onRead(uv_stream_t * stream, ssize_t nread, const uv_buf_t * buffer) {
        Connection * conn = (Connection *) stream->data;

        if (nread >= 0) {
            if (conn->req.parse(buffer, nread)) {
                // printf("METHOD: %s\n", getMethodString(conn->req.getMethod()));
                // printf("URL: %s\n", convertUnicodeToString(conn->req.getUrl()).c_str());
                // for (auto it : conn->req.headers) {
                //     printf("HEADER: %s=%s\n", convertUnicodeToString(it.first).c_str(), convertUnicodeToString(it.second).c_str());
                // }

                conn->server->httpRequestCallback(conn->req, conn->res);
                conn->buffer = conn->res.end();

                memset(&conn->write, 0, sizeof(uv_write_t));
                conn->write.data = conn;

                uv_write(&conn->write, stream, &conn->buffer, 1, Server::in_onWrite);
            }
        } else {
            if (nread == UV_EOF) {
                // Do nothing
            } else {
                ERROR("%s", uv_strerror(nread));
            }

            if (int err = uv_shutdown(&conn->shutdown, stream, Server::in_onShutdown)) {
                ERROR("%s", uv_strerror(err));
            }
        }

        // Deallocate the buffer memory
        free(buffer->base);
    }

    void Server::in_onWrite(uv_write_t * write, int status) {
        Connection * conn = (Connection *) write->data;

        // conn->shutdown.data = conn;
        // if (int err = uv_shutdown(&conn->shutdown, (uv_stream_t *) &conn->tcp, Server::in_onShutdown)) {
        //     ERROR("%s", uv_strerror(err));
        // }

        uv_close((uv_handle_t *) &conn->tcp, Server::in_onClose);
    }

    void Server::in_onShutdown(uv_shutdown_t * shutdown, int status) {
        uv_close((uv_handle_t *) shutdown->handle, Server::in_onClose);
    }

    void Server::in_onClose(uv_handle_t * handle) {
        Connection * conn = (Connection *) handle->data;
        delete conn;
    }
}
