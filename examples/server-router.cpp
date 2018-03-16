#include <http.h>

int main() {
    // Create the server we will use
    http::Server server;

    // Create the router that we will use to handle requests based on its path
    http::Router router;

    // Set a callback to handle requests to invalid paths
    // Typically this is the 404 page
    router.notFound([] (http::Request & req, http::Response & res) {
        res.setStatus(http::Status::NotFound);
        res.write("404 Not Found");
    });

    // Add a couple of routes that we can use to test with
    router.get("/", [] (http::Request & req, http::Response & res) {
        res.write("hello from \"/\"");
    });

    router.get("/test", [] (http::Request & req, http::Response & res) {
        res.write("hello from \"/test\"");
    });

    // Pass the router to be used to handle all requests
    // The router will delegate to the correct handlers accordingly
    server.onHttpRequest(router);

    // Listen on port 8080 for incoming requests
    if (server.listen(8080)) {
        // Run the server event loop
        // This will block until all events are complete
        // And because we don't stop listening for incoming requests, this will not end
        server.run();
    }

    return 0;
}
