#pragma once

#include <memory>
#include <string>

enum class Ty {
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

    Type(Ty ty, std::string name): ty{ty}, name{std::move(name)} {};

    inline virtual bool operator <(const Type& other) const {
        return ty < other.ty;
    }
};