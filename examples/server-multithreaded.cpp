#include <thread>
#include <vector>
#include <http.h>

int main() {
    // Create the router that we will use to handle requests based on its path.
    // A router is never modified by the server, so a single router can be shared between all threads.
    http::Router router;

    // Set a callback to handle requests to invalid paths.
    // Typically this is the 404 page.
    router.error([] (const std::exception & error, http::Request & req, http::Response & res) {
        // This status is set automatically, if it is not otherwise overridden.
        res.setStatus(http::Status::InternalServerError);

        // Note that the response may be filled with a partially complete response, depending on when the exception occurred.
        // So we may want to clear the response first.
        res.clear();

        res.write("500 Internal Server Error");
    });

    // Set a callback to handle requests to invalid paths.
    // Typically this is the 404 page.
    router.notFound([] (http::Request & req, http::Response & res) {
        // This status is set automatically, if it is not otherwise overridden.
        res.setStatus(http::Status::NotFound);

        res.write("404 Not Found");
    });

    // Add a couple of routes that we can use to test with.
    router.get("/", [] (http::Request & req, http::Response & res) {
        res.write("hello from \"/\"");
    });

    router.get("/test", [] (http::Request & req, http::Response & res) {
        res.write("hello from \"/test\"");
    });

    router.get("/error", [] (http::Request & req, http::Response & res) {
        // Any class of exception can be thrown, as long as the class is derived from `std::exception`.
        throw std::exception();
    });

    // Store a reference to each thread, so we can use them later if desired.
    std::vector<std::thread> tasks;

    // The handle to the shared socket that all threads will listen to.
    uv_os_sock_t sock;

    // Some multi-threading hardware
    std::mutex m;
    std::condition_variable cv;

    // Create a thread per core.
    const int THREAD_COUNT = std::thread::hardware_concurrency();
    for (int i = 0; i < THREAD_COUNT; ++i) {
        std::thread task([&, i] () {
            // Create the server we will use.
            http::Server server;

            // Pass the router to be used to handle all requests.
            // The router will delegate to the correct handlers accordingly.
            server.onHttpRequest(router);

            // Handle the first thread slightly differently when creating sockets...
            bool success;
            if (i == 0) {
                // Create the socket, and listen on port 8080 for incoming requests.
                success = server.listen(8080);

                if (success) {
                    // We are listening into the port.
                    // Let all of our co-workers know that they can listen too.
                    sock = server.getSocket();
                    cv.notify_all();
                } else {
                    // We couldn't listen to the port.
                    // Rather than going into the intricacies of proper error handling, just `abort()`.
                    abort();
                }
            } else {
                // Wait until the socket has been created so we can use it too.
                std::unique_lock<std::mutex> lock(m);
                cv.wait(lock, [&sock] () { return sock; });

                // Reuse the same socket as the first thread.
                success = server.reuseSocket(sock);
            }

            if (success) {
                // Note: This can be printed out of order
                printf("Running server[%d]\n", i);

                // Run the server event loop.
                // This will block until all events are complete.
                // And because we don't stop listening for incoming requests, this will not end.
                server.run();
            } else {
                printf("Error: failed to start server[%d]\n", i);
            }
        });

        // Threads can't be copied, but they can be moved.
        // So move this one to the back of the array.
        tasks.push_back(std::move(task));
    }

    // Some time passes...

    // Join each thread.
    for (int i = 0; i < tasks.size(); ++i) {
        tasks[i].join();
    }
    tasks.clear();

    return 0;
}
