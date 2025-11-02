#pragma once

#include <exception>
#include <string>
#include <vector>

class Exception : std::exception {
public:
    virtual ~Exception() = default;
    virtual std::string write(const std::vector<std::string>& source_lines) = 0;
};
