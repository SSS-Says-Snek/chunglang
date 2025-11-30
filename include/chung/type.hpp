#pragma once

#include <cstdint>
#include <string>

enum class Ty : uint8_t {
    // IG a placeholder for type inferencing?
    TNONE,

    TINVALID,

    TUINT64,
    TINT64,
    TFLOAT64,
    TSTRING
};

class Type {
public:
    Ty ty;
    std::string name;

    static Type tnone;
    static Type tinvalid;

    static Type tuint64;
    static Type tint64;
    static Type tfloat64;
    static Type tstring;
    static Type tuser;

    Type(Ty ty, std::string name) : ty{ty}, name{std::move(name)} {};

    // Needed for std::map and comparisons??
    bool operator<(const Type& other) const {
        return ty < other.ty;
    }
};
