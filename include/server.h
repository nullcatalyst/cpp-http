#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <uv.h>

#include "./request.h"
#include "./response.h"

namespace http {
    class Server {
    public:
        using HttpRequestHandler = std::function<void (Server & server, Request & req, Response & res)>;
        using HttpResponseHandler = std::function<void (Server & server, Request & req, Response & res)>;
        using AddressHandler = std::function<void (Server & server, uv_getaddrinfo_t)>;

    private:
        uv_loop_t * loop;
        uv_tcp_t tcp;
        uint16_t port;

        std::atomic<bool> running;
        std::mutex mutex;
        std::condition_variable cv;

        HttpRequestHandler httpRequestHandler;

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
        Server(uv_loop_t * loop);
        ~Server();

        void getAddress(const std::string & address, const AddressHandler & addresshandler);

        void onHttpRequest(const HttpRequestHandler & callback) { this->httpRequestHandler = callback; }


        void makeRequest(sockaddr * address, const Request & req, const HttpResponseHandler & callback);
        // void makeRequestToDomain(const char * domain, uint16_t port, const Request & request);

        bool listen(uint16_t port);


        void run();

        /**
         * This function blocks until the server has stopped.
         */
        void close();
    };
}
