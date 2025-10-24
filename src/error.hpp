#pragma once

#include <format>
#include <cstddef>
#include <iostream>

class Error {
public:
    Error() = delete;
    ~Error() = delete;

    Error(Error& copy) = delete;
    Error(Error&& copy) = delete;

    inline static void warning(size_t line, const char* msg) {
        std::cerr << std::format("WARN (line={}): {}\n", line, msg);
    }

    inline static void critical(size_t line, const char* msg) {
        std::cerr << std::format("ERROR (line={}): {}\n", line, msg);
        exit(1);
    }
};