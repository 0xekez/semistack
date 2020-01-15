//
//  instruction.hpp
//  semistack
//
//  Created by Zeke Medley on 1/9/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

#include <optional>
#include <variant>
#include <string>

#include "value.hpp"

namespace vm {

enum class InstType
{
    pi,     // Push immediate..
    sl,     // Store local.
    ll,     // Load local.
    sg,     // Store global.
    lg,     // Load global.
    
    puts,   // Pops and prints top of stack..
    copy,   // Coppies the value on top of the stack.
    
    exit,   // Stops execution.
    ret,    // Pops the current call frame and returns.
    
    add,
    sub,
    mul,
    div,
    
    jump,
    call,
    
    // These pop the compared values off the stack.
    jeq,   // Jump equal.
    jneq,  // Jump not equal.
    jlt,   // Jump less than.
    jgt,   // Jump greater than.
    
    label, // Represents a jumpable location in code. This should only appear in
           // before the code has entered the preprocessor.
};

using Immediate = std::optional<Value>;
using Instruction = std::pair<InstType, Immediate>;

// to_string methods for VM types.
std::string to_string(const InstType& i);
std::string to_string(const Value& v);
std::string to_string(const Immediate& i);
std::string to_string(const Instruction& i);

bool operator==(const Instruction& l, const Instruction& r);

}
