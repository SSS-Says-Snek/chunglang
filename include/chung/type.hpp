#pragma once

#include <cstdint>
#include <string>
#include <utility>

enum class Ty : uint8_t {
    // IG a placeholder for type inferencing?
    NONE,

    INVALID,

    UINT64,
    INT64,
    FLOAT64,
    STRING,
    VOID,
    USER
};

class Type {
public:
    Ty ty;
    std::string name;

    // Default values
    static Type none;
    static Type invalid;

    static Type uint64;
    static Type int64;
    static Type float64;
    static Type string;
    static Type void_;

    static Type user(std::string name) {
        return {Ty::USER, std::move(name)};
    }

    Type(Ty ty, std::string name) : ty{ty}, name{std::move(name)} {};

    // Needed for std::map and comparisons??
    bool operator<(const Type& other) const {
        return ty < other.ty;
    }
};
