#pragma once

#include <functional>
#include <uv.h>

#include "./request.h"
#include "./response.h"

namespace http {
    class Server {
        using HttpRequestHandler = std::function<void (Request & req, Response & res)>;

        uv_loop_t * loop;
        uv_tcp_t tcp;
        uint16_t port;
        HttpRequestHandler httpRequestHandler;

        static void onConnection(uv_stream_t * server, int status);
        static void onShutdown(uv_shutdown_t * shutdownReq, int status);
        static void onClose(uv_handle_t * handle);

        static void onAllocBuffer(uv_handle_t * handle, size_t suggestedSize, uv_buf_t * buffer);
        static void onRead(uv_stream_t * handle, ssize_t nread, const uv_buf_t * buffer);
        static void onWrite(uv_write_t * writeReq, int status);

    public:
        Server();
        Server(uv_loop_t * loop);
        ~Server();

        void onHttpRequest(const HttpRequestHandler & httpRequestHandler) { this->httpRequestHandler = httpRequestHandler; }

        void listen(uint16_t port);
    };
}
