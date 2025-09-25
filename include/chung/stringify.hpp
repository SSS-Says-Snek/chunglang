#pragma once

#include <iostream>
#include <string>

#include "chung/ast.hpp"

std::string stringify(const TokenType& op);
std::string stringify(const Token& token);