#pragma once

#include <atomic>
#include <functional>
#include <uv.h>

#include "./address.h"
#include "./request.h"
#include "./response.h"

namespace http {
    class Server {
    public:
        using HttpRequestCallback   = std::function<void (Request & req, Response & res)>;
        using HttpResponseCallback  = std::function<void (Request & req, Response & res)>;
        using DNSLookupCallback     = std::function<void (Address & address)>;

    private:
        uv_loop_t loop;
        uv_tcp_t tcp;

        std::atomic<bool> running;

        HttpRequestCallback httpRequestCallback;

        static void onAddress(uv_getaddrinfo_t * req, int status, struct addrinfo * res);
        static void allocBuffer(uv_handle_t * handle, size_t suggestedSize, uv_buf_t * buffer);

        static void in_onConnect(uv_stream_t * stream, int status);
        static void in_onRead(uv_stream_t * handle, ssize_t nread, const uv_buf_t * buffer);
        static void in_onWrite(uv_write_t * write, int status);
        static void in_onShutdown(uv_shutdown_t * shutdown, int status);
        static void in_onClose(uv_handle_t * handle);

        static void out_onConnect(uv_connect_t * connect, int status);
        static void out_onWrite(uv_write_t * write, int status);
        static void out_onRead(uv_stream_t * handle, ssize_t nread, const uv_buf_t * buffer);
        static void out_onShutdown(uv_shutdown_t * shutdown, int status);
        static void out_onClose(uv_handle_t * handle);

    public:
        Server();
        ~Server();

        void makeDNSLookup(const std::string & domainName, const DNSLookupCallback & callback);
        void makeRequest(Request && req, const HttpResponseCallback & callback);

        void onHttpRequest(const HttpRequestCallback & callback) { this->httpRequestCallback = callback; }

        bool listen(uint16_t port);
        bool reuseSocket(const Server & server);

        void run();

        /**
         * This function blocks until the server has stopped.
         */
        void close();
    };
}
