//
//  vm.hpp
//  semistack
//
//  Created by Zeke Medley on 1/9/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

//  Add modules to the VM. VM processes them (takes away labels, replaces with
//  absolute jumps). When we'd like to run the VM, add a new Frame to the stack
//  and then continue execution as normal. We should be able to largely ignore
//  the Call Frame for everything except local variables and program counter
//  information.

//  Always execute the frame on top of the stack. A jumpm instruction adds a new
//  Call Frame to the stack, and a ret instruction pops one. That should be the
//  only additional logic we need.

//  Call frame stores local variables for the session. VM holds global state.

#include "module.hpp"
#include "instruction.hpp"

#include <vector>
#include <array>
#include <stack>
#include <map>
#include <functional>
#include <iostream>

namespace vm {

using ModuleIndex = std::vector<Module>::size_type;

enum class ExitStatus: std::uint8_t
{
    ret,
    exit,
    error,
    cont,
};

struct CallFrame
{
    // The instruction that this frame is on.
    std::size_t _pc;
    // The Module that this frame is executing.
    ModuleIndex _moduleIndex;
    // Local variables inside of this call frame.
    // NOTE: This means that we're restricting a program to only 256 local
    // variables at a time. In exchange, we get some performance, and our sl, ll
    // instructions are made simpler because they can just specify an index.
    std::array<Value, 256> _locals;
    
    CallFrame(ModuleIndex mi): _moduleIndex(mi), _pc(0) {}
};

class VM
{
public:
    VM(): _outputFn([](std::string s){ std::cout << s << "\n"; }) {}
    VM(std::function<void(std::string)> fn): _outputFn(std::move(fn)) {}
    
    // Adds a module to the VM.
    bool addModule(vm::Module m);
    // Runs the selected module.
    ExitStatus run(std::string module_name);
    
private:
    ExitStatus runModule(const vm::Module& m);
    ExitStatus runInstruction(const Instruction& instruction);
    
    std::function<void(std::string)> _outputFn;
    
    std::array<Value, 256> _globals;
    
    std::vector<Module> _modules;
    // Maps module name to module index.
    std::map<std::string, ModuleIndex> _moduleLookup;
    
    std::stack<Value> _valueStack;
    std::stack<CallFrame> _callStack;
};

}
