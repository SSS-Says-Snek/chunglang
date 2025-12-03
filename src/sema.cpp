#include "chung/ast.hpp"
#include <memory>

std::unique_ptr<ResolvedAST> VarDeclareAST::resolve() {
}

std::unique_ptr<ResolvedAST> PrimitiveAST::resolve() {
    // switch (token.type) {
    //     case TokenType::INT64: {
    //         int64_t int64 = std::stoll(token.text);
    //         return std::make_unique<PrimitiveAST>(int64);
    //     }
    //     case TokenType::UINT64: {
    //         uint64_t uint64 = std::stoull(token.text);
    //         return std::make_unique<PrimitiveAST>(uint64);
    //     }
    //     case TokenType::FLOAT64: {
    //         double float64 = std::stod(token.text);
    //         return std::make_unique<PrimitiveAST>(float64);
    //     }
    //     case TokenType::STRING: {
    //         return std::make_unique<PrimitiveAST>(token.text);
    //     }
    //     default:
    //         // Invalid token
    //         throw push_exception("Invalid token in expression", token);
    // }

}
