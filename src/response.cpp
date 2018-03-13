#include "response.h"

#include <sstream>
#include "unicode/ustream.h"

namespace http {
    uv_buf_t Response::end() {
        std::stringstream sstream;

        sstream << "HTTP/1.1 " << int(status) << " " << getStatusMessage(status) << "\r\n";

        for (auto it : headers) {
            sstream << it.first << ": " << it.second << "\r\n";
        }

        sstream << "\r\n";
        sstream << body;

        std::string full = sstream.str();
        size_t length = full.length();

        uv_buf_t buffer = {
            .base = (char *) malloc(length * sizeof(char)),
            .len = length,
        };
        memcpy(buffer.base, full.c_str(), length);

        return buffer;
    }
}
