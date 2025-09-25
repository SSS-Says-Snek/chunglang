#include "chung/type.hpp"

// Not actually types
Type Type::tnone = Type{Ty::TNONE, "none"};
Type Type::tinvalid = Type{Ty::TINVALID, "invalid"};

Type Type::tuint64 = Type{Ty::TUINT64, "uint64"};
Type Type::tint64 = Type{Ty::TINT64, "int64"};
Type Type::tfloat64 = Type{Ty::TFLOAT64, "float64"};
Type Type::tstring = Type{Ty::TSTRING, "string"};