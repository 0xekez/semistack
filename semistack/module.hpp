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
private:
    std::string _name;
    std::vector<Instruction> _instructions;
    
public:
    void addInstruction(Instruction i)
    {
        _instructions.push_back(std::move(i));
    }
};

}
