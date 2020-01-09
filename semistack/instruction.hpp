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

namespace vm
{

enum class InstType
{
    pi,    // push immediate
    puts   // pops and prints top of stack.
};

using Value = std::variant<float, std::string>;
using Immediate = std::optional<Value>;
using Instruction = std::pair<InstType, Immediate>;

// to_string methods for VM types.
std::string to_string(InstType i);
std::string to_string(Immediate i);
std::string to_string(Instruction i);
}
