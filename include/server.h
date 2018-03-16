#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
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
        uint16_t port;

        std::atomic<bool> running;
        std::mutex mutex;
        std::condition_variable cv;

        HttpRequestCallback httpRequestCallback;

        static void onAddress(uv_getaddrinfo_t * req, int status, struct addrinfo * res);

        static void allocBuffer(uv_handle_t * handle, size_t suggestedSize, uv_buf_t * buffer);
        static void close(uv_handle_t * handle);
        static void shutdown(uv_shutdown_t * shutdown, int status);

        static void onConnection(uv_stream_t * stream, int status);
        static void onRequestRead(uv_stream_t * handle, ssize_t nread, const uv_buf_t * buffer);
        static void onResponseWrite(uv_write_t * write, int status);

        static void onConnect(uv_connect_t * connect, int status);
        static void onRequestWrite(uv_write_t * write, int status);
        static void onResponseRead(uv_stream_t * handle, ssize_t nread, const uv_buf_t * buffer);

    public:
        Server();
        ~Server();

        /**
         * Libuv owns the socket. DO NOT BREAK IT.
         * This exists as a method of sharing a socket between threads.
         */
        uv_os_sock_t getSocket() const {
#ifdef _WIN32
            return tcp.socket;
#else
            return tcp.io_watcher.fd;
#endif
        }

        void makeDNSLookup(const std::string & domainName, const DNSLookupCallback & callback);
        void makeRequest(const Address & address, const Request & req, const HttpResponseCallback & callback);

        void onHttpRequest(const HttpRequestCallback & callback) { this->httpRequestCallback = callback; }

        bool listen(uint16_t port);
        bool reuseSocket(uv_os_sock_t sock);

        void run();

        /**
         * This function blocks until the server has stopped.
         */
        void close();
    };
}
