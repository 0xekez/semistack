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

namespace vm {

class Module
{
public:
    std::string _name;
    std::vector<Instruction> _instructions;
    
    Module(std::string name): _name(std::move(name)) {}
    
    void addInstruction(Instruction i)
    {
        _instructions.push_back(std::move(i));
    }
    void addInstruction(InstType type, Immediate im = std::nullopt)
    {
        _instructions.push_back({std::move(type), std::move(im)});
    }
    bool operator==(const Module& rhs)
    {
        return _name == rhs._name && _instructions == rhs._instructions;
    }
};

}
