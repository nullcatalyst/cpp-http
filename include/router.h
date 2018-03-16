#pragma once

#include <functional>
#include <unordered_map>

#include "./unicode-string.h"
#include "./request.h"
#include "./response.h"
#include "./server.h"

namespace http {
    class Router {
    public:
        // using HttpResponseCallback = Server::HttpResponseCallback;
        using HttpResponseCallback = std::function<void (Request & req, Response & res)>;
        using WildcardRouteCallback = std::function<void (const icu::UnicodeString & token, Request & req, Response & res)>;

    private:
        struct Route {
            /**
             * The callbacks to use for the given method.
             * Currently this only supports GET and POST.
             */
            HttpResponseCallback methods[int(Method::Count)];

            /**
             * The map of sub-paths reachable from this one.
             */
            std::unordered_map<icu::UnicodeString, Route> routes;

            /**
             * A wildcard path, if the URL does not match any of the sub-paths.
             * There can only be one wildcard handler.
             */
            // WildcardRouteCallback wildcard;
        };

        /**
         * The ultimate destination if the path does not match any route.
         * Typically this is the 404 page.
         */
        HttpResponseCallback notFoundCallback;

        /**
         * The base route for this router.
         */
        Route route;

    public:
        Router & notFound(const HttpResponseCallback & callback) {
            this->notFoundCallback = callback;
            return *this;
        }

        Router & handle(Method method, const icu::UnicodeString & path, const HttpResponseCallback & callback);
        Router & get(const icu::UnicodeString & url, const HttpResponseCallback & callback) { return handle(Method::Get, url, callback); }
        Router & post(const icu::UnicodeString & url, const HttpResponseCallback & callback) { return handle(Method::Post, url, callback); }

        void operator () (Request & req, Response & res) const;
    };
}
