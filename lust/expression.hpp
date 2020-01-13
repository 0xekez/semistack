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
#include "util.hpp"
#include "logger.hpp"

namespace lust {

using Expression = std::variant<
                        std::unique_ptr<struct Number>,
                        std::unique_ptr<struct List>,
                        std::unique_ptr<struct Symbol>>;

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

using CodegenRes = std::optional<std::vector<vm::Instruction>>;

CodegenRes codegen(const Expression& e);

}
