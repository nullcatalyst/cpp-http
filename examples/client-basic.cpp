#include <http.h>

int main() {
    // Create the server we will use.
    http::Server server;

    // Make a DNS lookup for "google.com".
    server.makeDNSLookup("google.com", [] (http::Server & server, http::Address & address) {
        http::Request req;

        // Print the IP address, just for the curious.
        printf("address: %s\n", address.toString().c_str());

        // Set the port to 80, as we are going to make a regular HTTP request.
        address.setPort(80);

        // Set the IP address to send the request to.
        req.setAddress(address);

        // Set the URL (defaults to "/", but this is a useful example anyway).
        req.setUrl("/");

        // Add some request headers, if desired.
        req.setHeader("test-header", "test-value");

        // Finally, make the HTTP request.
        server.makeRequest(address, req, [] (http::Server & server, http::Request & req, http::Response & res) {
            // Print the response.
            printf("%s\n", http::convertUnicodeToString(res.getBody()).c_str());
        });
    });

    // Run the server event loop.
    // This will block until all events are complete.
    server.run();

    return 0;
}
