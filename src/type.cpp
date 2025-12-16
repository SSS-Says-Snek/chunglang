#include "chung/type.hpp"

// Not actually types
Type Type::none = Type{Ty::NONE, "none"};
Type Type::invalid = Type{Ty::INVALID, "invalid"};

Type Type::uint64 = Type{Ty::UINT64, "uint64"};
Type Type::int64 = Type{Ty::INT64, "int64"};
Type Type::float64 = Type{Ty::FLOAT64, "float64"};
Type Type::string = Type{Ty::STRING, "string"};
Type Type::void_ = Type{Ty::VOID, "void"};
// Doesn't make sense to make a tuser default (since the identifier is... user-specified)
