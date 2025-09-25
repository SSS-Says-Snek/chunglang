#pragma once

#include <string>

class Exception {
public:
    virtual ~Exception() = default;
    virtual std::string write() = 0;
};