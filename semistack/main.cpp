//
//  main.cpp
//  semistack
//
//  Created by Zeke Medley on 1/7/20.
//  Copyright © 2020 Zeke Medley. All rights reserved.
//

#include <iostream>

#include "logger.hpp"
#include "instruction.hpp"
#include "vm.hpp"
#include "transform.hpp"

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main(int argc, const char * argv[]) {
//    doctest::Context ctx;
//
//    // --exit to just run tests, --no-run to skip running them.
//    ctx.applyCommandLine(argc, argv);
//
//    // Don't break in the debugger.
//    ctx.setOption("no-breaks", true);
//
//    int res = ctx.run();
//
//    // Query flags (and --exit) rely on this.
//    if (ctx.shouldExit())
//        return res;
    
    vm::Module m("loop");
    
    //  pi 10     | 10
    //  copy      | 10 10
    //  pi 0      | 10 10 0
    //  jlt done  | 10
    //loop:
    //  pi -1     | 10 -1
    //  add       | 9
    //  copy      | 9 9
    //  puts      | 9
    //  copy      | 9 9
    //  pi 0      | 9 9 0
    //  jgt loop  | 9
    //done:
    //  exit
    
    m.addInstruction({vm::InstType::pi, 1000000});
    m.addInstruction({vm::InstType::copy, std::nullopt});
    m.addInstruction({vm::InstType::pi, 0});
    m.addInstruction({vm::InstType::jlt, "done"});
    m.addInstruction({vm::InstType::label, "loop"});
    m.addInstruction({vm::InstType::pi, -1});
    m.addInstruction({vm::InstType::add, std::nullopt});
//    m.addInstruction({vm::InstType::copy, std::nullopt});
//    m.addInstruction({vm::InstType::puts, std::nullopt});
    m.addInstruction({vm::InstType::copy, std::nullopt});
    m.addInstruction({vm::InstType::pi, 0});
    m.addInstruction({vm::InstType::jgt, "loop"});
    m.addInstruction({vm::InstType::label, "done"});
    m.addInstruction({vm::InstType::exit, std::nullopt});
    
    vm::VM v;
    vm::transform::assembleModule(m);
    v.addModule(std::move(m));
    v.run("loop");

    return 0;
}
