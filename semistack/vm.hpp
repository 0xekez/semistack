//
//  vm.hpp
//  semistack
//
//  Created by Zeke Medley on 1/9/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

//  Add fns to the VM. VM processes them (takes away labels, replaces with
//  absolute jumps). When we'd like to run the VM, add a new Frame to the stack
//  and then continue execution as normal. We should be able to largely ignore
//  the Call Frame for everything except local variables and program counter
//  information.

//  Always execute the frame on top of the stack. A jumpm instruction adds a new
//  Call Frame to the stack, and a ret instruction pops one. That should be the
//  only additional logic we need.

//  Call frame stores local variables for the session. VM holds global state.

#include "function.hpp"
#include "instruction.hpp"

#include <vector>
#include <array>
#include <stack>
#include <map>
#include <functional>
#include <iostream>

namespace vm {

using FnIndex = std::vector<Function>::size_type;

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
    // The Function that this frame is executing.
    FnIndex _fnIndex;
    // Local variables inside of this call frame.
    // NOTE: This means that we're restricting a program to only 256 local
    // variables at a time. In exchange, we get some performance, and our sl, ll
    // instructions are made simpler because they can just specify an index.
    std::array<Value, 256> _locals;
    
    CallFrame(FnIndex mi): _fnIndex(mi), _pc(0) {}
};

class VM
{
public:
    VM(): _outputFn([](std::string s){ std::cout << s << "\n"; }) {}
    VM(std::function<void(std::string)> fn): _outputFn(std::move(fn)) {}
    
    // Adds a function to the VM and associate it with name.
    bool addFunction(vm::Function m, std::string name);
    // Runs the selected function.
    ExitStatus run(std::string fn_name);
    
private:
    ExitStatus runFunction(const vm::Function& m);
    ExitStatus runInstruction(const Instruction& instruction);
    
    std::function<void(std::string)> _outputFn;
    
    std::array<Value, 256> _globals;
    
    std::vector<Function> _functions;
    // Maps function name to function index.
    std::map<std::string, FnIndex> _fnLookup;
    
    std::stack<Value> _valueStack;
    std::stack<CallFrame> _callStack;
};

}
