#include "router.h"

#include <iterator>

namespace {
    class URLIterator {
        const icu::UnicodeString & url;

    public:
        class iterator : public std::iterator<std::input_iterator_tag, const icu::UnicodeString> {
            const icu::UnicodeString & url;
            int begin;
            int end;

        public:
            explicit iterator(const icu::UnicodeString & url, int begin, int end) : url(url), begin(begin), end(end) {}

            const icu::UnicodeString operator * () const {
                return url.tempSubStringBetween(begin, end);
            }

            iterator & operator ++ () {
                begin = end + 1;

                end = url.indexOf("/", begin);
                if (end < 0) {
                    end = url.length();
                }

                return *this;
            }

            iterator operator ++ (int) {
                int begin = this->end + 1;

                int end = url.indexOf("/", begin);
                if (end < 0) {
                    end = url.length();
                }

                return iterator{
                    url,
                    begin,
                    end,
                };
            }

            bool operator == (const iterator & other) const { return begin == other.begin; }
            bool operator != (const iterator & other) const { return begin != other.begin; }
        };

        URLIterator(const icu::UnicodeString & url) : url(url) {}

        iterator begin() {
            int end = url.indexOf("/", 1);
            if (end < 0) {
                end = url.length();
            }

            return iterator{
                url,
                1,
                end,
            };
        }

        iterator end() {
            int end = url.length();

            return iterator{
                url,
                end + 1,
                end,
            };
        }
    };
}

namespace http {
    Router::Router() {
        error([] (const std::exception & error, http::Request & req, http::Response & res) {
            res.clear();
            res.getBody() << "500 Internal Server Error";
        });

        notFound([] (http::Request & req, http::Response & res) {
            res.getBody() << "404 Not Found";
        });
    }

    Router & Router::on(Method method, const icu::UnicodeString & url, const HttpResponseCallback & callback) {
        Route * currRoute = &route;

        for (const icu::UnicodeString & path : URLIterator(url)) {
            auto it = currRoute->routes.find(path);
            if (it != currRoute->routes.end()) {
                currRoute = it->second;
            } else {
                const auto [it, success] = currRoute->routes.emplace(icu::UnicodeString(path), new Route());
                currRoute = it->second;
            }
        }

        currRoute->methods[int(method)] = callback;

        return *this;
    }

    void Router::operator () (Request & req, Response & res) const {
#if __cpp_exceptions >= 199711
        try {
#endif
            const int method = int(req.getMethod());
            if (method < 0 || method >= int(Method::Count)) {
                // Check if this is a valid method that this router can handle
                res.setStatus(Status::NotFound);
                return notFoundCallback(req, res);
            }

            const Route * currRoute = &route;

            for (const icu::UnicodeString & path : URLIterator(req.getUrl())) {
                auto it = currRoute->routes.find(path);
                if (it != currRoute->routes.end()) {
                    currRoute = it->second;
                } else {
                    res.setStatus(Status::NotFound);
                    return notFoundCallback(req, res);
                }
            }

            if (currRoute->methods[method]) {
                return currRoute->methods[method](req, res);
            } else {
                res.setStatus(Status::NotFound);
                return notFoundCallback(req, res);
            }
#if __cpp_exceptions >= 199711
        } catch (const std::exception & error) {
            res.setStatus(Status::InternalServerError);
            return errorCallback(error, req, res);
        }
#endif
    }
}
