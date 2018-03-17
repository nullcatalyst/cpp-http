#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include <http.h>

struct ServerThread {
    http::Server * server;
    std::thread thread;

    ServerThread(http::Server * server, std::thread && thread) : server(server), thread(std::move(thread)) {}
};

// Store a reference to each thread, so we can use them later if desired.
const int THREAD_COUNT = std::thread::hardware_concurrency();
std::vector<ServerThread> tasks;


http::Server * createServer(const http::Router & router, http::Server * master, int index) {
    http::Server * server = new http::Server();

    server->onHttpRequest(router);

    bool success;
    if (master == nullptr) {
        success = server->listen(8080);
    } else {
        success = server->reuseSocket(*master);
    }

    if (success) {
        std::thread task([=, &router] () {
            printf("Running Server[%d]\n", index);
            server->run();

            printf("Server[%d] is complete\n", index);
        });

        tasks.emplace_back(server, std::move(task));
    } else {
        printf("Error: failed to start Server[%d]\n", index);
        abort();
    }

    return server;
}


int main() {
    // Create the router that we will use to handle requests based on its path.
    // A router is never modified by the server, so a single router can be shared between all threads.
    http::Router router;

    // Add a couple of routes that we can use to test with.
    router.get("/", [] (http::Request & req, http::Response & res) {
        res.write("hello from \"/\"");
    });

    // Useful for testing purposes; a special route that will stop the server if necessary.
    router.get("/stop", [&] (http::Request & req, http::Response & res) {
        res.write("stop");

        for (int i = tasks.size() - 1; i >= 0; --i) {
            tasks[i].server->close();
        }
    });

    // Create a thread per core.
    http::Server * master = createServer(router, nullptr, 0);
    for (int i = 1; i < THREAD_COUNT; ++i) {
        createServer(router, master, i);
    }

    // Some time passes...

    // Join each thread.
    for (int i = tasks.size() - 1; i >= 0; --i) {
        tasks[i].thread.join();
    }
    tasks.clear();

    return 0;
}
