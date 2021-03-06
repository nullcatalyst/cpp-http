#include <http.h>

int main() {
    // Create the server we will use.
    http::Server server;

    // Create the router that we will use to handle requests based on its path.
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

    // Pass the router to be used to handle all requests.
    // The router will delegate to the correct handlers accordingly.
    server.onHttpRequest(router);

    // Listen on port 8080 for incoming requests.
    if (server.listen(8080)) {
        // Run the server event loop.
        // This will block until all events are complete.
        // And because we don't stop listening for incoming requests, this will not end.
        server.run();
    }

    return 0;
}
