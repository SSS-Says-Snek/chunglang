#include <memory>
#include <vector>

#include "ast.hpp"

class Sema {
public:
    std::vector<std::unique_ptr<StmtAST>> ast;

    explicit Sema(std::vector<std::unique_ptr<StmtAST>> ast) : ast{std::move(ast)} {

    }
};
