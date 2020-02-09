//
//  compiler.hpp
//  lust
//
//  Created by Zeke Medley on 1/16/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

#include <map>

#include "expression.hpp"
#include "semistack/util.hpp"

namespace lust {

enum class Scope: std::uint8_t
{
    global,
    local,
};

// Scope and offset in that scope.
using ID = std::pair<Scope, size_t>;

class Compiler
{
private:
    // TODO: for globals store pointer to parent compiler.
    std::map<std::string, size_t> _locals;
    vm::Function& _target;
    
    bool compileDef(const std::list<Expression>& args);
    bool compilePrint(const std::list<Expression>& args);
    
public:
    bool compileExpression(const Expression& e);
    
    Compiler(vm::Function& target): _target(target) {}
};

}
