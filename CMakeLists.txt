cmake_minimum_required(VERSION 3.10)

project(http)
set(CMAKE_CXX_STANDARD 17)

find_package(ICU COMPONENTS uc io REQUIRED)

add_library(http "")

target_include_directories(http
PUBLIC
    include
PRIVATE
    src
)

target_sources(http
PRIVATE
    src/address.cpp
    src/message.cpp
    src/request.cpp
    src/response.cpp
    src/router.cpp
    src/server.cpp

    src/http-parser/http_parser.c
)

target_link_libraries(http
PUBLIC
    uv
    ICU::uc
    ICU::io
)
