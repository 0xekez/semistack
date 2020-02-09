//
//  expression.h
//  semistack
//
//  Created by Zeke Medley on 1/12/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

#include <variant>
#include <memory>
#include <string>
#include <vector>
#include <list>

#include "instruction.hpp"

#include "semistack/util.hpp"
#include "semistack/logger.hpp"
#include "semistack/function.hpp"

namespace lust {

using Expression = std::variant<
                        std::unique_ptr<struct Number>,
                        std::unique_ptr<struct List>,
                        std::unique_ptr<struct Symbol>,
                        std::unique_ptr<struct Function>>;

struct List
{
    std::list<Expression> _exprs;
    
    List(std::list<Expression> exprs): _exprs(std::move(exprs)) {}
};

struct Number
{
    float _num;
    Number(float num): _num(std::move(num)) {}
};

struct Symbol
{
    std::string _sym;
    Symbol(std::string sym): _sym(std::move(sym)) {}
};

struct Function
{
    // For scope resolution and call verification (correct num of args).
    std::vector<std::string> _args;
    Expression _body;
};

void printExpression(const Expression& e);
bool codegen(const Expression& e, vm::Function& target);

}
