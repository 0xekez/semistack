//
//  value.h
//  semistack
//
//  Created by Zeke Medley on 1/13/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#pragma once

//  Here, we'd like to make some modifications to how the VM represents values
//  inspired by a paper on Lua's implementation which is linked in the Notes.md
//  file.
//
//  In particular, we'd like to introduce first class functions. A function is
//  a value that holds information about its closure and a list of instructions
//  to be run when it is called. As a result, it likely makes sense to change
//  things up a little bit and make functions the things that you add code to
//  rather than Modules.

#include <variant>
#include <memory>
#include <list>
#include <string>
#include <vector>

namespace vm {

// Some forward declarations to make the compiler happy. Many of these also
// appear in instruction.hpp.
enum class InstType;
using Object = std::variant<std::string, std::shared_ptr<struct Function>>;
using Value = std::variant<float, Object>;
using Immediate = std::optional<Value>;
using Instruction = std::pair<InstType, Immediate>;

}
