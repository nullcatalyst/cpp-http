#pragma once

#if __cpp_exceptions >= 199711
#include <exception>
#endif

#include <functional>
#include <unordered_map>

#include "./unicode-string.h"
#include "./request.h"
#include "./response.h"
#include "./server.h"

namespace http {
    class Router {
    public:
        using HttpResponseCallback = std::function<void (Request & req, Response & res)>;
        using HttpErrorCallback = std::function<void (const std::exception & error, Request & req, Response & res)>;
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
            std::unordered_map<icu::UnicodeString, Route *> routes;

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

#if __cpp_exceptions >= 199711
        /**
         * The ultimate destination of any route if an uncaught exception is thrown.
         * Typically this is the 500 page.
         */
        HttpErrorCallback errorCallback;
#endif

        /**
         * The base route for this router.
         */
        Route route;

    public:
        Router();

        Router & notFound(const HttpResponseCallback & callback) {
            this->notFoundCallback = callback;
            return *this;
        }

#if __cpp_exceptions >= 199711
        Router & error(const HttpErrorCallback & callback) {
            this->errorCallback = callback;
            return *this;
        }
#endif

        Router & on(Method method, const icu::UnicodeString & path, const HttpResponseCallback & callback);
        Router & get(const icu::UnicodeString & url, const HttpResponseCallback & callback) { return on(Method::Get, url, callback); }
        Router & post(const icu::UnicodeString & url, const HttpResponseCallback & callback) { return on(Method::Post, url, callback); }

        void operator () (Request & req, Response & res) const;
    };
}
