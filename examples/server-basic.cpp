#include <http.h>

int main() {
    // Create the server we will use.
    http::Server server;

    // Set the function handler that will be called on each valid HTTP request.
    // Invalid requests will not result in this handler being called.
    server.onHttpRequest([] (http::Server & server, http::Request & req, http::Response & res) {
        // Set the response status (defaults to 200 "OK", but this is a useful example anyway).
        res.setStatus(http::Status::OK);

        // Add some response headers, if desired.
        res.setHeader("test-header", "test-value");

        // Write to the body of the response.
        // Multiple calls to `write()` will append the results together.
        res.write("Hello World");
    });

    // Listen on port 8080 for incoming requests.
    if (server.listen(8080)) {
        // Run the server event loop.
        // This will block until all events are complete.
        // And because we don't stop listening for incoming requests, this will not end.
        server.run();
    }

    return 0;
}
