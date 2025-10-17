#pragma once

#include <exception>
#include <string>

class Exception : std::exception {
public:
    virtual ~Exception() = default;
    virtual std::string write() = 0;
};
