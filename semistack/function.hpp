//
//  module.hpp
//  semistack
//
//  Created by Zeke Medley on 1/9/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

#include "instruction.hpp"
#include "util.hpp"

namespace vm {

// Upvalues are the means through which functions access variables in their
// closures. An upvalue stores a pointer to a Value that starts as being on the
// stack. When that value would normally be popped off the stack because the
// call frame it belongs to is destroyed the upvalue moves it into itself.
//
// We represent that as a variant. The pointer indicating that the value is
// still on the stack and a unique_ptr indicating that that value is owned by
// the upvalue.
using Upvalue = std::variant<Value*, std::unique_ptr<Value>>;

struct Function
{
    // We only ever create one upvalue per variable and we store them in here.
    // Once an upvalue is closed it is moved out of the list. This allows
    // closures to have mutable state.
    static std::list<Upvalue> openUpValues;
    
    std::vector<vm::Instruction> _instructions;
    
    std::vector<Upvalue> _closedUpvalues;
    
    bool operator==(const Function& l)
    {
        return _closedUpvalues == l._closedUpvalues &&
               _instructions == l._instructions;
    }
    
    void addInstruction(Instruction i)
    {
        _instructions.push_back(std::move(i));
    }
    
    template<class T>
    void addInstruction(InstType type, T immediate)
    {
        _instructions.emplace_back(
            util::make_instruction(std::move(type), std::move(immediate)));
    }
    
    void addInstruction(InstType type)
    {
        _instructions.emplace_back(util::make_instruction(std::move(type)));
    }
};

}
