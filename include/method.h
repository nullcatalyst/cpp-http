#pragma once

namespace http {
    enum class Method : uint32_t {
        Unknown = 0,

        Delete,
        Get,
        Head,
        // Options,
        // Patch,
        Post,
        Put,
    };
}
