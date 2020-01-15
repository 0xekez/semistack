//
//  vm.cpp
//  semistack
//
//  Created by Zeke Medley on 1/9/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include "vm.hpp"
#include "logger.hpp"
#include "util.hpp"
#include "transform.hpp"

using namespace vm;

bool vm::VM::addFunction(vm::Function fn, std::string name)
{
    auto [where, success] = _fnLookup.insert({name, _functions.size()});
    if ( ! transform::assembleFunction(fn) )
    {
        logger()->error("Failed to assemble function: " + name);
        return false;
    }
    _functions.emplace_back(std::move(fn));
    return success;
}

ExitStatus vm::VM::run(std::string module_name)
{
    logger()->maintain(_callStack.size() == 0,
                       "Non-empty call stack for top level run instruction.");
    
    if ( ! transform::linkFunctions(_functions, _fnLookup) )
    {
        logger()->error("Failed to link modules");
        return ExitStatus::error;
    }
    
    auto where = _fnLookup.find(module_name);
    if (where == _fnLookup.end())
    {
        logger()->error("Failed to lookup module: " + module_name);
        return ExitStatus::error;
    }
    
    // Push a CallFrame for the module.
    _callStack.emplace(where->second);
    
    auto res = runFunction(_functions.at(where->second));
    
    if (_valueStack.size())
    {
        logger()->debug("Non empty call stack at end of execution.");
    }
    return res;
}

ExitStatus vm::VM::runFunction(const Function& m)
{
    auto nextInstruction = [this]()-> Instruction&
    {
        auto& m =  _functions.at(_callStack.top()._fnIndex);
        return m._instructions.at(_callStack.top()._pc++);
    };
    
    ExitStatus res = ExitStatus::cont;
    while (res == ExitStatus::cont)
    {
        auto& instruction = nextInstruction();
        res = runInstruction(instruction);
    }
    return res;
}

ExitStatus VM::runInstruction(Instruction& instruction)
{
    return ExitStatus::cont;
}
