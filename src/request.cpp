#include "request.h"

#include "./http-parser/http_parser.h"

namespace {
    struct http_parser_state : public http_parser {
        const char * header_field_at;
        size_t header_field_length;
    };

    const http_parser_settings settings = {
        .on_url = [] (http_parser * parser, const char * at, size_t len) -> int {
            http::Request * req = (http::Request *) parser->data;

            if (at != nullptr && req != nullptr) {
                req->setUrl(http::convertRawToUnicode(at, len));
            }

            return 0;
        },

        .on_header_field = [] (http_parser * parser, const char * at, size_t length) -> int {
            http_parser_state * state = (http_parser_state *) parser;
            state->header_field_at = at;
            state->header_field_length = length;

            return 0;
        },

        .on_header_value = [] (http_parser * parser, const char * at, size_t length) -> int {
            http::Request * req = (http::Request *) parser->data;

            http_parser_state * state = (http_parser_state *) parser;
            req->addHeader(
                http::convertRawToUnicode(state->header_field_at, state->header_field_length).toLower(),
                http::convertRawToUnicode(at, length)
                );

            return 0;
        },

        .on_headers_complete = [] (http_parser * parser) -> int {
            http::Request * req = (http::Request *) parser->data;

            http::Method method;
            switch ((enum http_method) parser->method) {
                case HTTP_DELETE:       method = http::Method::Delete;  break;
                case HTTP_GET:          method = http::Method::Get;     break;
                case HTTP_HEAD:         method = http::Method::Head;    break;
                case HTTP_POST:         method = http::Method::Post;    break;
                case HTTP_PUT:          method = http::Method::Put;     break;
                default:                method = http::Method::Unknown; break;
            }

            return 0;
        },

        .on_body = [] (http_parser * parser, const char * at, size_t len) -> int {
            http::Request * req = (http::Request *) parser->data;

            if (at != nullptr && req != nullptr && (int) len > -1) {
                req->setBody(http::convertRawToUnicode(at, len));
            }

            return 0;
        },

        .on_message_complete = [] (http_parser * parser) -> int {
            return 0;
        },
    };
}

namespace http {
    bool Request::parse(const uv_buf_t * buffer, ssize_t nread) {
        http_parser_state parser;
        http_parser_init(&parser, HTTP_REQUEST);
        parser.data = this;
        parser.header_field_at = nullptr;
        parser.header_field_length = 0;

        ssize_t parsed = (ssize_t) http_parser_execute(&parser, &settings, buffer->base, nread);
        if (parsed < nread) {
            return false;
        }

        return true;
    }
}
