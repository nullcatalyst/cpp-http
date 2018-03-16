#pragma once

#include <cstdint> // uint32_t

namespace http {
#define HTTP_STATUS_MAP(XX)                                                     \
    XX(-1,  Unknown,                        Unknown)                            \
    XX(100, Continue,                       Continue)                           \
    XX(101, SwitchingProtocols,             Switching Protocols)                \
    XX(200, OK,                             OK)                                 \
    XX(201, Created,                        Created)                            \
    XX(202, Accepted,                       Accepted)                           \
    XX(203, NonAuthoritativeInformation,    Non-Authoritative Information)      \
    XX(204, NoContent,                      No Content)                         \
    XX(205, ResetContent,                   Reset Content)                      \
    XX(206, PartialContent,                 Partial Content)                    \
    XX(300, MultipleChoices,                Multiple Choices)                   \
    XX(301, MovedPermanently,               Moved Permanently)                  \
    XX(302, Found,                          Found)                              \
    XX(303, SeeOther,                       See Other)                          \
    XX(304, NotModified,                    Not Modified)                       \
    XX(307, TemporaryRedirect,              Temporary Redirect)                 \
    XX(308, PermanentRedirect,              Permanent Redirect)                 \
    XX(400, BadRequest,                     Bad Request)                        \
    XX(401, Unauthorized,                   Unauthorized)                       \
    XX(402, PaymentRequired,                Payment Required)                   \
    XX(403, Forbidden,                      Forbidden)                          \
    XX(404, NotFound,                       Not Found)                          \
    XX(405, MethodNotAllowed,               Method Not Allowed)                 \
    XX(406, NotAcceptable,                  Not Acceptable)                     \
    XX(407, ProxyAuthenticationRequired,    Proxy Authentication Required)      \
    XX(408, RequestTimeout,                 Request Timeout)                    \
    XX(409, Conflict,                       Conflict)                           \
    XX(410, Gone,                           Gone)                               \
    XX(411, LengthRequired,                 Length Required)                    \
    XX(412, PreconditionFailed,             Precondition Failed)                \
    XX(413, PayloadTooLarge,                Payload Too Large)                  \
    XX(414, URITooLong,                     URI Too Long)                       \
    XX(415, UnsupportedMediaType,           Unsupported Media Type)             \
    XX(416, RangeNotSatisfiable,            Range Not Satisfiable)              \
    XX(417, ExpectationFailed,              Expectation Failed)                 \
    XX(426, UpgradeRequired,                Upgrade Required)                   \
    XX(428, PreconditionRequired,           Precondition Required)              \
    XX(429, TooManyRequests,                Too Many Requests)                  \
    XX(431, RequestHeaderFieldsTooLarge,    RequestHeader Fields Too Large)     \
    XX(451, UnavailableForLegalReasons,     Unavailable For Legal Reasons)      \
    XX(500, InternalServerError,            Internal Server Error)              \
    XX(501, NotImplemented,                 Not Implemented)                    \
    XX(502, BadGateway,                     Bad Gateway)                        \
    XX(503, ServiceUnavailable,             Service Unavailable)                \
    XX(504, GatewayTimeout,                 Gateway Timeout)                    \
    XX(505, HTTPVersionNotSupported,        HTTP Version Not Supported)         \
    XX(511, NetworkAuthenticationRequired,  Network Authentication Required)    \

    enum class Status {
#define XX(num, name, string) name = num,
        HTTP_STATUS_MAP(XX)
#undef XX
    };

    inline const char * getStatusString(Status status) {
        switch (status) {
#define XX(num, name, string) case Status:: name : return #string ;
            HTTP_STATUS_MAP(XX)
#undef XX
            default: return "";
        }
    }

#undef HTTP_STATUS_MAP
}
